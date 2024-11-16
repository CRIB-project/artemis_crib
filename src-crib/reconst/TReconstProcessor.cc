/**
 * @file    TReconstProcessor.cc
 * @brief   for solid target reconstruction
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-09-03 14:33:39
 * @note    last modified: 2024-09-03 17:05:31
 * @details
 */

#include "TReconstProcessor.h"

#include "../geo/TDetectorParameter.h"
#include "TReactionInfo.h"
#include <Mass.h> // TSrim library
#include <TFile.h>
#include <TGraph.h>
#include <TKey.h>
#include <TLorentzVector.h>
#include <TRandom.h>

using art::crib::TReconstProcessor;

ClassImp(TReconstProcessor);

////////////////////////////////////////////////////////////////////////////////
/// From telescope object and beam (tracking) object, it calculate the
/// Ecm assuming reaction position is z = 0 for solid target.
///
/// Need variables in the steering file:
/// - InputCollection: telescope object, inherit from art::TTelescopeData
/// - InputTrackCollection: tracking object, inherit from art::TTrack
/// - OutputCollection: name of output branch, inherit from art::TReactionInfo
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
/// - UseCenterPosition: use center position at the detector.
///   - if DSSSD is not working, this flag is used.

TReconstProcessor::TReconstProcessor() : fInData(nullptr), fInTrackData(nullptr), fOutData(nullptr) {
    RegisterInputCollection("InputCollection", "telescope data inherit from TTelescopeData", fInputColName,
                            TString("tel"));
    RegisterInputCollection("InputTrackCollection", "tracking data inherit from TTrack", fInputTrackColName,
                            TString("track"));
    RegisterOutputCollection("OutputCollection", "reconstracted reaction information using TGTIK method", fOutputColName,
                             TString("result"));

    IntVec_t init_i_vec;
    RegisterProcessorParameter("ParticleZArray", "particle atomic numbers of the reaction", fParticleZArray, init_i_vec);
    RegisterProcessorParameter("ParticleAArray", "particle mass numbers of the reaction", fParticleAArray, init_i_vec);

    // custom function
    RegisterProcessorParameter("ExcitedEnergy", "excited energy", fExcitedEnergy, -1.0);

    RegisterOptionalInputInfo("DetectorParameter", "name of telescope parameter", fDetectorParameterName,
                              TString("prm_detectors"), &fDetectorPrm, "TClonesArray", "art::crib::TDetectorParameter");
    RegisterOptionalInputInfo("TargetParameter", "name of target parameter", fTargetParameterName,
                              TString("prm_targets"), &fTargetPrm, "TClonesArray", "art::crib::TTargetParameter");
    RegisterProcessorParameter("UseCenterPosition", "custom, use center position at the detecgtor", fDoCenterPos, false);
}

////////////////////////////////////////////////////////////////////////////////
/// free the memory

TReconstProcessor::~TReconstProcessor() {
    delete fOutData;
    fOutData = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// in this process, it prepares some variables

void TReconstProcessor::Init(TEventCollection *col) {
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

    if (fExcitedEnergy > 0)
        M3 = M3_default + fExcitedEnergy;
    else
        M3 = M3_default;

    Info("Init", "reconstract the reaction: %d%s + %d%s -> %d%s + %d%s <- detect",
         fParticleAArray[0], amdc::GetEl(fParticleZArray[0]).c_str(),
         fParticleAArray[1], amdc::GetEl(fParticleZArray[1]).c_str(),
         fParticleAArray[2], amdc::GetEl(fParticleZArray[2]).c_str(),
         fParticleAArray[3], amdc::GetEl(fParticleZArray[3]).c_str());

    Info("Init", "\tQ-value: %lf MeV", (M1 + M2) - (M3 + M4));

    // prepare output collection
    fOutData = new TClonesArray("art::crib::TReactionInfo");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);

    gRandom->SetSeed(time(nullptr));
}

////////////////////////////////////////////////////////////////////////////////
/// Main process

void TReconstProcessor::Process() {
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
    if (fExcitedEnergy > 0)
        excited_energy = fExcitedEnergy;

    // reaction position
    Double_t z = 0.0;
    Double_t Ecm = GetEcmFromDetectParticle(z, TrackData, Data);

    if (!IsValid(Ecm))
        return;

    TReactionInfo *outData = static_cast<TReactionInfo *>(fOutData->ConstructedAt(0));
    outData->SetID(0);
    outData->SetXYZ(TrackData->GetX(z), TrackData->GetY(z), z);

    outData->SetEnergy(Ecm);
    outData->SetBeamEnergy(Ecm * ((M1 + M2) / M2));

    auto [ELab, ALab] = GetELabALabPair(z, TrackData, Data);
    outData->SetThetaL((180.0 / TMath::Pi()) * ALab);
    outData->SetTheta(180.0 - (180.0 / TMath::Pi()) * GetCMAngle(ELab, Ecm, ALab)); // inverse kinematics
    outData->SetExEnergy(excited_energy);
}

////////////////////////////////////////////////////////////////////////////////
/// From <b>assuming Z position (reaction position) = 0</b>,
/// calculate the Ecm from detected particle information.
/// Currently it uses classsic kinematics.

Double_t TReconstProcessor::GetEcmFromDetectParticle(Double_t z, const TTrack *track, const TTelescopeData *data) {
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

std::pair<Double_t, Double_t> TReconstProcessor::GetELabALabPair(Double_t z, const TTrack *track, const TTelescopeData *data) {
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
    Double_t energy = data->GetEtotal();

    return {energy, theta};
}

////////////////////////////////////////////////////////////////////////////////
/// From detected energy and angle, calculate kinematics using relativity.
/// This is used in GetEcmFromDetectPartice method.
/// currently not avalable.

Double_t TReconstProcessor::GetEcm_kinematics(Double_t, Double_t, Double_t, Double_t) {
    return kInvalidD;
}

////////////////////////////////////////////////////////////////////////////////
/// From detected energy and angle, calculate kinematics using classic.
/// This is used in GetEcmFromDetectPartice method.
/// Please refer from okawa's master thesis for an detail.

Double_t TReconstProcessor::GetEcm_classic_kinematics(Double_t energy, Double_t theta) {
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

Double_t TReconstProcessor::GetCMAngle(Double_t ELab, Double_t Ecm, Double_t ALab) {
    Double_t alpha = (M2 * (M1 + M2)) / (2.0 * M1);
    Double_t beta = (M4 * (M3 + M4)) / (2.0 * M3);
    Double_t qvalue = (M1 + M2) - (M3 + M4);

    Double_t v4 = TMath::Sqrt(2.0 * ELab / M4);
    Double_t vcm = TMath::Sqrt(Ecm / alpha);
    Double_t v4cm = TMath::Sqrt((Ecm + qvalue) / beta);

    Double_t theta_cm = TMath::ACos((v4 * TMath::Cos(ALab) - vcm) / v4cm);
    return theta_cm;
}
