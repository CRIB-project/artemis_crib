/**
 * @file    TTGTIKProcessor.cc
 * @brief   Implementation of the TTGTIKProcessor class.
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-08-01 22:35:07
 * @note    last modified: 2025-03-04 18:21:05
 * @details bisection method (not Newton method)
 */

#include "TTGTIKProcessor.h"
#include "../TProcessorUtil.h"

#include "../geo/TDetectorParameter.h"
#include "TReactionInfo.h"
#include <Mass.h> // TSrim library
#include <TFile.h>
#include <TGraph.h>
#include <TKey.h>
#include <TLorentzVector.h>
#include <TRandom.h>

/// ROOT macro for class implementation
ClassImp(art::crib::TTGTIKProcessor);

namespace art::crib {

TTGTIKProcessor::TTGTIKProcessor()
    : fInData(nullptr),
      fInTrackData(nullptr),
      fOutData(nullptr),
      srim(nullptr) {
    RegisterInputCollection("InputCollection", "Input collection of telescope data objects (derived from TTelescopeData)",
                            fInputColName, TString("tel"));
    RegisterInputCollection("InputTrackCollection", "Input collection of tracking data objects (derived from TTrack)",
                            fInputTrackColName, TString("track"));
    RegisterOutputCollection("OutputCollection", "Output collection containing reaction reconstruction information using the TGTIK method",
                             fOutputColName, TString("result"));

    IntVec_t init_i_vec;
    RegisterProcessorParameter("InitialBeamEnergy", "Initial beam energy (in MeV) immediately after the exit window",
                               fInitialBeamEnergy, 0.0);
    RegisterProcessorParameter("TargetName", "Name of the target gas (used in TSrim calculation)",
                               fTargetName, TString(""));
    RegisterProcessorParameter("TargetPressure", "Target gas pressure in Torr",
                               fPressure, 0.0);
    RegisterProcessorParameter("TargetTemperature", "Target gas temperature in Kelvin",
                               fTemperature, 0.0);

    RegisterProcessorParameter("ParticleZArray", "Array of atomic numbers (Z) for reaction particles",
                               fParticleZArray, init_i_vec);
    RegisterProcessorParameter("ParticleAArray", "Array of mass numbers (A) for reaction particles",
                               fParticleAArray, init_i_vec);
    RegisterProcessorParameter("ExcitedEnergy", "Excited state energy (MeV); use a negative value if not applicable",
                               fExcitedEnergy, -1.0);

    // custom function
    RegisterProcessorParameter("UseCustomFunction", "Flag to enable custom processing functions for additional corrections",
                               fDoCustom, false);

    RegisterOptionalInputInfo("DetectorParameter", "Name of the telescope parameter collection (detector parameters)",
                              fDetectorParameterName, TString("prm_detectors"), &fDetectorPrm, "TClonesArray", "art::crib::TDetectorParameter");
    RegisterOptionalInputInfo("TargetParameter", "Name of the target parameter collection",
                              fTargetParameterName, TString("prm_targets"), &fTargetPrm, "TClonesArray", "art::crib::TTargetParameter");
    RegisterProcessorParameter("UseCenterPosition", "Flag to use the detector's center position (useful when the DSSSD is not operational)",
                               fDoCenterPos, false);
}

// free the memory
TTGTIKProcessor::~TTGTIKProcessor() {
    delete fOutData;
    fOutData = nullptr;
    delete srim;
    srim = nullptr;
}

/**
 * @details
 * This function prepares necessary input and output objects, validates parameters,
 * computes mass values for the reaction, initializes the TSrim object for energy loss calculations,
 * and sets up the output collection.
 */
void TTGTIKProcessor::Init(TEventCollection *col) {
    Info("Init", "%s, %s => %s", fInputColName.Data(), fInputTrackColName.Data(), fOutputColName.Data());

    // Retrieve the input telescope data collection.
    auto result = util::GetInputObject<TClonesArray>(
        col, fInputColName, "TClonesArray", "art::crib::TTelescopeData");
    if (std::holds_alternative<TString>(result)) {
        SetStateError(std::get<TString>(result));
        return;
    }
    fInData = std::get<TClonesArray *>(result);

    // Retrieve the input tracking data collection.
    auto result_track = util::GetInputObject<TClonesArray>(
        col, fInputTrackColName, "TClonesArray", "art::TTrack");
    if (std::holds_alternative<TString>(result_track)) {
        SetStateError(std::get<TString>(result_track));
        return;
    }
    fInTrackData = std::get<TClonesArray *>(result_track);

    if (!fDetectorPrm) {
        SetStateError(Form("Input not found: %s", fDetectorParameterName.Data()));
        return;
    }

    if (!fTargetPrm) {
        SetStateError(Form("Input not found: %s", fTargetParameterName.Data()));
        return;
    }

    if (fParticleZArray.size() != 4 || fParticleAArray.size() != 4) {
        SetStateError("Particle array size should be 4 in the steering file");
        return;
    }

    // unit = mass (MeV)
    // Calculate masses for reaction particles (unit: MeV).
    M1 = amdc::Mass(fParticleZArray[0], fParticleAArray[0]) * amdc::amu;
    M2 = amdc::Mass(fParticleZArray[1], fParticleAArray[1]) * amdc::amu;
    M3_default = amdc::Mass(fParticleZArray[2], fParticleAArray[2]) * amdc::amu;
    M4 = amdc::Mass(fParticleZArray[3], fParticleAArray[3]) * amdc::amu;

    if (!fDoCustom && fExcitedEnergy > 0)
        M3 = M3_default + fExcitedEnergy;
    else
        M3 = M3_default;

    Info("Init", "reconstract the reaction: %d%s + %d%s -> %d%s + %d%s (detected)",
         fParticleAArray[0], amdc::GetEl(fParticleZArray[0]).c_str(),
         fParticleAArray[1], amdc::GetEl(fParticleZArray[1]).c_str(),
         fParticleAArray[2], amdc::GetEl(fParticleZArray[2]).c_str(),
         fParticleAArray[3], amdc::GetEl(fParticleZArray[3]).c_str());
    Info("Init", "\tQ-value: %lf MeV", (M1 + M2) - (M3 + M4));

    // Initialize the TSrim object.
    const char *tsrim_path = std::getenv("TSRIM_DATA_HOME");
    if (!tsrim_path) {
        SetStateError("TSRIM_DATA_HOME environment variable is not defined");
        return;
    }
    srim = new TSrim();
    srim->AddElement("srim", 16, Form("%s/%s/range_fit_pol16_%s.txt", tsrim_path, fTargetName.Data(), fTargetName.Data()));

    // Prepare the output collection for reaction information.
    fOutData = new TClonesArray("art::crib::TReactionInfo");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);

