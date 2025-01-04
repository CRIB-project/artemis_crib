/**
 * @file    TMUXPositionConverter.cc
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 11:50:14
 * @note    last modified: 2025-01-04 15:56:46
 * @details
 */

#include "TMUXPositionConverter.h"

#include <TObjArray.h>
#include <TObjString.h>
#include <constant.h>

#include <algorithm>
#include <iterator>
#include <memory>

/// ROOT macro for class implementation
ClassImp(art::crib::TMUXPositionConverter);

namespace art::crib {
TMUXPositionConverter::TMUXPositionConverter() = default;

TMUXPositionConverter::~TMUXPositionConverter() = default;

/**
 * @details
 * - Checks if `fParams` is empty. If so, returns `kInvalidI` and logs a warning.
 * - Uses `std::lower_bound` to find the first element in `fParams` not less than `val`.
 * - If `val` is out of range (less than the first or greater than the last element), returns `kInvalidI`.
 * - Otherwise, returns the index of the boundary just below `val`.
 */
Double_t TMUXPositionConverter::Convert(const Double_t val) const {
    if (fParams.empty()) {
        Warning("Convert", "fParams is empty. Returning invalid value.");
        return kInvalidI;
    }

    auto it = std::lower_bound(fParams.begin(), fParams.end(), val);
    if (it == fParams.begin() || it == fParams.end()) {
        return kInvalidI;
    }

    return std::distance(fParams.begin(), it - 1);
}

/**
 * @details
 * - Strips leading and trailing spaces and removes comments starting with `#`.
 * - Replaces commas and tabs with spaces, and normalizes multiple spaces into a single space.
 * - Splits the string into tokens, attempts to convert each token to a `Double_t`, and stores valid values in `fParams`.
 * - Invalid tokens are skipped, and a warning is logged.
 * - After parsing, `fParams` is sorted to prepare for binary search operations.
 */
Bool_t TMUXPositionConverter::LoadString(const TString &str) {
    TString lineContent = str;
    lineContent = lineContent.Strip(TString::kBoth);
    if (lineContent.BeginsWith("#") || lineContent.IsNull()) {
        return kFALSE;
    }

    Ssiz_t hashIndex = lineContent.Index("#");
    if (hashIndex != kNPOS) {
        lineContent.Remove(hashIndex);
    }
    lineContent = lineContent.Strip(TString::kBoth);

    lineContent.ReplaceAll(",", " ");
    lineContent.ReplaceAll("\t", " ");
    lineContent.ReplaceAll(" +", " ");

    std::unique_ptr<TObjArray> tokens(lineContent.Tokenize(" "));
    if (!tokens) {
        return kFALSE;
    }

    for (int i = 0; i < tokens->GetEntries(); ++i) {
        auto *token = dynamic_cast<TObjString *>(tokens->At(i));
        if (!token) {
            Warning("LoadString", "Invalid token at index %d", i);
            continue;
        }

        TString valueStr = token->GetString();
        if (!valueStr.IsFloat()) {
            Warning("LoadString", "Invalid value: %s", valueStr.Data());
            continue;
        }
        fParams.emplace_back(valueStr.Atof());
    }
    std::sort(fParams.begin(), fParams.end());
    Info("LoadString", "Loaded %zu parameters", fParams.size());

    return !fParams.empty();
}

/**
 * @details
 * Iterates over all elements in `fParams` and logs their values using ROOT's `Info` function.
 */
void TMUXPositionConverter::Print(Option_t *) const {
    for (const auto &e : fParams) {
        Info("Print", "fParams element: %lf", e);
    }
}
} // namespace art::crib
