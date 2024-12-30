/**
 * @file    TMapSelector.cc
 * @brief   extract one catid data
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-12-23 11:56:21
 * @note    last modified: 2024-12-30 16:40:41
 * @details
 */

#include "TMapSelector.h"

#include <TCategorizedData.h>
#include <TRawDataObject.h>
#include <TSimpleData.h>

ClassImp(art::crib::TMapSelector);

namespace art::crib {
TMapSelector::TMapSelector() : fCategorizedData(nullptr), fOutData(nullptr) {
    RegisterInputCollection("CategorizedDataName", "name of the segmented data",
                            fCategorizedDataName, TString("catdata"));
    RegisterOutputCollection("OutputCollection", "name of the output branch",
                             fOutputColName, TString("channel"));

    IntVec_t init_i_vec;
    RegisterProcessorParameter("CatID", "Categorized ID, [cid, detid, type]",
                               fCatID, init_i_vec);
}

TMapSelector::~TMapSelector() {
    // release fOutData in TEventCollection
}

void TMapSelector::Init(TEventCollection *col) {
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
    if (fCatID.size() != 3) {
        SetStateError("CatID must contain exactly 3 elements: [cid, detid, type]");
        return;
    }

    fOutData = new TClonesArray("art::TSimpleData");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);
    Info("Init", "%s -> %s, CatID = %d",
         fCategorizedDataName.Data(), fOutputColName.Data(), fCatID[0]);
}

void TMapSelector::Process() {
    fOutData->Clear("C");
    if (!fCategorizedData) {
        Warning("Process", "No CategorizedData object");
        return;
    }

    auto *cat_array = fCategorizedData->FindCategory(fCatID[0]);
    if (!cat_array) {
        // Warning("Process", "No data having catid = %d", fCatID[0]);
        return;
    }

    const int nDet = cat_array->GetEntriesFast();
    int counter = 0;
    for (int iDet = 0; iDet < nDet; ++iDet) {
        auto *det_array = static_cast<TObjArray *>(cat_array->At(iDet));
        if (!det_array)
            continue;
        auto *data_array = static_cast<TObjArray *>(det_array->At(fCatID[2]));
        if (!data_array)
            continue;
        const int nData = data_array->GetEntriesFast();
        for (int iData = 0; iData < nData; ++iData) {
            auto *data = dynamic_cast<TRawDataObject *>(data_array->At(iData));
            if (data && data->GetDetID() == fCatID[1]) {
                auto *outData = static_cast<art::TSimpleData *>(fOutData->ConstructedAt(counter));
                counter++;
                outData->SetValue(data->GetValue());
            }
        }
    }
}

} // namespace art::crib
