/**
 * @file    TDetectParticleProcessor.cc
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-18 14:36:43
 * @note    last modified: 2024-09-18 15:57:31
 * @details
 */

#include "TDetectParticleProcessor.h"

#include "../geo/TDetectorParameter.h"
#include "../geo/TTargetParameter.h"
#include "../telescope/TTelescopeData.h"
#include "TParticleInfo.h"
#include <Mass.h> // TSrim library
#include <TRandom.h>

using art::crib::TDetectParticleProcessor;

ClassImp(TDetectParticleProcessor);

TDetectParticleProcessor::TDetectParticleProcessor()
    : fInData(nullptr), fInTrackData(nullptr), fOutData(nullptr), fInGeom(nullptr),
      fDetectorPrm(nullptr), fTargetPrm(nullptr), srim(nullptr) {
    RegisterInputCollection("InputCollection", "input branch (collection) name", fInputColName, TString("ions"));
    RegisterInputCollection("InputTrackCollection", "input track branch (collection) name", fInputTrackColName, TString("track"));
    RegisterOutputCollection("OutputCollection", "output branch (collection) name", fOutputColName,
                             TString("reaction_particles"));

    RegisterProcessorParameter("TargetIsGas", "Bool, target is targer or not", fTargetIsGas, false);
    RegisterProcessorParameter("TargetName", "gas target name", fTargetName, TString(""));
    RegisterProcessorParameter("TargetPressure", "Target presure", fTargetPressure, 0.0);

    DoubleVec_t init_d_vec;
    RegisterProcessorParameter("EnergyResolution", "energy resolution MeV unit", fEResolution, init_d_vec);
    RegisterProcessorParameter("TimingResolution", "timing resolution ns unit", fTResolution, init_d_vec);

    RegisterOptionalInputInfo("DetectorParameter", "name of telescope parameter", fDetectorParameterName,
                              TString("prm_detectors"), &fDetectorPrm, "TClonesArray", "art::crib::TDetectorParameter");
    /// currently not use this object
    RegisterOptionalInputInfo("TargetParameter", "name of target parameter", fTargetParameterName,
                              TString("prm_targets"), &fTargetPrm, "TClonesArray", "art::crib::TTargetParameter");
}

TDetectParticleProcessor::~TDetectParticleProcessor() {
    delete fOutData;
    delete srim;
    fOutData = nullptr;
    srim = nullptr;
}

