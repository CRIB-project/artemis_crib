/**
 * @file    TMUXCalibrationProcessor.cc
 * @brief   Implementation of the TMUXCalibrationProcessor class for calibrating timing, charge, and position data.
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 11:09:46
 * @note    last modified: 2025-01-02 22:33:28
 * @details
 */

#include "TMUXCalibrationProcessor.h"
#include "../TProcessorUtil.h"

#include "TMUXData.h"
#include "TMUXPositionConverter.h"
#include <TAffineConverter.h>
#include <TRandom.h>
#include <TTimingChargeData.h>
#include <constant.h>

/// ROOT macro for class implementation
ClassImp(art::crib::TMUXCalibrationProcessor);

namespace art::crib {

namespace {
/**
 * @brief Constant representing no conversion.
 */
const TString kNoConversion = "no_conversion";
/**
 * @brief Threshold for applying reflection to the detector ID.
 */
constexpr int kReflectionThreshold = 8;
} // namespace

TMUXCalibrationProcessor::TMUXCalibrationProcessor()
    : fInData(nullptr), fOutData(nullptr),
      fTimingConverterArray(nullptr), fChargeConverterArray(nullptr),
      fPositionConverterArray(nullptr) {
    RegisterInputCollection("InputCollection", "Array of TMUXData objects",
                            fInputColName, TString("mux_raw"));
    RegisterOutputCollection("OutputCollection", "Output array of TTimingChargeData objects",
                             fOutputColName, TString("mux_cal"));

    RegisterProcessorParameter("TimingConverterArray",
                               "Timing parameter object of TAffineConverter",
                               fTimingConverterArrayName, kNoConversion);
    RegisterProcessorParameter("ChargeConverterArray",
                               "Energy parameter object of TAffineConverter",
                               fChargeConverterArrayName, kNoConversion);
    RegisterProcessorParameter("PositionConverterArray",
                               "Position parameter object of TMUXPositionConverter",
                               fPositionConverterArrayName, kNoConversion);

    RegisterProcessorParameter("HasReflection", "Reverse strip order (0--7) if true",
                               fHasReflection, kFALSE);
    RegisterProcessorParameter("InputIsDigital", "Add randomness if true",
                               fInputIsDigital, kTRUE);
}

TMUXCalibrationProcessor::~TMUXCalibrationProcessor() {
    delete fOutData;
    fOutData = nullptr;
}

/**
 * @details
 * This method initializes the processor by:
 * - Retrieving the input and output data collections based on user configuration.
 * - Setting up the converter arrays (`fTimingConverterArray`, `fChargeConverterArray`, `fPositionConverterArray`)
 *   based on the specified parameters.
 * - Logging warnings if parameters are not set or if required converters are missing.
 *
 * If the required position converter array is not provided, the processor is put into an error state.
 */
void TMUXCalibrationProcessor::Init(TEventCollection *col) {
    // lambda function for initialize converter arrays
    auto initConverterArray = [this, col](const TString &name, TClonesArray *&array, const char *paramName) {
        if (name == kNoConversion) {
            Warning("Init", "Parameter '%s' is not set. Using no conversion.", paramName);
            array = nullptr;
        } else {
            array = util::GetParameterObject(col, name);
        }
    };

    auto result = util::GetInputObject<TClonesArray>(
        col, fInputColName, "TClonesArray", "art::crib::TMUXData");

    if (std::holds_alternative<TString>(result)) {
        SetStateError(std::get<TString>(result));
        return;
    }
    fInData = std::get<TClonesArray *>(result);
    Info("Init", "%s => %s", fInputColName.Data(), fOutputColName.Data());

    initConverterArray(fTimingConverterArrayName, fTimingConverterArray, "TimingConverterArray");
    initConverterArray(fChargeConverterArrayName, fChargeConverterArray, "ChargeConverterArray");

    if (fPositionConverterArrayName == kNoConversion) {
        SetStateError("Position parameters are required");
        return;
    }
    fPositionConverterArray = util::GetParameterObject(col, fPositionConverterArrayName);

    fOutData = new TClonesArray("art::TTimingChargeData");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);
}

/**
 * @details
 * The `Process` method performs the following steps:
 * 1. Clears the output data collection to prepare for new entries.
 * 2. Checks the validity of the input data:
 *    - If no data is present (`nData == 0`), the method exits as this is a valid condition.
 *    - If multiple data entries are found, a warning is logged and the method exits, as multi-data processing is not supported.
 * 3. Retrieves the first entry in the input collection and casts it to `TMUXData`.
 * 4. Converts the position data (`P1`) to a detector ID.
 * 5. Applies reflection to the detector ID if `fHasReflection` is enabled.
 * 6. Calibrates the charge and timing values using the respective converter arrays.
 * 7. Stores the calibrated data in the output collection.
 */
void TMUXCalibrationProcessor::Process() {
    fOutData->Clear("C");
    if (!fInData) {
        Warning("Process", "No Input Data object");
        return;
    }

    const int nData = fInData->GetEntriesFast();
    if (nData == 0)
        return;
    else if (nData > 1) {
        Warning("Process", "It doesn't support multi-data currently");
        return;
    }

    const auto *data = dynamic_cast<const TMUXData *>(fInData->At(0));
    if (!data)
        return;

    // int mux_detid = data->GetID();
    int hit1_detid = ConvertPosition(data->GetP1(), 0);
    // int hit2_detid = ConvertPosition(data->GetP2(), 1); // not implemented

    if (!IsValid(hit1_detid))
        return;

    if (fHasReflection && hit1_detid >= 0 && hit1_detid < kReflectionThreshold) {
        hit1_detid = kReflectionThreshold - 1 - hit1_detid;
    }

    auto *outData = static_cast<TTimingChargeData *>(fOutData->ConstructedAt(0));
    outData->SetID(hit1_detid);
    outData->SetCharge(CalibrateValue(data->GetE1(), hit1_detid, fChargeConverterArray));
    outData->SetTiming(CalibrateValue(data->GetTrig(), hit1_detid, fTimingConverterArray));
}

/**
 * @details
 * Converts a raw position value to a detector ID by using the position converter array.
 * If the position converter array is not available or the conversion fails, an invalid value (`kInvalidD`) is returned.
 */
double TMUXCalibrationProcessor::ConvertPosition(double pos, int id) const {
    if (!fPositionConverterArray)
        return kInvalidD;
    auto *converter = static_cast<TMUXPositionConverter *>(fPositionConverterArray->At(id));
    return converter ? converter->Convert(pos) : kInvalidD;
}

/**
 * @details
 * Calibrates a raw value (such as timing or charge) using the specified converter array.
 * If the input is digital (`fInputIsDigital`), a random value is added to the raw input to simulate analog noise.
 * If the converter array is not available or the conversion fails, the raw value is returned unchanged.
 */
double TMUXCalibrationProcessor::CalibrateValue(double raw, int id, const TClonesArray *converterArray) const {
    raw += (fInputIsDigital ? gRandom->Uniform() : 0);
    if (!converterArray)
        return raw;
    auto *converter = static_cast<TAffineConverter *>(converterArray->At(id));
    return converter ? converter->Convert(raw) : raw;
}

} // namespace art::crib
