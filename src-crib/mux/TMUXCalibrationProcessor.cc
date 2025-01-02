/**
 * @file    TMUXCalibrationProcessor.cc
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 11:09:46
 * @note    last modified: 2025-01-02 18:33:28
 * @details treat only pos1 and ene1
 */

#include "TMUXCalibrationProcessor.h"
#include "../TProcessorUtil.h"

#include "TMUXData.h"
#include "TMUXPositionConverter.h"
#include <TAffineConverter.h>
#include <TRandom.h>
#include <TTimingChargeData.h>
#include <constant.h>

ClassImp(art::crib::TMUXCalibrationProcessor);

namespace art::crib {
// Default constructor
TMUXCalibrationProcessor::TMUXCalibrationProcessor()
    : fInData(nullptr), fOutData(nullptr),
      fTimingConverterArray(nullptr), fChargeConverterArray(nullptr),
      fPositionConverterArray(nullptr) {
    RegisterInputCollection("InputCollection",
                            "array of objects inheriting from art::crib::TMUXData",
                            fInputColName, TString("mux_raw"));
    RegisterOutputCollection("OutputCollection", "output class will be art::TTimingChargeData",
                             fOutputColName, TString("mux_cal"));

    RegisterProcessorParameter("TimingConverterArray",
                               "normally output of TAffineConverterArrayGenerator",
                               fTimingConverterArrayName, TString("no_conversion"));
    RegisterProcessorParameter("ChargeConverterArray",
                               "normally output of TAffineConverterArrayGenerator",
                               fChargeConverterArrayName, TString("no_conversion"));
    RegisterProcessorParameter("PositionConverterArray",
                               "normally output of TAffineConverterArrayGenerator",
                               fPositionConverterArrayName, TString("no_conversion"));

    RegisterProcessorParameter("HasReflection", "whether strip is normal order or not",
                               fHasReflection, kFALSE);
    RegisterProcessorParameter("InputIsDigital", "whether input is digital or not",
                               fInputIsDigital, kTRUE);
}

TMUXCalibrationProcessor::~TMUXCalibrationProcessor() {
    delete fOutData;
    fOutData = nullptr;
}

void TMUXCalibrationProcessor::Init(TEventCollection *col) {
    // Input data initialization
    auto result = util::GetInputObject<TClonesArray>(
        col, fInputColName, "TClonesArray", "art::crib::TMUXData");

    if (std::holds_alternative<TString>(result)) {
        SetStateError(std::get<TString>(result));
        return;
    }
    fInData = std::get<TClonesArray *>(result);
    Info("Init", "%s => %s", fInputColName.Data(), fOutputColName.Data());

    if (fTimingConverterArrayName.CompareTo("no_conversion")) {
        fTimingConverterArray = util::GetParameterObject(col, fTimingConverterArrayName);
    }

    if (fChargeConverterArrayName.CompareTo("no_conversion")) {
        fChargeConverterArray = util::GetParameterObject(col, fChargeConverterArrayName);
    }

    if (fPositionConverterArrayName.CompareTo("no_conversion")) {
        fPositionConverterArray = util::GetParameterObject(col, fPositionConverterArrayName);
    } else {
        SetStateError("Position parameters are necessary");
        return;
    }

    fOutData = new TClonesArray("art::TTimingChargeData");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);
}

void TMUXCalibrationProcessor::Process() {
    fOutData->Clear("C");
    if (!fInData) {
        Warning("Process", "No Input Data object");
        return;
    }

    int nData = fInData->GetEntriesFast();
    if (nData == 0)
        return;
    if (nData > 1) {
        Warning("Process", "It doesn't support multi-data currently");
        return;
    }

    const auto *inData = static_cast<const TDataObject *>(fInData->At(0));
    const auto *data = dynamic_cast<const TMUXData *>(inData);
    if (!data)
        return;

    // int mux_detid = data->GetID();
    int hit1_detid = static_cast<TMUXPositionConverter *>(fPositionConverterArray->At(0))
                         ->Convert(data->GetP1());
    // int hit2_detid = static_cast<TMUXPositionConverter *>(fPositionConverterArray->At(1))
    //                      ->Convert(data->GetP2());

    if (!IsValid(hit1_detid))
        return;

    if (fHasReflection) {
        for (int i = 0; i < 8; ++i) {
            if (hit1_detid == i) {
                hit1_detid = 7 - i;
                break;
            }
        }
    }

    auto *outData = static_cast<TTimingChargeData *>(fOutData->ConstructedAt(0));
    outData->SetID(hit1_detid);

    // process of energy and timing
    double e1_raw = data->GetE1() + (fInputIsDigital ? gRandom->Uniform() : 0);
    double e1_cal = e1_raw;
    if (fChargeConverterArray) {
        auto *energy_prm = static_cast<TAffineConverter *>(fChargeConverterArray->At(hit1_detid));
        if (energy_prm) {
            e1_cal = energy_prm->Convert(e1_raw);
        }
    }
    outData->SetCharge(e1_cal);

    // use first timing
    double t1_raw = data->GetTrig() + (fInputIsDigital ? gRandom->Uniform() : 0);
    double t1_cal = t1_raw;
    if (fTimingConverterArray) {
        auto *timing_prm = static_cast<TAffineConverter *>(fTimingConverterArray->At(hit1_detid));
        if (timing_prm) {
            t1_cal = timing_prm->Convert(t1_raw);
        }
    }
    outData->SetTiming(t1_cal);
}
} // namespace art::crib