void TDetectParticleProcessor::Init(TEventCollection *col) {
    fInGeom = reinterpret_cast<TGeoManager **>(col->GetObjectRef("geom"));
    if (!fInGeom) {
        SetStateError("gate array not found. Run TUserGeoInitializer before this.");
        return;
    }

    Info("Init", "%s => %s", fInputColName.Data(), fOutputColName.Data());

    fInData = reinterpret_cast<TClonesArray **>(col->GetObjectRef(fInputColName.Data()));
    if (!fInData) {
        SetStateError(Form("input not found: %s", fInputColName.Data()));
        return;
    }
    const TClass *const cl1 = (*fInData)->GetClass();
    if (!cl1->InheritsFrom(art::crib::TParticleInfo::Class())) {
        SetStateError("contents of input array must inherit from art::crib::TParticleInfo");
        return;
    }
    fInTrackData = reinterpret_cast<TClonesArray **>(col->GetObjectRef(fInputTrackColName.Data()));
    if (!fInTrackData) {
        SetStateError(Form("input not found: %s", fInputTrackColName.Data()));
        return;
    }
    const TClass *const cl2 = (*fInTrackData)->GetClass();
    if (!cl2->InheritsFrom(art::TTrack::Class())) {
        SetStateError("contents of input array must inherit from art::TTrack");
        return;
    }

    if (!fDetectorPrm) {
        SetStateError(Form("not found detector parameter object %s", fDetectorParameterName.Data()));
        return;
    }
    auto det_num = (*fDetectorPrm)->GetEntriesFast();
    Info("Init", "set %d number of detectors", det_num);

    // currently not used
    // if (!fTargetPrm) {
    //     SetStateError(Form("not found target parameter object %s", fTargetParameterName.Data()));
    // }

    if (fEResolution.size() == 1) { /// same energy resolution for all detector
        for (Int_t iDet = 0; iDet < det_num - 1; iDet++) {
            fEResolution.emplace_back(fEResolution[0]);
        }
    } else if (fEResolution.size() == 0) { /// ignore energy resolution
        for (Int_t iDet = 0; iDet < det_num; iDet++) {
            fEResolution.emplace_back(0.0);
        }
    } else if ((Int_t)fEResolution.size() != det_num) {
        SetStateError(Form("Resolution parameter size is wrong, should be %d, but %ld",
                           (*fDetectorPrm)->GetEntriesFast(), fEResolution.size()));
        return;
    }

    if (fTResolution.size() == 1) { /// same timing resolution for all detector
        for (Int_t iDet = 0; iDet < det_num - 1; iDet++) {
            fTResolution.emplace_back(fTResolution[0]);
        }
    } else if (fTResolution.size() == 0) { /// ignore energy resolution
        for (Int_t iDet = 0; iDet < det_num; iDet++) {
            fTResolution.emplace_back(0.0);
        }
    } else if ((Int_t)fTResolution.size() != det_num) {
        SetStateError(Form("Resolution parameter size is wrong, should be %d, but %ld",
                           (*fDetectorPrm)->GetEntriesFast(), fTResolution.size()));
        return;
    }

    fOutData = new TClonesArray("art::crib::TTelescopeData");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);

    /// initialization for TSrim
    const char *tsrim_path = std::getenv("TSRIM_DATA_HOME");
    if (!tsrim_path) {
        SetStateError("TSRIM_DATA_HOME environment variable is not defined");
        return;
    }
    // get All SRIM table for the target
    Info("Init", "Initializing SRIM table...");
    srim = new TSrim("srim", 16,
                     Form("%s/%s/range_fit_pol16_%s.txt", tsrim_path, fTargetName.Data(), fTargetName.Data()));
    Info("Init", "\t\"%s\" list loaded.", fTargetName.Data());

    StringVec_t material_names;
    for (auto i = 0; i < det_num; i++) {
        auto detprm = dynamic_cast<TDetectorParameter *>((*fDetectorPrm)->At(i));
        for (auto j = 0; j < detprm->GetN(); j++) {
            material_names.emplace_back(detprm->GetMaterial(j));
        }
    }
    StringVec_t unique_names = GetUniqueElements(material_names);
    for (const auto &str : unique_names) {
        srim->AddElement("srim", 16, Form("%s/%s/range_fit_pol16_%s.txt", tsrim_path, str.Data(), str.Data()));
    }

    gRandom->SetSeed(time(nullptr));
}