    // Seed the random number generator. (used in custom function)
    gRandom->SetSeed(time(nullptr));
}

/**
 * @details
 * This function processes the input telescope and tracking data to reconstruct the reaction information.
 */
void TTGTIKProcessor::Process() {
    fOutData->Clear("C");
    if (!fInData) {
        Warning("Process", "No input data object");
        return;
    }

    const int nData = fInData->GetEntriesFast();
    if (nData == 0)
        return;
    else if (nData != 1) {
        Warning("Process", "The input %s entry number is not 1, but %d.",
                fInputColName.Data(), nData);
    }

    const int nTrackData = fInTrackData->GetEntriesFast();
    if (nTrackData == 0)
        return;
    if (nTrackData != 1) {
        Warning("Process", "The input %s entry number is not 1 but %d.",
                fInputTrackColName.Data(), nTrackData);
    }

    const auto *Data = dynamic_cast<const TTelescopeData *>(fInData->At(0));
    const auto *TrackData = dynamic_cast<const TTrack *>(fInTrackData->At(0));

    if (!IsValid(Data->GetTelID()))
        return;

    // Apply an energy threshold: skip events with total energy below 0.01.
    if (Data->GetEtotal() < 0.01)
        return;

    // Process excited state energy.
    Double_t excited_energy = 0.0;
    ///////////////////////////////////////////
    // custom process (currently for 26Si(a, p) analysis)
    ///////////////////////////////////////////
    if (fDoCustom) {
        excited_energy = GetCustomExcitedEnergy(Data->GetTelID(), Data->GetEtotal());
        if (excited_energy < -1.0)
            return; // assuming (a, 2p) process
        M3 = M3_default + excited_energy;
    } else if (fExcitedEnergy > 0)
        excited_energy = fExcitedEnergy;

    Double_t reac_z = GetReactionPosition(TrackData, Data);
    if (!IsValid(reac_z))
        return;

    // debug
    // std::cout << reac_z << std::endl;

    auto *outData = static_cast<TReactionInfo *>(fOutData->ConstructedAt(0));
    outData->SetID(0);
    outData->SetXYZ(TrackData->GetX(reac_z), TrackData->GetY(reac_z), reac_z);

    Double_t Ecm = GetEcmFromBeam(reac_z, TrackData);
    outData->SetEnergy(Ecm);

    auto [ELab, ALab] = GetELabALabPair(reac_z, TrackData, Data);
    outData->SetTheta(180.0 - (180.0 / TMath::Pi()) * GetCMAngle(ELab, Ecm, ALab));
    outData->SetExEnergy(excited_energy);
}

