/**
 * @file    TTGTIKProcessor.cc
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-08-01 22:35:07
 * @note    last modified: 2024-10-31 17:42:25
 * @details bisection method (not Newton method)
 */

#include "TTGTIKProcessor.h"

#include "../geo/TDetectorParameter.h"
#include "TReactionInfo.h"
#include <Mass.h> // TSrim library
#include <TFile.h>
#include <TGraph.h>
#include <TKey.h>
#include <TLorentzVector.h>
#include <TRandom.h>

using art::crib::TTGTIKProcessor;

ClassImp(TTGTIKProcessor);

////////////////////////////////////////////////////////////////////////////////
/// From telescope object and beam (tracking) object, it calculate the
/// reaction position (event reconstruction) using Thick Gas Target method.
/// (Thick Gas Target Inverse Kinematics : TGTIK)
///
/// Need variables in the steering file:
/// - InputCollection: telescope object, inherit from art::TTelescopeData
/// - InputTrackCollection: tracking object, inherit from art::TTrack
/// - OutputCollection: name of output branch, inherit from art::TReactionInfo
/// - InitialBeamEnergy: beam energy in LAB system in MeV unit
/// - TargetName: target gas name for TSrim input
/// - TargetPressure: gas pressure in Torr unit
/// - TargetTemperature: gas emperature
/// - ParticleZArray: [Z1, Z2, Z3, Z4]
/// - ParticleAArray: [A1, A2, A3, A4]
///   - assuming two body reaction (NEED size 4 aray)
///   - [beam, target, fragment, residual]
///   - example: A(a,b)B reaction,
///   - the order is [a, A, b, B]
///   - this is "inverse kinematics", so we assume the detected particle is "B"
///
/// Option:
/// - ExcitedEnergy: if you want to treat excited state transition, input the
/// excited energy (MeV) of Z3 particles
/// - UseCustomFunction: if you need extra treatment, you can add the process.
///   - if it is "true", the custom treatment is on.
///   - this is for 26Si(a, p)29P analysis to treat the excited state effect.
/// - UseCenterPosition: use center position at the detector.
///   - if DSSSD is not working, this flag is used.

TTGTIKProcessor::TTGTIKProcessor() : fInData(nullptr), fInTrackData(nullptr), fOutData(nullptr) {
    RegisterInputCollection("InputCollection", "telescope data inherit from TTelescopeData", fInputColName,
                            TString("tel"));
    RegisterInputCollection("InputTrackCollection", "tracking data inherit from TTrack", fInputTrackColName,
                            TString("track"));
    RegisterOutputCollection("OutputCollection", "reconstracted reaction information using TGTIK method", fOutputColName,
                             TString("result"));

    IntVec_t init_i_vec;
    RegisterProcessorParameter("InitialBeamEnergy", "beam energy just after the window", fInitialBeamEnergy, 0.0);
    RegisterProcessorParameter("TargetName", "target gas name", fTargetName, TString(""));
    RegisterProcessorParameter("TargetPressure", "target pressure (Torr)", fPressure, 0.0);
    RegisterProcessorParameter("TargetTemperature", "target temperature (K)", fTemperature, 0.0);

    RegisterProcessorParameter("ParticleZArray", "particle atomic numbers of the reaction", fParticleZArray, init_i_vec);
    RegisterProcessorParameter("ParticleAArray", "particle mass numbers of the reaction", fParticleAArray, init_i_vec);

    // custom function
    RegisterProcessorParameter("ExcitedEnergy", "excited energy", fExcitedEnergy, -1.0);
    RegisterProcessorParameter("UseCustomFunction", "custom process", fDoCustom, false);

    RegisterOptionalInputInfo("DetectorParameter", "name of telescope parameter", fDetectorParameterName,
                              TString("prm_detectors"), &fDetectorPrm, "TClonesArray", "art::crib::TDetectorParameter");
    RegisterOptionalInputInfo("TargetParameter", "name of target parameter", fTargetParameterName,
                              TString("prm_targets"), &fTargetPrm, "TClonesArray", "art::crib::TTargetParameter");
    RegisterProcessorParameter("UseCenterPosition", "custom, use center position at the detecgtor", fDoCenterPos, false);
}

////////////////////////////////////////////////////////////////////////////////
/// free the memory