void TDetectParticleProcessor::Process() {
    fOutData->Clear("C");
    TGeoManager *geom = static_cast<TGeoManager *>(*fInGeom);

    for (Int_t iData = 0; iData < (*fInData)->GetEntriesFast(); ++iData) {
        const TDataObject *const inData = static_cast<TDataObject *>((*fInData)->At(iData));
        const TParticleInfo *const Data = dynamic_cast<const TParticleInfo *>(inData);

        TTelescopeData *outData = static_cast<TTelescopeData *>(fOutData->ConstructedAt(iData));
        outData->SetID(iData);

        Double_t energy = Data->GetEnergy();
        if (energy < 0.01) {
            continue; // stop in the target
        }

        Double_t current_z = Data->GetCurrentZ();
        TTrack track = Data->GetTrack();
        TVector3 first_position(track.GetX(current_z), track.GetY(current_z), current_z);
        TVector3 velocity = (Data->GetLorentzVector()).Vect().Unit();

        // check if beam particle hits the detector
        geom->SetCurrentPoint(first_position.X(), first_position.Y(), first_position.Z());
        geom->SetCurrentDirection(velocity.X(), velocity.Y(), velocity.Z());

        // exclude TOP boundary
        geom->GetCurrentNode();
        geom->FindNode();

        // geom->FindNextBoundary(10000.0); // need to set some values??
        geom->FindNextBoundary();
        Double_t distance = geom->GetStep();
        geom->Step();
        Bool_t isHit = geom->IsStepEntering();
        if (!isHit) {
            continue;
        }

        TString hitname = geom->GetPath(); // ex. hitname = /TOP_1/tel1_0, /TOP_1/tel4_3
        auto index_hitpath = hitname.Last('_');
        TString det_id = hitname(index_hitpath + 1, hitname.Length()); // get last number /TOP_1/tel1_0 -> 0
        if (hitname.Length() < 8 || det_id.Atoi() >= (*fDetectorPrm)->GetEntriesFast()) {
            // /TOP_1/ -> length = 7
            continue;
        }

        if (fTargetIsGas) {
            energy = srim->EnergyNew(Data->GetAtomicNumber(), Data->GetMassNumber(), energy,
                                     std::string(fTargetName.Data()), distance, fTargetPressure, 300.0);
            if (energy < 0.01) {
                continue; // stop in the target
            }
        }

        outData->SetTelID(det_id.Atoi() + 1); /// 1 start (not 0)
        TVector3 det_position(distance * velocity.X(), distance * velocity.Y(), distance * velocity.Z());
        det_position += first_position;
        outData->SetPosition(det_position);

        TParameterObject *inPrm = static_cast<TParameterObject *>((*fDetectorPrm)->At(det_id.Atoi()));
        TDetectorParameter *Prm = dynamic_cast<TDetectorParameter *>(inPrm);
        outData->SetN(Prm->GetN());

        det_position -= TVector3(Prm->GetCenterRotPos(0), Prm->GetCenterRotPos(1), Prm->GetCenterRotPos(2));
        det_position.RotateY(-Prm->GetAngle());

        outData->SetXID(GetStripID(det_position.X(), Prm->GetStripNum(0), Prm->GetSize(0)));
        outData->SetYID(GetStripID(det_position.Y(), Prm->GetStripNum(1), Prm->GetSize(1)));
        if (outData->GetXID() == -1 || outData->GetYID() == -1) {
            continue; /// hit from side, so not caliculate energy
        }

        // calculate TOF
        Double_t ion_mass = amdc::Mass(Data->GetAtomicNumber(), Data->GetMassNumber()) * amdc::amu;                // MeV
        Double_t duration = distance / (TMath::Sqrt(1.0 - TMath::Power(ion_mass / (ion_mass + energy), 2.0)) * c); // ns
        duration += Data->GetDurationTime();
        outData->PushTimingArray(gRandom->Gaus(duration, fTResolution[det_id.Atoi()]));

        // caliculate energy
        Double_t energy_total = 0.0;
        for (auto iMat = 0; iMat < Prm->GetN(); iMat++) {
            if (energy > 0.01) {
                Double_t new_energy = srim->EnergyNew(Data->GetAtomicNumber(), Data->GetMassNumber(), energy,
                                                      std::string((Prm->GetMaterial(iMat)).Data()), Prm->GetThickness(iMat));
                energy_total += energy - new_energy;
                outData->PushEnergyArray(energy - new_energy);
                energy = new_energy;
            } else {
                outData->PushEnergyArray(0.0);
            }
        }
        outData->SetEtotal(gRandom->Gaus(energy_total, fEResolution[det_id.Atoi()]));

        // caliculate LAB angle
        const TDataObject *const inTrackData = static_cast<TDataObject *>((*fInTrackData)->At(0));
        const TTrack *const TrackData = dynamic_cast<const TTrack *>(inTrackData);
        TVector3 beam(TrackData->GetX(1.0) - TrackData->GetX(0.0), TrackData->GetY(1.0) - TrackData->GetY(0.0), 1.0);
        Double_t theta = beam.Angle(velocity);
        outData->SetTheta_L(theta * TMath::RadToDeg());
    }
}

std::vector<TString> TDetectParticleProcessor::GetUniqueElements(const std::vector<TString> &input) {
    std::unordered_set<std::string> uniqueSet;
    std::vector<TString> result;

    for (const auto &tstr : input) {
        std::string str(tstr.Data());
        if (uniqueSet.find(str) == uniqueSet.end()) {
            uniqueSet.insert(str);
            result.emplace_back(tstr);
        }
    }
    return result;
}

Int_t TDetectParticleProcessor::GetStripID(Double_t pos, Int_t max_strip, Double_t size) {
    Int_t result = -1;
    for (Int_t i = 0; i < max_strip; i++) {
        if (size * ((Double_t)i / max_strip - 0.5) < pos && pos < size * ((Double_t)(i + 1) / max_strip - 0.5)) {
            result = i;
            break;
        }
    }
    // if(result == -1) std::cout << "warning, stripID is incorrect" << std::endl;
    return result;
}

// ===========================================
// kinematics
// tof
// beta = v/c
// E = m/sqrt(1-beta^2)
// v = sqrt(1-(M/E)^2) c