/**
 * @details
 * This function computes the reaction position (z-coordinate) by employing the bisection method.
 * Although an alternative Newton method is available, the bisection method is preferred for its robustness.
 * Newton method is not implemented yet.
 */
Double_t TTGTIKProcessor::GetReactionPosition(const TTrack *track, const TTelescopeData *data) {
    // return newton(track, data);
    return bisection(track, data);
}

/**
 * @details
 * Currently it is unavailable, return kInvalidD.
 */
Double_t TTGTIKProcessor::newton(const TTrack *, const TTelescopeData *) {
    return kInvalidD;
}

/**
 * @details
 * This function first performs a rough search over the interval [kInitialMin, kInitialMax] to
 * identify a valid subinterval where the target function is defined and exhibits a sign change.
 * It then applies the standard bisection method within that subinterval to converge on a zero.
 */
Double_t TTGTIKProcessor::bisection(const TTrack *track, const TTelescopeData *data) {
    // Stage 1: Rough bracketing within [kInitialMin, kInitialMax].
    const Int_t numSteps = 10;
    const Double_t stepSize = (kInitialMax - kInitialMin) / static_cast<Double_t>(numSteps);
    bool signChanged = false;
    bool firstValid = true;
    Double_t z_low = kInitialMin;  // Bracketing lower bound (to be determined)
    Double_t z_high = kInitialMax; // Bracketing upper bound (to be determined)
    Double_t prev_z = 0.0, prev_f = 0.0;

    // Single loop: Identify valid function values and detect a sign change.
    for (Int_t i = 0; i <= numSteps; ++i) {
        Double_t current_z = kInitialMin + i * stepSize;
        Double_t current_f = TargetFunction(current_z, track, data);
        if (!IsValid(current_f))
            continue; // Skip invalid function values.
        if (firstValid) {
            // First valid sample found.
            prev_z = current_z;
            prev_f = current_f;
            z_low = current_z;  // Initialize bracket lower bound.
            z_high = current_z; // Initialize bracket upper bound.
            firstValid = false;
        } else {
            z_high = current_z; // Update bracket upper bound with the current valid value.
            if (prev_f * current_f < 0.0) {
                // Sign change detected between the previous valid sample and the current one.
                z_low = prev_z;
                z_high = current_z;
                signChanged = true;
                break;
            }
            // Update previous valid sample.
            prev_z = current_z;
            prev_f = current_f;
        }
    }
    if (firstValid) {
        Warning("bisection", "No valid function value found in the initial range. (E = %lf)", data->GetEtotal());
        return kInvalidD;
    }
    if (!signChanged) {
        Warning("bisection", "Could not find a valid zero crossing in the initial range. (E = %lf)", data->GetEtotal());
        return kInvalidD;
    }

    // Validate the bracket: Ensure both endpoints are valid and have opposite signs.
    Double_t f_low = TargetFunction(z_low, track, data);
    Double_t f_high = TargetFunction(z_high, track, data);
    if (!IsValid(f_low) || !IsValid(f_high)) {
        Warning("bisection", "f(z_low) or f(z_high) is invalid.");
        return kInvalidD;
    }
    if (f_low * f_high > 0.0) {
        Warning("bisection", "f(z_low) and f(z_high) do not bracket a zero: L = %lf /  H = %lf, E = %lf",
                f_low, f_high, data->GetEtotal());
        return kInvalidD;
    }
    if (z_low == kInitialMin || z_high == kInitialMax) {
        Warning("bisection", "Bracketing failed: interval touches boundaries.");
        return kInvalidD;
    }

    // Stage 2: Standard bisection method within the bracket [z_low, z_high].
    Double_t left = z_low;
    Double_t right = z_high;
    Double_t middle = 0.0;
    Int_t iteration = 0;
    Double_t f_left = f_low;

    while (TMath::Abs(right - left) > kEpsilon) {
        middle = (left + right) / 2.0;
        Double_t f_middle = TargetFunction(middle, track, data);
        if (!IsValid(f_left) || !IsValid(f_middle))
            return kInvalidD;

        if (f_left * f_middle < 0.0) {
            right = middle;
        } else {
            left = middle;
            f_left = f_middle;
        }

        iteration++;
        if (iteration >= kMaxIteration) {
            Warning("bisection", "Convergence not achieved within maximum iteration!");
            return kInvalidD;
        }
    }
    return middle;
}