TTGTIKProcessor::~TTGTIKProcessor() {
    delete fOutData;
    fOutData = nullptr;
    delete srim;
    srim = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// in this process, it prepares some variables

void TTGTIKProcessor::Init(TEventCollection *col) {
    Info("Init", "%s, %s => %s", fInputColName.Data(), fInputTrackColName.Data(), fOutputColName.Data());

    fInData = reinterpret_cast<TClonesArray **>(col->GetObjectRef(fInputColName.Data()));
    if (!fInData) {
        SetStateError(Form("input not found: %s", fInputColName.Data()));
        return;
    }

    fInTrackData = reinterpret_cast<TClonesArray **>(col->GetObjectRef(fInputTrackColName.Data()));
    if (!fInTrackData) {
        SetStateError(Form("input not found: %s", fInputTrackColName.Data()));
        return;
    }

    const TClass *const cl1 = (*fInData)->GetClass();
    const TClass *const cl2 = (*fInTrackData)->GetClass();
    if (!(cl1->InheritsFrom(art::crib::TTelescopeData::Class()))) {
        SetStateError(Form("%s need to inherit from TTelescopeData", fInputColName.Data()));
        return;
    }
    if (!(cl2->InheritsFrom(art::TTrack::Class()))) {
        SetStateError(Form("%s need to inherit from TTrack", fInputTrackColName.Data()));
        return;
    }

    if (!fDetectorPrm) {
        SetStateError(Form("input not found: %s", fDetectorParameterName.Data()));
        return;
    }

    if (!fTargetPrm) {
        SetStateError(Form("input not found: %s", fTargetParameterName.Data()));
        return;
    }

    if (fParticleZArray.size() != 4 || fParticleAArray.size() != 4) {
        SetStateError("Particle array size should be 4 in the steering file");
        return;
    }

    // unit = mass (MeV)
    M1 = amdc::Mass(fParticleZArray[0], fParticleAArray[0]) * amdc::amu;
    M2 = amdc::Mass(fParticleZArray[1], fParticleAArray[1]) * amdc::amu;
    M3_default = amdc::Mass(fParticleZArray[2], fParticleAArray[2]) * amdc::amu;
    M4 = amdc::Mass(fParticleZArray[3], fParticleAArray[3]) * amdc::amu;

    if (!fDoCustom && fExcitedEnergy > 0)
        M3 = M3_default + fExcitedEnergy;
    else
        M3 = M3_default;

    Info("Init", "reconstract the reaction: %d%s + %d%s -> %d%s + %d%s <- detect",
         fParticleAArray[0], amdc::GetEl(fParticleZArray[0]).c_str(),
         fParticleAArray[1], amdc::GetEl(fParticleZArray[1]).c_str(),
         fParticleAArray[2], amdc::GetEl(fParticleZArray[2]).c_str(),
         fParticleAArray[3], amdc::GetEl(fParticleZArray[3]).c_str());

    Info("Init", "\tQ-value: %lf MeV", (M1 + M2) - (M3 + M4));

    // TSrim object
    const char *tsrim_path = std::getenv("TSRIM_DATA_HOME");
    if (!tsrim_path) {
        SetStateError("TSRIM_DATA_HOME environment variable is not defined");
        return;
    }
    srim = new TSrim();
    srim->AddElement("srim", 16, Form("%s/%s/range_fit_pol16_%s.txt", tsrim_path, fTargetName.Data(), fTargetName.Data()));

    // prepare output collection
    fOutData = new TClonesArray("art::crib::TReactionInfo");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);

    gRandom->SetSeed(time(nullptr));
}

////////////////////////////////////////////////////////////////////////////////
/// Main process

