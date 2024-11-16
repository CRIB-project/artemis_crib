/**
 * @file    TBranchCopyProcessor.cc
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 16:06:15
 * @note    last modified: 2024-08-23 20:47:25
 * @details
 */

#include "TBranchCopyProcessor.h"

#include "TDataObject.h"

using art::crib::TBranchCopyProcessor;

ClassImp(TBranchCopyProcessor);

TBranchCopyProcessor::TBranchCopyProcessor() : fInData(nullptr), fOutData(nullptr) {
    RegisterInputCollection("InputCollection", "name of the origin branch", fInputColName, TString("origin"));
    RegisterOutputCollection("OutputCollection", "output collection name", fOutputColName, TString("copy"));
}

TBranchCopyProcessor::~TBranchCopyProcessor() {
    delete fOutData;
    fOutData = nullptr;
}

void TBranchCopyProcessor::Init(TEventCollection *col) {
    Info("Init", "%s => %s copy", fInputColName.Data(), fOutputColName.Data());

    fInData = reinterpret_cast<TClonesArray **>(col->GetObjectRef(fInputColName.Data()));
    if (!fInData) {
        SetStateError(TString::Format("input not found: %s", fInputColName.Data()));
        return;
    }

    const TClass *const cl = (*fInData)->GetClass();
    fOutData = new TClonesArray(cl);
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);
}

void TBranchCopyProcessor::Process() {
    fOutData->Clear("C");

    for (Int_t iData = 0; iData < (*fInData)->GetEntriesFast(); iData++) {
        const TDataObject *const inData = static_cast<TDataObject *>((*fInData)->At(iData));
        TDataObject *outData = static_cast<TDataObject *>(fOutData->ConstructedAt(iData));

        inData->Copy(*outData);
    }
}
