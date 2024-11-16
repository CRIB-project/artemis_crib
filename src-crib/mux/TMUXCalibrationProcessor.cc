/**
 * @file    TMUXCalibrationProcessor.cc
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 11:09:46
 * @note    last modified: 2024-08-23 21:28:40
 * @details treat only pos1 and ene1
 */

#include "TMUXCalibrationProcessor.h"

#include "TConverterUtil.h"
#include "TMUXData.h"
#include "TMUXPositionConverter.h"
#include "TTimingChargeData.h"
#include <TRandom.h>

using art::crib::TMUXCalibrationProcessor;

namespace {
const TString DEFAULT_CONV_NAME("no_conversion");
}

ClassImp(TMUXCalibrationProcessor);

// Default constructor
TMUXCalibrationProcessor::TMUXCalibrationProcessor()
    : fInData(nullptr), fOutData(nullptr),
      fTimingConverterArray(nullptr), fChargeConverterArray(nullptr),
      fPositionConverterArray(nullptr),
      fInputHasMUXData(kFALSE) {
    RegisterInputCollection("InputCollection",
                            "array of objects inheriting from art::crib::TMUXData",
                            fInputColName, TString("mux_raw"));
    RegisterOutputCollection("OutputCollection", "output class will be art::TTimingChargeData",
                             fOutputColName, TString("mux_cal"));
    RegisterProcessorParameter("TimingConverterArray",
                               "normally output of TAffineConverterArrayGenerator",
                               fTimingConverterArrayName, DEFAULT_CONV_NAME);
    RegisterProcessorParameter("ChargeConverterArray",
                               "normally output of TAffineConverterArrayGenerator",
                               fChargeConverterArrayName, DEFAULT_CONV_NAME);
    RegisterProcessorParameter("PositionConverterArray",
                               "normally output of TAffineConverterArrayGenerator",
                               fPositionConverterArrayName, DEFAULT_CONV_NAME);
    RegisterProcessorParameter("HasReflection", "whether strip is normal order or not",
                               fHasReflection, kFALSE);
    RegisterProcessorParameter("InputIsDigital", "whether input is digital or not",
                               fInputIsDigital, kTRUE);
    RegisterProcessorParameter("Sparse", "hit will be inserted at the index corresponding to its id if sparse is 0, otherwize hit will be added ordinaly (default: 1)",
                               fIsSparse, kTRUE);
}

TMUXCalibrationProcessor::~TMUXCalibrationProcessor() {
    delete fOutData;
    delete fTimingConverterArray;
    delete fChargeConverterArray;
    delete fPositionConverterArray;
    fOutData = nullptr;
    fTimingConverterArray = nullptr;
    fChargeConverterArray = nullptr;
    fPositionConverterArray = nullptr;
}

void TMUXCalibrationProcessor::Init(TEventCollection *col) {
    Info("Init", "%s => %s", fInputColName.Data(), fOutputColName.Data());
    fInData = reinterpret_cast<TClonesArray **>(col->GetObjectRef(fInputColName.Data()));
    if (!fInData) {
        SetStateError(TString::Format("input not found: %s", fInputColName.Data()));
        return;
    }

    const TClass *const cl = (*fInData)->GetClass();
    fInputHasMUXData = cl->InheritsFrom(art::crib::TMUXData::Class());

    if (!fInputHasMUXData) {
        SetStateError("contents of input array must inherit from art::crib::TMUXData");
        return;
    }

    if (fTimingConverterArrayName.CompareTo(DEFAULT_CONV_NAME)) {
        TConverterUtil::SetConverterArray(&fTimingConverterArray, fTimingConverterArrayName, col);
    }

    if (fChargeConverterArrayName.CompareTo(DEFAULT_CONV_NAME)) {
        TConverterUtil::SetConverterArray(&fChargeConverterArray, fChargeConverterArrayName, col);
    }

    if (fPositionConverterArrayName.CompareTo(DEFAULT_CONV_NAME)) {
        TConverterUtil::SetConverterArray(&fPositionConverterArray, fPositionConverterArrayName, col);
    }

    if (!fTimingConverterArray || !fChargeConverterArray || !fPositionConverterArray) {
        SetStateError("no timing, charge and position converter registered.");
        return;
    }

    fOutData = new TClonesArray("art::TTimingChargeData");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);
}

void TMUXCalibrationProcessor::Process() {
    fOutData->Clear("C");

    for (Int_t iData = 0, nData = (*fInData)->GetEntriesFast();
         iData != nData; ++iData) {

        const TDataObject *const inData = static_cast<TDataObject *>((*fInData)->At(iData));
        const TMUXData *const DataM = dynamic_cast<const TMUXData *>(inData);

        // process of position
        if (!fPositionConverterArray)
            return;
        Double_t pos1_raw = DataM->GetP1();
        Int_t detID = kInvalidI;
        for (Int_t i = 0; i < (Int_t)(fPositionConverterArray->size()); ++i) {
            detID = fPositionConverterArray->at(i)->Convert(pos1_raw);
            if (detID == 1) {
                detID = i;
                break;
            } else {
                detID = kInvalidI;
            }
        }
        if (!IsValid(detID))
            continue;
        // if(detID == kInvalidI) continue;

        if (fHasReflection) {
            for (Int_t i = 0; i < 8; ++i) {
                if (detID == i) {
                    detID = 7 - i;
                    break;
                }
            }
        }

        TTimingChargeData *outData = static_cast<TTimingChargeData *>(fOutData->ConstructedAt(iData));
        outData->SetDetID(detID);

        // TDataObject *const outDataID = dynamic_cast<TDataObject*>(outData);
        // outDataID->SetID(detID);

        // process of charge and timing
        Double_t charge = DataM->GetE1() + (fInputIsDigital ? gRandom->Uniform() : 0);
        if (fChargeConverterArray && detID >= (Int_t)fChargeConverterArray->size()) {
            SetStateError(TString::Format("size of ChargeConverterArray (%d) is smaller than detID (%d)", (Int_t)fChargeConverterArray->size(), detID));
            return;
        }
        const Double_t outputQ =
            fChargeConverterArray ? fChargeConverterArray->at(detID)->Convert(charge) : charge;
        ICharge *const outDataQ = dynamic_cast<ICharge *>(outData);
        outDataQ->SetCharge(outputQ);

        const Double_t timing = DataM->GetTrig() + (fInputIsDigital ? gRandom->Uniform() : 0);
        if (fTimingConverterArray && detID >= (Int_t)fTimingConverterArray->size()) {
            SetStateError(TString::Format("size of TimingConverterArray (%d) is smaller than detID (%d)", (Int_t)fTimingConverterArray->size(), detID));
            return;
        }
        const Double_t outputT =
            fTimingConverterArray ? fTimingConverterArray->at(detID)->Convert(timing) : timing;
        ITiming *const outDataT = dynamic_cast<ITiming *>(outData);
        outDataT->SetTiming(outputT);
    }

    if (fIsSparse) {
        // sort data in the same event in ascending order of timing
        TTimingChargeData::SetSortType(TTimingChargeData::kTiming);
        TTimingChargeData::SetSortOrder(TTimingChargeData::kASC);
        fOutData->Sort();
        fOutData->Compress();
    } else {
        for (Int_t i = 0, n = fOutData->GetEntriesFast(); i != n; ++i) {
            fOutData->ConstructedAt(i);
        }
    }
}
