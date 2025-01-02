/**
 * @file    TMUXPositionValidator.cc
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-30 10:02:46
 * @note    last modified: 2025-01-02 22:53:03
 * @details
 */

#include "TMUXPositionValidator.h"
#include "../TProcessorUtil.h"

#include "TMUXData.h"

ClassImp(art::crib::TMUXPositionValidator);

namespace art::crib {
TMUXPositionValidator::TMUXPositionValidator()
    : fInput(nullptr), fOutput(nullptr) {
    RegisterInputCollection("InputCollection", "Input collection",
                            fInputColName, TString("input"));
    RegisterOutputCollection("OutputCollection", "Output collection",
                             fOutputColName, TString("validated"));

    const DoubleVec_t range(2, 0.);
    RegisterProcessorParameter("ValidPositionRange", "[min, max] ignored if min == max",
                               fValidPositionRange, range);
}

TMUXPositionValidator::~TMUXPositionValidator() {
    delete fOutput;
    fOutput = nullptr;
}

void TMUXPositionValidator::Init(TEventCollection *col) {
    auto result = util::GetInputObject<TClonesArray>(
        col, fInputColName, "TClonesArray", "art::crib::TMUXData");

    if (std::holds_alternative<TString>(result)) {
        SetStateError(std::get<TString>(result));
        return;
    }
    fInData = std::get<TClonesArray *>(result);
    Info("Init", "%s => %s", fInputColName.Data(), fOutputColName.Data());

    if (fValidPositionRange.size() != 0) {
        SetStateError(Form("Position range size should be 0, but %zu",
                           fValidPositionRange.size()));
        return;
    }
    if (fValidPositionRange[0] > fValidPositionRange[1]) {
        SetStateError("Position range : min > max");
        return;
    }

    fOutData = new TClonesArray("art::crib::TMUXData");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);
}

void TMUXPositionValidator::Process() {
    fOutData->Clear("C");

    const int nData = fInData->GetEntriesFast();
    for (int iData = 0; iData < nData; ++iData) {
        const auto *data = dynamic_cast<const TMUXData *>(fInData->At(iData));
        if (!data)
            continue;
        Double_t pos1_raw = data->GetP1();
        if (pos1_raw < fValidPositionRange[0] || fValidPositionRange[1] < pos1_raw)
            continue;
        auto *outData = fOutData->ConstructedAt(fOutData->GetEntriesFast());
        data->Copy(*outData);
    }
}
} // namespace art::crib
