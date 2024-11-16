/**
 * @file    TTelescopeProcessor.cc
 * @brief   gather the telescope information to the one object
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-17 17:52:58
 * @note    last modified: 2024-08-23 21:05:12
 * @details treat the largest value of each layor
 *          the data of X side is used for DSSSD
 *          assume beam position (0, 0) and direction (0, 0, 1)
 *          NEED OutputColName == prm/geo detector name
 */

#include "TTelescopeProcessor.h"

#include "../geo/TDetectorParameter.h"
#include "../geo/TTargetParameter.h"
#include "TTelescopeData.h"
#include "TTimingChargeData.h"
#include <TRandom.h>

using art::crib::TTelescopeProcessor;

ClassImp(TTelescopeProcessor);

// Default constructor
TTelescopeProcessor::TTelescopeProcessor()
    : fInData1(nullptr), fInData2(nullptr), fInData3(nullptr), fOutData(nullptr), fTelID(0) {
    RegisterInputCollection("InputCollection1",
                            "array of objects inheriting from art::TTimingChargeData",
                            fInputColName1, TString("dEX"));
    RegisterInputCollection("InputCollection2",
                            "array of objects inheriting from art::TTimingChargeData",
                            fInputColName2, TString("dEY"));
    RegisterInputCollection("InputCollection3",
                            "array of objects inheriting from art::TTimingChargeData",
                            fInputColName3, TString("E"));
    RegisterOutputCollection("OutputCollection", "output class will be art::crib::TTelescopeData",
                             fOutputColName, TString("telescope"));

    RegisterProcessorParameter("IsDSSSD", "Bool, true: first layer is DSSSD, false: first and second layer is SSSSD", fIsDSSSD, true);
    RegisterProcessorParameter("UseRandom", "Bool, true: Uniform distribution in one pixel, false: center of the pixel", fUseRandom, false);

    RegisterOptionalInputInfo("DetectorParameter", "name of detector parameter defined in TUserGeoInitializer", fDetPrmName,
                              TString("prm_detectors"), &fDetParameters, "TClonesArray", "art::crib::TDetectorParameter");
    RegisterOptionalInputInfo("TargetParameter", "name of target parameter defined in TUserGeoInitializer", fTargetPrmName,
                              TString("prm_targets"), &fTargetParameters, "TClonesArray", "art::crib::TTargetParameter");
}

TTelescopeProcessor::~TTelescopeProcessor() {
    delete fOutData;
    fOutData = nullptr;
}

void TTelescopeProcessor::Init(TEventCollection *col) {
    Info("Init", "%s %s %s => %s", fInputColName1.Data(), fInputColName2.Data(), fInputColName3.Data(), fOutputColName.Data());

    // check if X strip SSD object exist or not
    fInData1 = reinterpret_cast<TClonesArray **>(col->GetObjectRef(fInputColName1.Data()));
    if (!fInData1) {
        SetStateError(TString::Format("input not found: %s", fInputColName1.Data()));
        return;
    }

    // check if Y strip SSD object exist or not
    fInData2 = reinterpret_cast<TClonesArray **>(col->GetObjectRef(fInputColName2.Data()));
    if (!fInData2) {
        SetStateError(TString::Format("input not found: %s", fInputColName2.Data()));
        return;
    }

    // check if thick SSDs object exist or not
    fInData3 = reinterpret_cast<TClonesArray **>(col->GetObjectRef(fInputColName3.Data()));
    if (!fInData3) {
        SetStateError(TString::Format("input not found: %s", fInputColName3.Data()));
        return;
    }

    // detector parameter
    if (!fDetParameters) {
        fHasDetPrm = false;
    } else {
        fHasDetPrm = true;
        for (Int_t iDet = 0; iDet < (*fDetParameters)->GetEntriesFast(); iDet++) {
            TParameterObject *inPrm = static_cast<TParameterObject *>((*fDetParameters)->At(iDet));
            TDetectorParameter *prm = dynamic_cast<TDetectorParameter *>(inPrm);
            if (prm->GetDetName() == fOutputColName) {
                fTelID = iDet + 1;
                fDetParameter = prm;
            }
        }
        if (fTelID == 0) {
            Warning("Init", "cannot find %s node, please check TUserGeoInitializer", fOutputColName.Data());
            fHasDetPrm = false;
        }
    }

    // target parameter
    if (!fTargetParameters) {
        fHasTargetPrm = false;
    } else {
        fHasTargetPrm = true;
        if ((*fTargetParameters)->GetEntriesFast() == 0) {
            Warning("Init", "cannot find target node. please check TUserGeoInitializer");
            fHasTargetPrm = false;
        } else {
            // use the first target node
            TParameterObject *inPrm = static_cast<TParameterObject *>((*fTargetParameters)->At(0));
            TTargetParameter *prm = dynamic_cast<TTargetParameter *>(inPrm);
            fTargetParameter = prm;
            Info("Init", "use first target %s information (only use the position)", prm->GetTargetName().Data());
        }
    }

    if (!fHasDetPrm || !fHasTargetPrm) {
        Warning("Init", "not initialized by TUserGeoInitializer, not calculate geometry info");
    }

    // check if input collection is valid or not
    const TClass *const cl1 = (*fInData1)->GetClass();
    const TClass *const cl2 = (*fInData2)->GetClass();
    const TClass *const cl3 = (*fInData3)->GetClass();
    fInputHasData = (cl1->InheritsFrom(art::TTimingChargeData::Class())) && (cl2->InheritsFrom(art::TTimingChargeData::Class())) && (cl3->InheritsFrom(art::TTimingChargeData::Class()));

    if (!fInputHasData) {
        SetStateError("contents of input array must inherit from art::TTimingChargeData");
        return;
    }

    fOutData = new TClonesArray("art::crib::TTelescopeData");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);

    gRandom->SetSeed(time(nullptr));
}

