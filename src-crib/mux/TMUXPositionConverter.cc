/**
 * @file    TMUXPositionConverter.cc
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 11:50:14
 * @note    last modified: 2025-01-01 21:34:03
 * @details
 */

#include "TMUXPositionConverter.h"

#include <TObjArray.h>
#include <TObjString.h>
#include <constant.h>

ClassImp(art::crib::TMUXPositionConverter);

namespace art::crib {
TMUXPositionConverter::TMUXPositionConverter() {
    fParams.clear();
    fParams.shrink_to_fit();
}

TMUXPositionConverter::~TMUXPositionConverter() = default;

Double_t TMUXPositionConverter::Convert(const Double_t val) const {
    if (fParams[0] < val && val < fParams[1]) {
        return 1;
    } else {
        return kInvalidI;
    }
}

Bool_t TMUXPositionConverter::LoadString(const TString &str) {
    TString lineContent = str;
    if (lineContent.BeginsWith("#"))
        return kFALSE;

    Ssiz_t hashIndex = str.Index("#");
    if (hashIndex != kNPOS)
        lineContent.Remove(hashIndex);

    lineContent = lineContent.Strip(TString::kBoth);
    if (lineContent.IsNull())
        return kFALSE;

    lineContent.ReplaceAll(",", " ");
    lineContent.ReplaceAll("\t", " ");
    TString singleSpaceLine;
    for (int i = 0; i < lineContent.Length(); ++i) {
        if (lineContent[i] != ' ' || (i > 0 && lineContent[i - 1] != ' ')) {
            singleSpaceLine.Append(lineContent[i]);
        }
    }

    TObjArray *tokens = singleSpaceLine.Tokenize(" ");
    if (!tokens) {
        return kFALSE;
    }

    for (Int_t i = 0; i < tokens->GetEntries(); ++i) {
        TObjString *token = dynamic_cast<TObjString *>(tokens->At(i));
        if (!token) {
            Warning("LoadString", "Invalid token at index %d", i);
            continue;
        }

        TString valueStr = token->GetString();
        if (!valueStr.IsFloat()) {
            Warning("LoadString", "Invalid value: %s", valueStr.Data());
            continue;
        }
        fParams.push_back(valueStr.Atof()); // fValues は std::vector<Double_t>
    }

    delete tokens;
    Info("LoadString", "Loaded %d parameters", static_cast<int>(fParams.size()));

    return !fParams.empty();
}

void TMUXPositionConverter::Print(Option_t *) const {
    // const TString indent(' ', gROOT->GetDirLevel());
    // printf("OBJ: %s\t%s\n", IsA()->GetName(), GetName());
    // printf("%s f(x) = %lf + %lf * x\n", indent.Data(), fParam[0], fParam[1]);
}
} // namespace art::crib