/**
 * @details
 * This function calculates two center-of-mass energies based on an assumed reaction
 * position (z):
 * - Ecm(beam): Calculated from beam information.
 * - Ecm(detected): Calculated from detected particle information.
 *
 * The target function is defined as:
 *   f(z) = Ecm(beam) - Ecm(detected)
 *
 * A zero crossing of this function indicates that the assumed z position corresponds
 * to the true reaction position. The (x, y, z) coordinates are then determined from the tracking data.
 */
Double_t TTGTIKProcessor::TargetFunction(Double_t z, const TTrack *track, const TTelescopeData *data) {
    Double_t Ecm_beam = GetEcmFromBeam(z, track);
    Double_t Ecm_detect = GetEcmFromDetectParticle(z, track, data);
    if (!IsValid(Ecm_beam) || !IsValid(Ecm_detect)) {
        return kInvalidD;
    }
    return Ecm_beam - Ecm_detect;
}

/**
 * @details
 * This function calculates the center-of-mass energy (Ecm) based on the beam's kinematics
 * and its energy loss when traversing the target material. The beam is considered to be
 * the first particle (Z1) of the reaction system [Z1, Z2, Z3, Z4].
 *
 * The procedure is as follows:
 * - Compute the beam's flight vector from its initial position (at z = 0) to the assumed
 *   reaction position z.
 * - Determine the effective path length through the target by applying a sign based on z.
 * - Use the TSrim library to compute the residual energy after energy loss.
 * - Calculate the beam's velocity (β) from its kinetic energy.
 * - Obtain the beam's direction from the track angles (assumed small so that tan(theta) approximations hold),
 *   and compute the normalized beta vector.
 * - Boost a beam TLorentzVector (initially at rest with mass M1) using this beta vector so that its energy becomes M1 + kinetic energy.
 * - Construct a target TLorentzVector at rest (mass M2), sum with the beam, and boost to the center-of-mass frame.
 * - Finally, return the total kinetic energy in the center-of-mass frame, which is the sum of (E - M) for both particles.
 */
