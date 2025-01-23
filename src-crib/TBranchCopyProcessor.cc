/**
 * @file    TBranchCopyProcessor.cc
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 16:06:15
 * @note    last modified: 2025-01-10 11:02:19
 * @details
 */

#include "TBranchCopyProcessor.h"
#include "TProcessorUtil.h"

#include <TDataObject.h>

/// ROOT macro for class implementation
ClassImp(art::crib::TBranchCopyProcessor);

namespace art::crib {
TBranchCopyProcessor::TBranchCopyProcessor() {
    RegisterInputCollection("InputCollection", "name of the origin branch",
                            fInputColName, fInputColName);
    RegisterOutputCollection("OutputCollection", "output collection name",
                             fOutputColName, fOutputColName);
}

TBranchCopyProcessor::~TBranchCopyProcessor() {
    delete fOutData;
    fOutData = nullptr;
}

/**
 * @details
 * Sets up the input and output collections by retrieving the input collection
 * and preparing a corresponding output collection of the same class type.
 */
void TBranchCopyProcessor::Init(TEventCollection *col) {
    auto result = util::GetInputObject<TClonesArray>(
        col, fInputColName, "TClonesArray", "art::TDataObject");

    if (std::holds_alternative<TString>(result)) {
        SetStateError(std::get<TString>(result));
        return;
    }

    fInData = std::get<TClonesArray *>(result);
    Info("Init", "%s => %s copy", fInputColName.Data(), fOutputColName.Data());

    const auto *cl = fInData->GetClass();
    if (!cl) {
        SetStateError(Form("Failed to get TClass at branch: %s",
                           fInputColName.Data()));
        return;
    }
    fOutData = new TClonesArray(cl);
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);
}

/**
 * @details
 * Iterates over the entries in the input collection and creates a copy
 * of each entry in the output collection using the `Copy` method of `TDataObject`.
 */
void TBranchCopyProcessor::Process() {
    fOutData->Clear("C");

    for (int iData = 0; iData < fInData->GetEntriesFast(); iData++) {
        const auto *inData = static_cast<TDataObject *>(fInData->At(iData));
        auto *outData = static_cast<TDataObject *>(fOutData->ConstructedAt(iData));

        inData->Copy(*outData);
    }
}
} // namespace art::crib