void TTGTIKProcessor::Process() {
    fOutData->Clear("C");

    Int_t nData = (*fInData)->GetEntriesFast();
    if (nData == 0)
        return;
    else if (nData != 1) {
        std::cerr << "warning: the input " << fInputColName << " entry number is not 1, but " << nData << std::endl;
    }

    Int_t nTrackData = (*fInTrackData)->GetEntriesFast();
    if (nTrackData == 0)
        return;
    if (nTrackData != 1) {
        std::cerr << "warning: the input " << fInputTrackColName << " entry number is not 1, but " << nTrackData
                  << std::endl;
    }

    const TDataObject *const inData = static_cast<TDataObject *>((*fInData)->At(0));
    const TTelescopeData *const Data = dynamic_cast<const TTelescopeData *>(inData);
    const TDataObject *const inTrackData = static_cast<TDataObject *>((*fInTrackData)->At(0));
    const TTrack *const TrackData = dynamic_cast<const TTrack *>(inTrackData);

    if (!IsValid(Data->GetTelID()))
        return;

    // energy threshold
    if (Data->GetEtotal() < 0.01)
        return;

    // excited energy process
    Double_t excited_energy = 0.0;
    if (fDoCustom) {
        excited_energy = GetCustomExcitedEnergy(Data->GetTelID(), Data->GetEtotal());
        if (excited_energy < -1.0)
            return;
        M3 = M3_default + excited_energy;
    } else if (fExcitedEnergy > 0)
        excited_energy = fExcitedEnergy;

    Double_t reac_z = GetReactionPosition(TrackData, Data);

    if (!IsValid(reac_z))
        return;

    // std::cout << reac_z << std::endl;

    TReactionInfo *outData = static_cast<TReactionInfo *>(fOutData->ConstructedAt(0));
    outData->SetID(0);
    outData->SetXYZ(TrackData->GetX(reac_z), TrackData->GetY(reac_z), reac_z);

    Double_t Ecm = GetEcmFromBeam(reac_z, TrackData);
    outData->SetEnergy(Ecm);

    auto [ELab, ALab] = GetELabALabPair(reac_z, TrackData, Data);
    outData->SetTheta(180.0 - (180.0 / TMath::Pi()) * GetCMAngle(ELab, Ecm, ALab));
    outData->SetExEnergy(excited_energy);
}

////////////////////////////////////////////////////////////////////////////////
/// Using bisection method (if you want Newton method is also available,
/// but in default, we use bisection method)

Double_t TTGTIKProcessor::GetReactionPosition(const TTrack *track, const TTelescopeData *data) {
    // return newton(track, data);
    return bisection(track, data);
}

////////////////////////////////////////////////////////////////////////////////
/// Currently not available, return kInvalidD.

Double_t TTGTIKProcessor::newton(const TTrack *, const TTelescopeData *) {
    return kInvalidD;
}

////////////////////////////////////////////////////////////////////////////////
/// Normal bisection method.

Double_t TTGTIKProcessor::bisection(const TTrack *track, const TTelescopeData *data) {
    Double_t flow = TargetFunction(kInitialMin, track, data);
    Double_t fhigh = TargetFunction(kInitialMax, track, data);
    Double_t low = kInitialMin;
    Double_t high = kInitialMax;
    Double_t ftmp = TargetFunction(kInitialMin, track, data);

    // search initial z position (rough bisection)
    Int_t step = 10;
    for (Int_t i = 0; i < step + 1; i++) {
        Double_t tmpz = kInitialMin + (Double_t)i * (kInitialMax - kInitialMin) / (Double_t)step;
        Double_t f = TargetFunction(tmpz, track, data);
        if (!IsValid(f))
            continue;

        if (ftmp * f > 0.0) {
            ftmp = f;
        } else {
            flow = ftmp;
            fhigh = f;
            low = kInitialMin + (Double_t)(i - 1) * (kInitialMax - kInitialMin) / (Double_t)step;
            high = tmpz;
            break;
        }
    }
    if (!IsValid(flow) || !IsValid(fhigh)) {
        std::cerr << "[Warning in bisect] f(low) or f(high) is invalid" << std::endl;
        return kInvalidD;
    }
    if (flow * fhigh > 0) {
        std::cerr << "[Warning in bisect] f(low) * f(high) > 0"
                  << ", f(low): " << flow << ", f(high): " << fhigh
                  << ", energy = " << data->GetEtotal()
                  << std::endl;
        return kInvalidD;
    }
    if (low == kInitialMin || high == kInitialMax) {
        std::cerr << "[Warning in bisect] could not find =0 position" << std::endl;
        return kInvalidD;
    }

    // start standard bisection method
    Double_t left = low;
    Double_t right = high;

    Double_t middle = 0.0;
    Int_t itr = 0;
    while (TMath::Abs(right - left) > kEpsilon) {
        middle = (left + right) / 2.0;

        Double_t fleft = TargetFunction(left, track, data);
        Double_t fmiddle = TargetFunction(middle, track, data);
        if (!IsValid(fleft) || !IsValid(fmiddle))
            return kInvalidD;

        if (fleft * fmiddle < 0) {
            right = middle;
        } else {
            left = middle;
        }

        itr++;
        if (itr == kMaxIteration) {
            std::cerr << "[bisection] not converge!" << std::endl;
            return kInvalidD;
        }
    }
    return middle;
}

