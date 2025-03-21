/**
 * @file    TMUXDataMappingProcessor.cc
 * @brief   Implementation of TMUXDataMappingProcessor for mapping categorized data.
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 09:47:17
 * @note    last modified: 2025-03-05 18:32:52
 * @details
 */

#include "TMUXDataMappingProcessor.h"
#include "../TProcessorUtil.h"

#include "TMUXData.h"
#include <TCategorizedData.h>
#include <TRawDataObject.h>
#include <constant.h>

/// ROOT macro for class implementation
ClassImp(art::crib::TMUXDataMappingProcessor);

namespace art::crib {

/**
 * @details
 * The constructor initializes the processor by setting up input/output
 * collection names and default parameter values. These values can be
 * overridden via the steering file or configuration options.
 */
TMUXDataMappingProcessor::TMUXDataMappingProcessor()
    : fCategorizedData(nullptr), fOutData(nullptr) {
    RegisterInputCollection("CategorizedDataName", "name of the segmented data",
                            fCategorizedDataName, TString("catdata"));
    RegisterOutputCollection("OutputCollection", "name of the output branch",
                             fOutputColName, TString("mux"));

    RegisterProcessorParameter("CatID", "Category ID", fCatID, -1);
}

/**
 * @details
 * The destructor ensures that dynamically allocated memory for `fOutData`
 * is properly released to prevent memory leaks.
 */
TMUXDataMappingProcessor::~TMUXDataMappingProcessor() {
    delete fOutData;
    fOutData = nullptr;
}

/**
 * @details
 * During initialization, the processor retrieves the input categorized data
 * collection from the provided event collection and validates its type.
 * It also initializes the output data array (`fOutData`) as a TClonesArray
 * of TMUXData objects.
 *
 * If the required input collection or parameters are invalid, an error
 * state is set, and processing will not proceed.
 */
void TMUXDataMappingProcessor::Init(TEventCollection *col) {
    // Categorized data initialization
    auto result = util::GetInputObject<TCategorizedData>(
        col, fCategorizedDataName, "art::TCategorizedData");

    if (std::holds_alternative<TString>(result)) {
        SetStateError(std::get<TString>(result));
        return;
    }
    fCategorizedData = std::get<TCategorizedData **>(result);

    // CatID validation
    if (fCatID < 0) {
        SetStateError("CatID must be set in the steering file");
        return;
    }

    delete fOutData; // Release memory allocated for fOutData if it exists
    fOutData = new TClonesArray("art::crib::TMUXData");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);
    Info("Init", "%s -> %s, CatID = %d",
         fCategorizedDataName.Data(), fOutputColName.Data(), fCatID);
}

/**
 * @details
 * Processes the categorized data and maps it to TMUXData objects. For each
 * detector, the method extracts relevant data and creates a TMUXData object
 * in the output array.
 *
 * If the input collection is not initialized or the specified category is
 * not found, appropriate warnings are logged, and the method exits early.
 */
void TMUXDataMappingProcessor::Process() {
    fOutData->Clear("C");
    if (!fCategorizedData) {
        Warning("Process", "No CategorizedData object");
        return;
    }

    const auto *cat_array = (*fCategorizedData)->FindCategory(fCatID);
    if (!cat_array)
        return;

    const int nDet = cat_array->GetEntriesFast();
    int counter = 0;
    for (int iDet = 0; iDet < nDet; ++iDet) {
        const auto *det_array = static_cast<const TObjArray *>(cat_array->At(iDet));
        if (!det_array)
            continue;

        auto *outData = static_cast<TMUXData *>(fOutData->ConstructedAt(counter));
        counter++;
        int detID = ProcessDetectorData(det_array, outData);
        outData->SetID(detID);
    }
}

/**
 * @details
 * Processes data for a single detector and maps it to a TMUXData object.
 * This method extracts energy, position, and timing data from the raw
 * detector arrays and assigns them to the corresponding fields in `mux`.
 *
 * Special handling is applied to timing data to aggregate multiple values.
 */
int TMUXDataMappingProcessor::ProcessDetectorData(const TObjArray *det_array, TMUXData *mux) {
    double raw_data[TMUXData::kNRAW] = {kInvalidD, kInvalidD, kInvalidD, kInvalidD, kInvalidD};
    int detID = kInvalidI;
    for (int iType = 0; iType < TMUXData::kNRAW; ++iType) {
        const auto *data_array = static_cast<const TObjArray *>(det_array->At(iType));
        if (!data_array)
            continue;

        if (iType < TMUXData::kNRAW - 1) {
            // index = 0--3 assuming ADC data
            if (data_array->GetEntriesFast() != 1) {
                Warning("ProcessDetectorData", "MUX data [%d] size is not 1", iType);
                continue;
            }
            const auto *data = dynamic_cast<const TRawDataObject *>(data_array->At(0));
            if (data) {
                raw_data[iType] = data->GetValue();
                detID = data->GetDetID();
            }
        } else { // Timing treatment for MHTDC
            const int nData = data_array->GetEntriesFast();
            for (int iData = 0; iData < nData; ++iData) {
                const auto *data = dynamic_cast<const TRawDataObject *>(data_array->At(iData));
                if (data) {
                    if (!IsValid(raw_data[iType]))
                        raw_data[iType] = data->GetValue();
                    mux->PushTiming(data->GetValue());
                    detID = data->GetDetID();
                }
            }
        }
    }
    mux->SetE1(raw_data[0]);
    mux->SetE2(raw_data[1]);
    mux->SetP1(raw_data[2]);
    mux->SetP2(raw_data[3]);
    mux->SetTrig(raw_data[4]);

    return detID;
}

} // namespace art::crib
