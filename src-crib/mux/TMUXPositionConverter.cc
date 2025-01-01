/**
 * @file    TMUXPositionConverter.cc
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 11:50:14
 * @note    last modified: 2025-01-01 19:17:29
 * @details
 */

#include "TMUXPositionConverter.h"

#include <constant.h>

ClassImp(art::crib::TMUXPositionConverter);

namespace art::crib {
TMUXPositionConverter::TMUXPositionConverter() = default;
TMUXPositionConverter::~TMUXPositionConverter() = default;

Double_t TMUXPositionConverter::Convert(const Double_t val) const {
    if (fParams[0] < val && val < fParams[1]) {
        return 1;
    } else {
        return kInvalidI;
    }
}

Bool_t TMUXPositionConverter::LoadString(const TString &s) {
    // const Int_t commentStartIndex = s.First('#');
    // const TString &strWithoutComment =
    //     commentStartIndex == -1 ? TString(s) : TString(s).Replace(commentStartIndex, s.Length(), "");
    // TObjArray *a = strWithoutComment.Tokenize(" ,\t");
    // const UInt_t nTokens = a->GetEntriesFast();
    // if (!nTokens) {
    //     return kFALSE;
    // } else if (nTokens != fParams.size()) {
    //     Error("LoadString", "Number of parameter is %d (expected: %d)", nTokens, (UInt_t)fParam.size());
    //     return kFALSE;
    // }

    // for (Int_t i = 0, n = fParams.size(); i != n; ++i) {
    //     const TString &subs = static_cast<TObjString *>(a->UncheckedAt(i))->GetString();
    //     if (!subs.IsFloat())
    //         return kFALSE;
    //     fParams[i] = subs.Atof();
    // }

    // fLoaded = kTRUE;
    return kTRUE;
}

void TMUXPositionConverter::Print(Option_t *) const {
    // const TString indent(' ', gROOT->GetDirLevel());
    // printf("OBJ: %s\t%s\n", IsA()->GetName(), GetName());
    // printf("%s f(x) = %lf + %lf * x\n", indent.Data(), fParam[0], fParam[1]);
}
} // namespace art::crib