Double_t TTGTIKProcessor::GetEcmFromBeam(Double_t z, const TTrack *track) {
    // Determine the sign for the effective target thickness based on z.
    // (Positive z implies forward thickness; negative z implies reverse thickness.)
    Int_t sign = z > 0 ? 1 : -1;

    // Calculate the beam flight vector from the initial position (z = 0) to the assumed reaction position.
    TVector3 beam_flight(track->GetX(z) - track->GetX(0),
                         track->GetY(z) - track->GetY(0),
                         z);

    // Calculate the residual energy after energy loss in the target.
    // The effective path length is given by the magnitude of beam_flight multiplied by the sign.
    Double_t energy = srim->EnergyNew(fParticleZArray[0],
                                      fParticleAArray[0],
                                      fInitialBeamEnergy,
                                      std::string(fTargetName.Data()),
                                      sign * beam_flight.Mag(),
                                      fPressure,
                                      fTemperature);
    if (energy < 0.01)
        return 0.0;

    // Calculate the beam's velocity (beta) from the relativistic relation:
    // beta = sqrt(1 - (M1/(M1 + kineticEnergy))^2)
    Double_t beta = TMath::Sqrt(1.0 - TMath::Power(M1 / (M1 + energy), 2));

    // Retrieve the track angles for the beam direction.
    Double_t ang[2] = {track->GetA(), track->GetB()};

    // Compute the norm of the direction vector (tan(angleA), tan(angleB), 1).
    Double_t norm = TMath::Sqrt(TMath::Power(TMath::Tan(ang[0]), 2) +
                                TMath::Power(TMath::Tan(ang[1]), 2) + 1.0);

    // Calculate the beta vector components by normalizing the direction.
    Double_t betaX = beta * TMath::Tan(ang[0]) / norm;
    Double_t betaY = beta * TMath::Tan(ang[1]) / norm;
    Double_t betaZ = beta / norm;
    TVector3 betaVector(betaX, betaY, betaZ);

    // Construct a TLorentzVector for the beam particle at rest with mass M1.
    TLorentzVector beam(0., 0., 0., M1);
    // Boost the beam vector to obtain its 4-momentum (energy becomes M1 + energy).
    beam.Boost(betaVector);

    // Construct a TLorentzVector for the target particle at rest with mass M2.
    TLorentzVector target(0., 0., 0., M2);

    // Compute the total 4-momentum and determine the boost vector for the center-of-mass frame.
    TLorentzVector cm_vec = beam + target;
    TVector3 beta_cm = cm_vec.BoostVector();
    // Boost both beam and target into the center-of-mass frame.
    beam.Boost(-beta_cm);
    target.Boost(-beta_cm);

    // Return the total kinetic energy in the center-of-mass frame.
    // Kinetic energy is computed as (E - M) for each particle.
    return (beam.E() - beam.M()) + (target.E() - target.M());
}

/**
 * @details
 * This function computes the center-of-mass energy (Ecm) using the laboratory energy and angle
 * of the detected particle obtained from the assumed reaction position (z). Currently, it employs
 * classical kinematics for the calculation.
 */
Double_t TTGTIKProcessor::GetEcmFromDetectParticle(Double_t z, const TTrack *track, const TTelescopeData *data) {
    auto [energy, theta] = GetELabALabPair(z, track, data);
    if (!IsValid(energy))
        return kInvalidD;

    // kinematics (using bisection method) detected particle id=3
    // relative kinematics (not implemented)
    // return GetEcm_kinematics(energy, theta, 0.01, 1.0e+4);

    // classic kinematics
    return GetEcm_classic_kinematics(energy, theta);
}

/**
 * @details
 * This function calculates the LAB energy (ELab) and LAB angle (ALab) of the detected particle,
 * based on an assumed reaction position (z). It retrieves the detector parameters corresponding to
 * the telescope ID, computes the effective detection position using either the strip information or
 * the detector center position (depending on the flag), and then applies the TSrim library to
 * determine the energy loss. The LAB angle is computed as the angle between the track direction and
 * the vector from the reaction position to the detection position.
 */