////////////////////////////////////////////////////////////////////////////////
/// From <b>assuming Z position (reaction position)</b>,
/// calculate the Ecm from two direction.
/// - Ecm(beam): Ecm calculate from beam information
/// - Ecm(data): Ecm calculate from detected particle information
/// The target function is Ecm(beam) - Ecm(data).
/// When the value become 0, the assumed Z is reconstructed position.
/// (x, y, z) position will be calculated from tracking object.

Double_t TTGTIKProcessor::TargetFunction(Double_t z, const TTrack *track, const TTelescopeData *data) {
    Double_t Ecm_beam = GetEcmFromBeam(z, track);
    Double_t Ecm_detect = GetEcmFromDetectParticle(z, track, data);
    if (!IsValid(Ecm_beam) || !IsValid(Ecm_detect)) {
        return kInvalidD;
    }
    return Ecm_beam - Ecm_detect;
}

////////////////////////////////////////////////////////////////////////////////
/// From <b>assuming Z position (reaction position)</b>,
/// calculate the Ecm from beam kinematics and energy loss.
/// The beam is Z1 particle of [Z1, Z2, Z3, Z4].

Double_t TTGTIKProcessor::GetEcmFromBeam(Double_t z, const TTrack *track) {
    // if sign > 0 enewz, sign < 0, eoldz (thickness is negative)
    Int_t sign = z > 0 ? 1 : -1;
    TVector3 beam_flight(track->GetX(z) - track->GetX(0), track->GetY(z) - track->GetY(0), z);
    Double_t energy = srim->EnergyNew(fParticleZArray[0], fParticleAArray[0],
                                      fInitialBeamEnergy, std::string(fTargetName.Data()),
                                      sign * beam_flight.Mag(), fPressure, fTemperature);
    if (energy < 0.01)
        return 0.0;

    Double_t beta = TMath::Sqrt(1.0 - TMath::Power(M1 / (M1 + energy), 2));
    Double_t ang[2] = {track->GetA(), track->GetB()};
    Double_t norm = TMath::Sqrt(TMath::Tan(ang[0]) * TMath::Tan(ang[0]) + TMath::Tan(ang[1]) * TMath::Tan(ang[1]) + 1.0);
    Double_t beta_array[3] = {beta * TMath::Tan(ang[0]) / norm, beta * TMath::Tan(ang[1]) / norm, beta * 1.0 / norm};

    TLorentzVector beam(0., 0., 0., M1);
    beam.Boost(beta_array[0], beta_array[1], beta_array[2]);

    TLorentzVector target(0., 0., 0., M2);

    TLorentzVector cm_vec = beam + target;
    TVector3 beta_cm = cm_vec.BoostVector();
    beam.Boost(-beta_cm);
    target.Boost(-beta_cm);

    return (beam.E() - beam.M()) + (target.E() - target.M());
}

////////////////////////////////////////////////////////////////////////////////
/// From <b>assuming Z position (reaction position)</b>,
/// calculate the Ecm from detected particle information.
/// Currently it uses classsic kinematics.

Double_t TTGTIKProcessor::GetEcmFromDetectParticle(Double_t z, const TTrack *track, const TTelescopeData *data) {
    auto [energy, theta] = GetELabALabPair(z, track, data);
    if (!IsValid(energy))
        return kInvalidD;

    // kinematics (using bisection method) detected particle id=3
    // return GetEcm_kinematics(energy, theta, 0.01, 1.0e+4);

    // classic kinematics
    return GetEcm_classic_kinematics(energy, theta);
}

////////////////////////////////////////////////////////////////////////////////
/// From <b>assuming Z position (reaction position)</b>,
/// calculate the ELab and ALab of Z4 from detected particle information.
/// It uses TSrim library.

