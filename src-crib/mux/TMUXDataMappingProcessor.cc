/**
 * @file    TMUXDataMappingProcessor.cc
 * @brief   Implementation of TMUXDataMappingProcessor for mapping categorized data.
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 09:47:17
 * @note    last modified: 2024-12-31 22:51:19
 * @details
 */

#include "TMUXDataMappingProcessor.h"

#include "TMUXData.h"
#include <TCategorizedData.h>
#include <TRawDataObject.h>
#include <constant.h>

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
    auto cat_ref = col->GetObjectRef(fCategorizedDataName);
    if (!cat_ref) {
        SetStateError(Form("No input collection '%s'", fCategorizedDataName.Data()));
        return;
    }

    auto cat_obj = static_cast<TObject *>(*cat_ref);
    if (!cat_obj->InheritsFrom("art::TCategorizedData")) {
        SetStateError(Form("Invalid input collection '%s': not TCategorizedData",
                           fCategorizedDataName.Data()));
        return;
    }
    fCategorizedData = static_cast<TCategorizedData *>(cat_obj);

    // CatID validation
    if (fCatID < 0) {
        SetStateError("CatID must be set in the steering file");
        return;
    }

    delete fOutData; // Release memory allocated for fOutData if it exists
    fOutData = new TClonesArray("art::TMUXData");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);
    Info("Init", "%s -> %s, CatID = %d",
         fCategorizedDataName.Data(), fOutputColName.Data(), fCatID);
}

/**
 * @details
 * The `Process` method iterates over categorized data, processes each
 * detector's data using `ProcessDetectorData`, and populates the output
 * array (`fOutData`) with TMUXData objects.
 *
 * If the required input collection is not initialized or the specified
 * category ID is not found, appropriate warnings are logged.
 */
void TMUXDataMappingProcessor::Process() {
    fOutData->Clear("C");
    if (!fCategorizedData) {
        Warning("Process", "No CategorizedData object");
        return;
    }

    const auto *cat_array = fCategorizedData->FindCategory(fCatID);
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

int TMUXDataMappingProcessor::ProcessDetectorData(const TObjArray *det_array, TMUXData *mux) {
    double raw_data[TMUXData::kNRAW] = {kInvalidD, kInvalidD, kInvalidD, kInvalidD, kInvalidD};
    int detID = kInvalidI;
    for (int iType = 0; iType < TMUXData::kNRAW; ++iType) {
        const auto *data_array = static_cast<const TObjArray *>(det_array->At(iType));
        if (!data_array)
            continue;

        if (iType < TMUXData::kNRAW - 1 && data_array->GetEntriesFast() == 1) {
            const auto *data = dynamic_cast<const TRawDataObject *>(data_array->At(0));
            if (data) {
                raw_data[iType] = data->GetValue();
                detID = data->GetDetID();
            }
        } else if (iType == TMUXData::kNRAW - 1) {
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