std::pair<Double_t, Double_t> TTGTIKProcessor::GetELabALabPair(Double_t z, const TTrack *track, const TTelescopeData *data) {
    // Retrieve detector parameters based on the telescope ID.
    Int_t tel_id = data->GetTelID();
    const TParameterObject *const inPrm = static_cast<TParameterObject *>((*fDetectorPrm)->At(tel_id - 1));
    const TDetectorParameter *Prm = dynamic_cast<const TDetectorParameter *>(inPrm);
    if (!Prm) {
        Warning("GetELabALabPair", "Parameter is not found");
        return {kInvalidD, kInvalidD};
    }

    // Obtain the number of strips and calculate individual strip sizes for X and Y directions.
    Int_t stripNum[2] = {Prm->GetStripNum(0), Prm->GetStripNum(1)};
    Int_t stripID[2] = {data->GetXID(), data->GetYID()};
    Double_t stripSize[2] = {Prm->GetSize(0) / static_cast<Double_t>(stripNum[0]),
                             Prm->GetSize(1) / static_cast<Double_t>(stripNum[1])};

    // Determine the detection position along X.
    Double_t det_x = 0.0;
    if (fDoCenterPos) {
        // Use the detector center position.
        det_x = Prm->GetSize(0) / 2.0;
    } else if (stripID[0] >= 0 && stripID[0] < stripNum[0]) {
        // Use the position corresponding to the valid strip ID.
        det_x = -Prm->GetSize(0) / 2.0 + stripSize[0] * static_cast<Double_t>(stripID[0]);
    } else {
        // Default to the detector center if stripID is invalid.
        det_x = Prm->GetSize(0) / 2.0;
    }

    // Determine the detection position along Y.
    Double_t det_y = 0.0;
    if (fDoCenterPos)
        det_y = Prm->GetSize(1) / 2.0;
    else if (stripID[1] >= 0 && stripID[1] < stripNum[1])
        det_y = -Prm->GetSize(1) / 2.0 + stripSize[1] * (Double_t)stripID[1];
    else
        det_y = Prm->GetSize(1) / 2.0;

    // Build the detection position vector.
    TVector3 detect_position(det_x, det_y, Prm->GetDistance());
    // Apply rotation about the Y-axis according to the detector angle.
    detect_position.RotateY(Prm->GetAngle()); // rad
    // Apply translation to adjust for the detector's rotational center.
    detect_position += TVector3(Prm->GetCenterRotPos(0),
                                Prm->GetCenterRotPos(1),
                                Prm->GetCenterRotPos(2));

    // Determine the reaction position based on tracking data.
    TVector3 reaction_position(track->GetX(z), track->GetY(z), z);
    // Calculate the approximate track direction using positions at z = 0 and z = 1.
    TVector3 track_direction(track->GetX(1.) - track->GetX(0.),
                             track->GetY(1.) - track->GetY(0.),
                             1.0);

    // Compute the LAB angle as the angle between the track direction and the vector from the reaction position to the detection position.
    Double_t theta = track_direction.Angle(detect_position - reaction_position); // LAB, rad

    // Use TSrim to calculate the LAB energy for the detected particle (assumed particle ID = 3).
    Double_t energy = srim->EnergyNew(fParticleZArray[3], fParticleAArray[3], // id = 3 particle
                                      data->GetEtotal(), std::string(fTargetName.Data()),
                                      -(detect_position - reaction_position).Mag(),
                                      fPressure, fTemperature);
    return {energy, theta};
}

/**
 * @details
 * This function is intended to compute the center-of-mass energy (Ecm) from the detected
 * laboratory energy and angle using relativistic formulas. It is designed to be used in the
 * GetEcmFromDetectParticle method. However, the relativistic kinematics implementation is
 * currently not available, and this function always returns kInvalidD.
 */
Double_t TTGTIKProcessor::GetEcm_kinematics(Double_t, Double_t, Double_t, Double_t) {
    return kInvalidD;
}

/**
 * @details
 * This function calculates the center-of-mass energy (Ecm) from the detected particle's
 * laboratory energy and angle using classical (non-relativistic) kinematics. It is used in the
 * GetEcmFromDetectParticle method. The formulas employed here are based on those detailed in
 * Okawa's master thesis.
 */