void TTelescopeProcessor::Process() {
    fOutData->Clear("C");

    Int_t nData1 = (*fInData1)->GetEntriesFast();
    Int_t nData2 = (*fInData2)->GetEntriesFast();
    Int_t nData3 = (*fInData3)->GetEntriesFast();

    // if no hit, do nothing
    if (nData1 == 0 && nData2 == 0 && nData3 == 0) {
        return;
    }

    TTelescopeData *outData = static_cast<TTelescopeData *>(fOutData->ConstructedAt(0));
    outData->Clear();
    outData->SetID(0); // always 0
    outData->SetTelID(fTelID);
    if (fHasDetPrm && fHasTargetPrm) {
        outData->SetN(fDetParameter->GetN());
    } else {
        outData->SetN(DEFAULT_SSD_MAX_NUMBER);
    }

    Double_t Etotal = 0.0;
    Double_t dE = 0.0;

    // dEX process
    //    search the highest channel
    Int_t dEXID = -1;
    Double_t dEX_tmp = 0.0;
    for (Int_t iData1 = 0; iData1 < nData1; ++iData1) {
        const TDataObject *const inData1 = static_cast<TDataObject *>((*fInData1)->At(iData1));
        const TTimingChargeData *const Data1 = dynamic_cast<const TTimingChargeData *>(inData1);
        /// need to update: care about overflow event more clevar
        if (dEX_tmp < Data1->GetCharge() && Data1->GetCharge() < 50.0) { // overflow signal should have very large value
            dEXID = iData1;
            dEX_tmp = Data1->GetCharge();
        }
    }

    //    fill the highest channel
    if (nData1 != 0 && dEXID != -1) {
        const TDataObject *const inData1 = static_cast<TDataObject *>((*fInData1)->At(dEXID));
        const TTimingChargeData *const Data1 = dynamic_cast<const TTimingChargeData *>(inData1);
        Double_t energy = Data1->GetCharge();
        Double_t timing = Data1->GetTiming();

        // judge if the event is pedestal or not
        if (fHasDetPrm) {
            if (energy < fDetParameter->GetPedestal(0)) { // first layer
                energy = 0.0;
            }
        }

        outData->SetXID(Data1->GetDetID());
        if (fIsDSSSD) {
            outData->SetdE(energy); // now fdE is used the value of dEX
        } else {
            dE += energy;
        }
        outData->SetdEX(energy);
        outData->SetTelXTiming(timing);
        outData->PushEnergyArray(energy);
        outData->PushTimingArray(timing);
        Etotal += energy;
    } else {
        outData->PushEnergyArray(0.0);
        outData->PushTimingArray(kInvalidD);
    }

    // dEY process
    //    search the highest channel
    Int_t dEYID = -1;
    Double_t dEY_tmp = 0.0;
    for (Int_t iData2 = 0; iData2 < nData2; ++iData2) {
        const TDataObject *const inData2 = static_cast<TDataObject *>((*fInData2)->At(iData2));
        const TTimingChargeData *const Data2 = dynamic_cast<const TTimingChargeData *>(inData2);
        /// need to update: care about overflow event more clevar
        if (dEY_tmp < Data2->GetCharge() && Data2->GetCharge() < 50.0) { // overflow signal should have very large value
            dEYID = iData2;
            dEY_tmp = Data2->GetCharge();
        }
    }

    //    fill the highest channel
    if (nData2 != 0 && dEYID != -1) {
        const TDataObject *const inData2 = static_cast<TDataObject *>((*fInData2)->At(dEYID));
        const TTimingChargeData *const Data2 = dynamic_cast<const TTimingChargeData *>(inData2);
        Double_t energy = Data2->GetCharge();
        Double_t timing = Data2->GetTiming();

        // judge if this event is pedestal or not
        if (fHasDetPrm) {
            if (fIsDSSSD) {
                if (energy < fDetParameter->GetPedestal(0)) { // first layer
                    energy = 0.0;
                }
            } else {
                if (energy < fDetParameter->GetPedestal(1)) { // second layer
                    energy = 0.0;
                }
            }
        }

        outData->SetYID(Data2->GetDetID());
        if (!fIsDSSSD) {
            dE += energy;
            outData->SetdE(dE);
            outData->PushEnergyArray(energy);
            outData->PushTimingArray(timing);
            Etotal += energy;
        }
        outData->SetdEY(energy);
        outData->SetTelYTiming(timing);
    } else if (!fIsDSSSD) {
        outData->PushEnergyArray(0.0);
        outData->PushTimingArray(kInvalidD);
    }

    // geometry process
    /// for the definition, please check https://okawak.github.io/artemis_crib/example/simulation/geometry/index.html
    if (IsValid(outData->GetXID()) && IsValid(outData->GetYID()) && fHasDetPrm && fHasTargetPrm) {
        Double_t target_z = fTargetParameter->GetZ();
        TVector3 center_rot(fDetParameter->GetCenterRotPos(0), fDetParameter->GetCenterRotPos(1), fDetParameter->GetCenterRotPos(2));
        TVector3 offset(fDetParameter->GetOffset(0), fDetParameter->GetOffset(1), fDetParameter->GetOffset(2));
        DoubleVec_t size = fDetParameter->GetSize();
        IntVec_t stripN = fDetParameter->GetStripNum();
        Int_t xid = outData->GetXID();
        Int_t yid = outData->GetYID();

        // (x, y) position before rotation
        Double_t x = size[0] * (2.0 * (Double_t)xid + 1.0 - (Double_t)stripN[0]) / (2.0 * (Double_t)stripN[0]);
        if (fUseRandom) {
            x += gRandom->Uniform(-size[0] / (2.0 * (Double_t)stripN[0]), size[0] / (2.0 * (Double_t)stripN[0]));
        }
        Double_t y = size[1] * (2.0 * (Double_t)yid + 1.0 - (Double_t)stripN[1]) / (2.0 * (Double_t)stripN[1]);
        if (fUseRandom) {
            y += gRandom->Uniform(-size[1] / (2.0 * (Double_t)stripN[1]), size[1] / (2.0 * (Double_t)stripN[1]));
        }

        // rotate the detector
        TVector3 det_pos(x, y, fDetParameter->GetDistance());
        det_pos += offset;
        det_pos.RotateY(fDetParameter->GetAngle()); // radian
        det_pos += center_rot;
        outData->SetPosition(det_pos);

        TVector3 target(0., 0., target_z);
        TVector3 beam(0., 0., 1.); // assume beam center!
        Double_t theta = beam.Angle(det_pos - target);
        outData->SetTheta_L(theta * TMath::RadToDeg());
    }

    // Thick SSDs process
    Double_t E = 0.0;
    Int_t NE;
    if (fHasDetPrm && fHasTargetPrm) {
        NE = fIsDSSSD ? fDetParameter->GetN() - 1 : fDetParameter->GetN() - 2;
    } else {
        NE = fIsDSSSD ? DEFAULT_SSD_MAX_NUMBER - 1 : DEFAULT_SSD_MAX_NUMBER - 2;
    }
    Int_t index = fIsDSSSD ? 1 : 2;

    if (NE < nData3) {
        Warning("process", "Thick SSD number is wrong between actual data and define in get/expname.yaml");
    }

    for (Int_t iE = 0; iE < NE; iE++) {
        Int_t itr = -1;
        // get "iE" index object
        /// in case single-pad SSD don't have data
        for (Int_t iData3 = 0; iData3 < nData3; ++iData3) {
            const TDataObject *const inData3 = static_cast<TDataObject *>((*fInData3)->At(iData3));
            const TTimingChargeData *const Data3 = dynamic_cast<const TTimingChargeData *>(inData3);
            if (Data3->GetDetID() == iE) {
                itr = iData3;
                break;
            }
        }

        if (itr < 0) {
            outData->PushEnergyArray(0.0);
            outData->PushTimingArray(kInvalidD);
        } else {
            const TDataObject *const inData3 = static_cast<TDataObject *>((*fInData3)->At(itr));
            const TTimingChargeData *const Data3 = dynamic_cast<const TTimingChargeData *>(inData3);
            Double_t energy = Data3->GetCharge();
            Double_t timing = Data3->GetTiming();

            // judge if the event is pedestal or not
            if (fHasDetPrm) {
                if (energy < fDetParameter->GetPedestal(index + iE)) {
                    energy = 0.0;
                }
            }

            outData->PushEnergyArray(energy);
            outData->PushTimingArray(timing);
            E += energy;
            Etotal += energy;
        }
    }

    outData->SetE(E);
    outData->SetEtotal(Etotal);
}