std::pair<Double_t, Double_t> TTGTIKProcessor::GetELabALabPair(Double_t z, const TTrack *track, const TTelescopeData *data) {
    Int_t tel_id = data->GetTelID();
    const TParameterObject *const inPrm = static_cast<TParameterObject *>((*fDetectorPrm)->At(tel_id - 1));
    const TDetectorParameter *Prm = dynamic_cast<const TDetectorParameter *>(inPrm);
    if (!Prm) {
        std::cerr << "parameter is not found" << std::endl;
        return {kInvalidD, kInvalidD};
    }

    Int_t stripNum[2] = {Prm->GetStripNum(0), Prm->GetStripNum(1)};
    Int_t stripID[2] = {data->GetXID(), data->GetYID()};
    Double_t stripSize[2] = {Prm->GetSize(0) / (Double_t)stripNum[0], Prm->GetSize(1) / (Double_t)stripNum[1]};

    Double_t det_x, det_y;
    if (fDoCenterPos)
        det_x = -Prm->GetSize(0) / 2.0 + Prm->GetSize(0);
    else if (stripID[0] >= 0 && stripID[0] < stripNum[0])
        det_x = -Prm->GetSize(0) / 2.0 + stripSize[0] * (Double_t)stripID[0];
    else
        det_x = -Prm->GetSize(0) / 2.0 + Prm->GetSize(0);

    if (fDoCenterPos)
        det_y = -Prm->GetSize(1) / 2.0 + Prm->GetSize(1);
    else if (stripID[1] >= 0 && stripID[1] < stripNum[1])
        det_y = -Prm->GetSize(1) / 2.0 + stripSize[1] * (Double_t)stripID[1];
    else
        det_y = -Prm->GetSize(1) / 2.0 + Prm->GetSize(1);

    TVector3 detect_position(det_x, det_y, Prm->GetDistance());
    detect_position.RotateY(Prm->GetAngle()); // rad
    detect_position += TVector3(Prm->GetCenterRotPos(0), Prm->GetCenterRotPos(1), Prm->GetCenterRotPos(2));

    TVector3 reaction_position(track->GetX(z), track->GetY(z), z);
    TVector3 track_direction(track->GetX(1.) - track->GetX(0.), track->GetY(1.) - track->GetY(0.), 1.0);

    Double_t theta = track_direction.Angle(detect_position - reaction_position); // LAB, rad
    Double_t energy = srim->EnergyNew(fParticleZArray[3], fParticleAArray[3],    // id = 3 particle
                                      data->GetEtotal(), std::string(fTargetName.Data()),
                                      -(detect_position - reaction_position).Mag(),
                                      fPressure, fTemperature);
    return {energy, theta};
}

////////////////////////////////////////////////////////////////////////////////
/// From detected energy and angle, calculate kinematics using relativity.
/// This is used in GetEcmFromDetectPartice method.
/// currently not avalable.

Double_t TTGTIKProcessor::GetEcm_kinematics(Double_t, Double_t, Double_t, Double_t) {
    return kInvalidD;
}

////////////////////////////////////////////////////////////////////////////////
/// From detected energy and angle, calculate kinematics using classic.
/// This is used in GetEcmFromDetectPartice method.
/// Please refer from okawa's master thesis for an detail.

Double_t TTGTIKProcessor::GetEcm_classic_kinematics(Double_t energy, Double_t theta) {
    Double_t alpha = (M2 * (M1 + M2)) / (2.0 * M1);
    Double_t beta = (M4 * (M3 + M4)) / (2.0 * M3);
    Double_t qvalue = (M1 + M2) - (M3 + M4);

    Double_t v4 = TMath::Sqrt(2.0 * energy / M4);
    // elastic scattering
    if (TMath::Abs(alpha - beta) < 1.0e-5) {
        Double_t vcm_elastic = -(qvalue - beta * v4 * v4) / (2.0 * beta * v4 * TMath::Cos(theta));
        if (vcm_elastic < 0) {
            std::cerr << "vcm < 0! : vcm = " << vcm_elastic
                      << ", det_energy : " << energy << ", theta : " << theta << std::endl;
            return kInvalidD;
        }
        return alpha * vcm_elastic * vcm_elastic;
    }
    Double_t b = (beta * v4 * TMath::Cos(theta)) / (alpha - beta);
    Double_t c = (qvalue - beta * v4 * v4) / (alpha - beta);
    Double_t D = b * b - c;
    if (D < 0) {
        if (TMath::Abs(D) < 1.0e-5) {
            D = 0.0;
        } else {
            std::cerr << "b^2 - c = " << D << " < 0, det_energy : " << energy << ", theta : " << theta << std::endl;
            return kInvalidD;
        }
    }

    Double_t vcm = -b + TMath::Sqrt(D);
    if (vcm < 0) {
        std::cerr << "vcm < 0! : vcm = " << -b << " + " << TMath::Sqrt(D) << ", det_energy : " << energy
                  << ", theta : " << theta << std::endl;
        return kInvalidD;
    }

    return alpha * vcm * vcm;
}