Double_t TTGTIKProcessor::GetEcm_classic_kinematics(Double_t energy, Double_t theta) {
    // Compute kinematic factors based on the masses:
    // alpha: factor from the beam (particle 1) and target (particle 2) system.
    // beta: factor from the detected particle (particle 4) and the complementary fragment (particle 3).
    Double_t alpha = (M2 * (M1 + M2)) / (2.0 * M1);
    Double_t beta = (M4 * (M3 + M4)) / (2.0 * M3);
    // Q-value of the reaction.
    Double_t qvalue = (M1 + M2) - (M3 + M4);

    // Calculate the velocity of the detected particle (particle 4) from its kinetic energy.
    // Using the classical relation: energy = 0.5 * M4 * v4^2  =>  v4 = sqrt(2 * energy / M4)
    Double_t v4 = TMath::Sqrt(2.0 * energy / M4);

    // Elastic scattering case: when alpha and beta are nearly equal.
    if (TMath::Abs(alpha - beta) < 1.0e-5) {
        Double_t cosTheta = TMath::Cos(theta);
        if (TMath::Abs(cosTheta) < 1.0e-5) {
            Warning("GetEcm_classic_kinematics", "cos(theta) is too small in elastic scattering calculation");
            return kInvalidD;
        }
        // Calculate the center-of-mass velocity for elastic scattering.
        Double_t vcm_elastic = -(qvalue - beta * v4 * v4) / (2.0 * beta * v4 * cosTheta);
        if (vcm_elastic < 0) {
            Warning("GetEcm_classic_kinematics", "vcm < 0! : vcm = %lf, det_energy = %lf, theta = %lf",
                    vcm_elastic, energy, theta);
            return kInvalidD;
        }
        return alpha * vcm_elastic * vcm_elastic;
    }

    // Non-elastic scattering case:
    // Solve the quadratic equation for v_cm: v_cm = -b + sqrt(b^2 - c),
    // where b = (beta * v4 * cos(theta)) / (alpha - beta) and
    //       c = (qvalue - beta * v4 * v4) / (alpha - beta)
    Double_t denominator = (alpha - beta);
    Double_t b = (beta * v4 * TMath::Cos(theta)) / denominator;
    Double_t c = (qvalue - beta * v4 * v4) / denominator;
    Double_t D = b * b - c;
    if (D < 0) {
        if (TMath::Abs(D) < 1.0e-5) {
            D = 0.0;
        } else {
            Warning("GetEcm_classic_kinematics", "b^2 - c = %lf < 0, det_energy = %lf, theta = %lf",
                    D, energy, theta);
            return kInvalidD;
        }
    }

    Double_t vcm = -b + TMath::Sqrt(D);
    if (vcm < 0) {
        Warning("GetEcm_classic_kinematics", "vcm < 0!, vcm = %lf + %lf, det_energy = %lf, theta = %lf",
                -b, TMath::Sqrt(D), energy, theta);
        return kInvalidD;
    }

    return alpha * vcm * vcm;
}

/**
 * @details
 * This function calculates the CM scattering angle (θ_cm) based on the detected particle's
 * laboratory energy (ELab) and laboratory scattering angle (ALab), as well as the center-of-mass
 * energy (Ecm). The calculation employs classical kinematics, using coefficients derived from
 * the masses of the reaction participants. In particular:
 * - α is computed from the beam (particle 1) and target (particle 2) masses.
 * - β is computed from the detected particle (particle 4) and the complementary fragment (particle 3) masses.
 * - q-value represents the mass difference between the entrance and exit channels.
 * The function then computes the classical velocities of the detected particle in the LAB frame (v4),
 * the beam velocity in the CM frame (v_cm), and the detected particle's velocity in the CM frame (v4_cm).
 * Finally, the CM angle is obtained from the cosine relationship:
 *   θ_cm = acos((v4*cos(ALab) - v_cm) / v4_cm).
 */
Double_t TTGTIKProcessor::GetCMAngle(Double_t ELab, Double_t Ecm, Double_t ALab) {
    // Calculate kinematic coefficients from the particle masses.
    Double_t alpha = (M2 * (M1 + M2)) / (2.0 * M1);
    Double_t beta = (M4 * (M3 + M4)) / (2.0 * M3);
    Double_t qvalue = (M1 + M2) - (M3 + M4);

    // Compute the detected particle's velocity in the LAB frame (classical approximation).
    Double_t v4 = TMath::Sqrt(2.0 * ELab / M4);
    // Compute the effective beam velocity in the center-of-mass frame.
    Double_t vcm = TMath::Sqrt(Ecm / alpha);
    // Compute the detected particle's velocity in the center-of-mass frame.
    Double_t v4cm = TMath::Sqrt((Ecm + qvalue) / beta);

    // Calculate the CM scattering angle using the cosine law.
    Double_t theta_cm = TMath::ACos((v4 * TMath::Cos(ALab) - vcm) / v4cm);
    return theta_cm;
}

/**
 * @details
 * This is used for 26Si(a, p)29P analysis.
 * This function uses a custom random generator ROOT file (specific to this analysis)
 * to assign an excited state energy for 29P based on TALYS simulation data. It reads
 * graphs from a directory corresponding to the given telescope ID, computes the ratio
 * of excited state contributions as a function of energy, and then, using a uniform
 * random number, selects one excited state.
 *
 * NOTE: This function is not designed for generic use and performs file I/O on an
 * event-by-event basis, which is inefficient.
 */
