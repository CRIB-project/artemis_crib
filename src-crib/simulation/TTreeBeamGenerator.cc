/**
 * @file    TTreeBeamGenerator.cc
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-09 15:57:01
 * @note    last modified: 2024-08-23 21:21:37
 * @details
 */

#include "TTreeBeamGenerator.h"

#include "TParticleInfo.h"
#include <Mass.h> // TSrim library
#include <TRandom.h>

using art::crib::TTreeBeamGenerator;

ClassImp(TTreeBeamGenerator);

TTreeBeamGenerator::TTreeBeamGenerator() : fOutData(nullptr) {
    RegisterInputCollection("InputCollection", "input track collection name", fInputColName, TString("track"));
    RegisterOutputCollection("OutputCollection", "simulation result collection", fOutputColName, TString("beam"));

    RegisterProcessorParameter("MassNum", "beam mass number", fMassNum, 0);
    RegisterProcessorParameter("AtomicNum", "beam atomic number", fAtmNum, 0);
    RegisterProcessorParameter("ChargeNum", "beam charge number", fChargeNum, 0);
    RegisterProcessorParameter("IniEnergy", "beam energy (MeV)", fBeamEnergy, 100.0);

    RegisterProcessorParameter("Esigma", "dispersion of beam energy (MeV)", fEsigma, 1.0);
}

TTreeBeamGenerator::~TTreeBeamGenerator() {
    delete fOutData;
    fOutData = nullptr;
}

void TTreeBeamGenerator::Init(TEventCollection *col) {
    Info("Init", "%s => %s", fInputColName.Data(), fOutputColName.Data());

    /// using TSrim library
    fMass = amdc::Mass(fAtmNum, fMassNum) * amdc::amu; // MeV
    Info("Init", "beam: (Z, A, M) = (%d, %d, %.5lf)", fAtmNum, fMassNum, fMass / amdc::amu);

    fInData = reinterpret_cast<TClonesArray **>(col->GetObjectRef(fInputColName.Data()));
    if (!fInData) {
        SetStateError(TString::Format("input not found: %s", fInputColName.Data()));
        return;
    }
    const TClass *const cl = (*fInData)->GetClass();
    if (!cl->InheritsFrom(art::TTrack::Class())) {
        SetStateError("contents of input array must inherit from art::TTrack");
    }

    fOutData = new TClonesArray("art::crib::TParticleInfo");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);

    gRandom->SetSeed(time(nullptr));
}

void TTreeBeamGenerator::Process() {
    fOutData->Clear("C");

    if ((*fInData)->GetEntriesFast() != 1) {
        return;
    }

    const TDataObject *const inData = static_cast<TDataObject *>((*fInData)->At(0));
    const TTrack *const Data = dynamic_cast<const TTrack *>(inData);

    TParticleInfo *outData = static_cast<TParticleInfo *>(fOutData->ConstructedAt(0));
    outData->SetMassNumber(fMassNum);
    outData->SetAtomicNumber(fAtmNum);
    outData->SetCharge(fChargeNum);

    Double_t energy = gRandom->Gaus(fBeamEnergy, fEsigma);
    outData->SetEnergy(energy);

    Double_t angx = Data->GetA();
    Double_t angy = Data->GetB();

    Double_t beta = TMath::Sqrt(1.0 - TMath::Power(fMass / (fMass + energy), 2)); // kinematics
    Double_t norm =
        TMath::Sqrt(TMath::Tan(angx) * TMath::Tan(angx) + TMath::Tan(angy) * TMath::Tan(angy) + 1.0); // kinematics
    Double_t beta_x = beta * TMath::Tan(angx) / norm;
    Double_t beta_y = beta * TMath::Tan(angy) / norm;
    Double_t beta_z = beta * 1.0 / norm;

    TLorentzVector beam(0., 0., 0., fMass);
    beam.Boost(beta_x, beta_y, beta_z);

    outData->SetID(0);
    outData->SetLorentzVector(beam);
    outData->SetCurrentZ(0.);
    outData->SetZeroTime();
    outData->SetTrack(Data->GetX(), Data->GetY(), 0., angx, angy);
}