////////////////////////////////////////////////////////////////////////////////
/// Assuming Ecm and detected energy, calculate CM angle.
/// This is also used classic kinematics

Double_t TTGTIKProcessor::GetCMAngle(Double_t ELab, Double_t Ecm, Double_t ALab) {
    Double_t alpha = (M2 * (M1 + M2)) / (2.0 * M1);
    Double_t beta = (M4 * (M3 + M4)) / (2.0 * M3);
    Double_t qvalue = (M1 + M2) - (M3 + M4);

    Double_t v4 = TMath::Sqrt(2.0 * ELab / M4);
    Double_t vcm = TMath::Sqrt(Ecm / alpha);
    Double_t v4cm = TMath::Sqrt((Ecm + qvalue) / beta);

    Double_t theta_cm = TMath::ACos((v4 * TMath::Cos(ALab) - vcm) / v4cm);
    return theta_cm;
}

////////////////////////////////////////////////////////////////////////////////
/// This is used for 26Si(a, p)29P analysis.
/// from TALYS simulation data, the ratio of excited energy
/// are calculated and from the saved custom ROOT file,
/// assign the excited energy randomly.
///
/// this is not generic function, so file I/O is occur event by event.
/// So it is not an efficient function.
/// (almost all process can be included in Init function.)
///
/// You can modify as you like.
/// NOTE: current version seems to have memory leak

Double_t TTGTIKProcessor::GetCustomExcitedEnergy(Int_t telID, Double_t Etotal) {
    // up to 40th excited states of 29P
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

    // initialization
    TString filepath = "/home/okawa/art_analysis/develop/develop/ana2/random_generator.root";
    TFile *file = TFile::Open(filepath);
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file! :\n"
                  << filepath << std::endl;
        delete file;
        return 0.0;
    }

    TDirectory *dir = (TDirectory *)file->Get(Form("tel%d", telID));
    if (!dir) {
        std::cerr << "Error opening directory: tel" << telID << std::endl;
        file->Close();
        return 0.0;
    }

    std::vector<TGraph *> graphs;
    std::vector<TGraph *> grs_ratio;
    TList *keys = dir->GetListOfKeys();
    TIter next(keys);
    TKey *key;
    while ((key = (TKey *)next())) {
        TObject *obj = key->ReadObj();
        if (obj->InheritsFrom(TGraph::Class())) {
            graphs.emplace_back((TGraph *)obj);
            TGraph *g = new TGraph();
            grs_ratio.emplace_back(g);
        }
    }

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

        // for (const auto *gr : graphs) {
        for (Size_t j = 0; j < graphs.size(); j++) {
            // Double_t raw_val = graphs[j]->Eval(ene, nullptr, "S");
            Double_t raw_val = graphs[j]->Eval(ene);
            if (raw_val < 0.0)
                raw_val = 0.0;
            Double_t ratio = 0.0;

            if (total > 0.001)
                ratio = raw_val / total;

            if (j == 0 && total < 0.001)
                ratio = 1.0;

            grs_ratio[j]->SetPoint(i_point, ene, ratio);
        }
        i_point++;
    }

    // process of random number
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
            std::cerr << "could not assign excited id!" << std::endl;
        }
    }

    file->Close();
    delete file;

    // debug
    // std::cout << Etotal << ", assigned ex_id = " << ex_id << std::endl;

    return ex_energys[ex_id];
}
