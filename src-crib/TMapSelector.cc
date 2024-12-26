/**
 * @file    TMapSelector.cc
 * @brief   extract one catid data
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-12-23 11:56:21
 * @note    last modified: 2024-12-23 16:53:00
 * @details
 */

#include "TMapSelector.h"

#include <TCategorizedData.h>
#include <vector>

ClassImp(art::crib::TMapSelector);

namespace art::crib {
TMapSelector::TMapSelector() : fCategorizedData(nullptr), fOutData(nullptr) {
    RegisterInputCollection("CategorizedDataName", "name of the segmented data",
                            fCategorizedDataName, TString("catdata"));
    RegisterOutputCollection("OutputCollection", "name of the output branch",
                             fOutputColName, TString("channel"));

    RegisterProcessorParameter("CatID", "categorized ID, (catid)",
                               fCatID, -1);
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
    if (fCatID < 0) {
        SetStateError(Form("CatID is invalid: %d", fCatID));
        return;
    }

    fOutData = new TClonesArray("std::vector<int>");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);
    Info("Init", "%s -> %s, CatID = %d",
         fCategorizedDataName.Data(), fOutputColName.Data(), fCatID);
}

void TMapSelector::Process() {
    fOutData->Clear("C");
    if (!fCategorizedData) {
        Warning("Process", "No CategorizedData object");
        return;
    }

    auto *cat_array = fCategorizedData->FindCategory(fCatID);
    if (!cat_array) {
        Warning("Process", "No data having catid = %d", fCatID);
        return;
    }

    const int nCat = cat_array->GetEntriesFast();
    int counter = 0;
    for (int iCat = 0; iCat < nCat; ++iCat) {
        std::cout << iCat << std::endl;
    }
    std::cout << std::endl;
}

} // namespace art::crib