Double_t TTGTIKProcessor::GetCustomExcitedEnergy(Int_t telID, Double_t Etotal) {
    // Up to 40th excited states of 29P
    Int_t ex_id = 0;
    const Double_t ex_energys[42] = {
        0.0,
        1.38355,
        1.95391,
        2.4227,
        3.1059,
        3.4476,
        4.0805,
        4.343001,
        4.642001,
        4.759001,
        4.95410142,
        5.0470012,
        5.293002,
        5.527001,
        5.583001,
        5.716001,
        5.740001,
        5.826001,
        5.968,
        6.191,
        6.328,
        6.505,
        6.577,
        6.828,
        6.956,
        7.021,
        7.148,
        7.272,
        7.361,
        7.456,
        7.523,
        7.641,
        7.755,
        7.95,
        7.998,
        8.106,
        8.234,
        8.297,
        8.379,
        8.432,
        8.51,
        -100.0,
    };
    Double_t max_energy = 30.0; // MeV
    if (Etotal > max_energy)
        return 0.0;

    // Open the custom ROOT file.
    TString filepath = "/home/okawa/art_analysis/develop/develop/ana2/random_generator.root";
    TFile *file = TFile::Open(filepath);
    if (!file || file->IsZombie()) {
        Warning("GetCustomExcitedEnergy", "Error opening file: %s", filepath.Data());
        if (file)
            delete file;
        return 0.0;
    }

    // Get the directory for the given telescope ID.
    TDirectory *dir = dynamic_cast<TDirectory *>(file->Get(Form("tel%d", telID)));
    if (!dir) {

        Warning("GetCustomExcitedEnergy", "Error opening directory: tel%d", telID);
        file->Close();
        delete file;
        return 0.0;
    }

    // Read TGraph objects from the directory.
    std::vector<TGraph *> graphs;
    std::vector<TGraph *> grs_ratio;
    TList *keys = dir->GetListOfKeys();
    TIter next(keys);
    TKey *key = nullptr;
    while ((key = (TKey *)next())) {
        TObject *obj = key->ReadObj();
        if (obj && obj->InheritsFrom(TGraph::Class())) {
            graphs.emplace_back(static_cast<TGraph *>(obj));
            TGraph *g = new TGraph();
            grs_ratio.emplace_back(g);
        } else {
            delete obj;
        }
    }

    // Build the ratio graphs over an energy range from 5.0 to max_energy (step 0.1).
    Int_t i_point = 0;
    for (Double_t ene = 5.0; ene < max_energy; ene += 0.1) {
        Double_t total = 0.0;
        for (const auto *gr : graphs) {
            // Double_t val = gr->Eval(ene, nullptr, "S");
            Double_t val = gr->Eval(ene);
            if (val < 0.0)
                val = 0.0;
            total += val;
        }

        for (Size_t j = 0; j < graphs.size(); j++) {
            // Double_t raw_val = graphs[j]->Eval(ene, nullptr, "S");
            Double_t raw_val = graphs[j]->Eval(ene);
            if (raw_val < 0.0)
                raw_val = 0.0;
            Double_t ratio = (total > 0.001) ? raw_val / total : (j == 0 ? 1.0 : 0.0);
            grs_ratio[j]->SetPoint(i_point, ene, ratio);
        }
        i_point++;
    }

    // Use a uniform random number to select an excited state based on the ratio graphs.
    Double_t uniform = gRandom->Uniform();
    Double_t tmp = 0.0;
    for (Size_t i = 0; i < grs_ratio.size(); i++) {
        // Double_t val = grs_ratio[i]->Eval(Etotal, nullptr, "S");
        Double_t val = grs_ratio[i]->Eval(Etotal);
        if (val < 0.0)
            val = 0.0;
        tmp += val;
        if (uniform < tmp) {
            ex_id = i;
            break;
        }
        if (i == grs_ratio.size() - 1) {
            Warning("GetCustomExcitedEnergy", "Could not assign excited id!");
        }
    }

    // Clean up dynamically allocated TGraph objects in grs_ratio.
    for (auto g : grs_ratio) {
        delete g;
    }
    grs_ratio.clear();

    // Clean up the TGraph objects from the file.
    for (auto g : graphs) {
        delete g;
    }
    graphs.clear();

    file->Close();
    delete file;

    // debug
    // std::cout << Etotal << ", assigned ex_id = " << ex_id << std::endl;

    return ex_energys[ex_id];
}

} // namespace art::crib
