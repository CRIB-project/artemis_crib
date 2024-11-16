/**
 * @file    TMUXPositionConverter.cc
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 11:50:14
 * @note    last modified: 2024-08-23 21:30:56
 * @details
 */

#include "TMUXPositionConverter.h"

#include "constant.h"
#include <TROOT.h>

using art::crib::TMUXPositionConverter;

ClassImp(TMUXPositionConverter);

TMUXPositionConverter::~TMUXPositionConverter() {
}

Double_t TMUXPositionConverter::Convert(const Double_t val) const {
    if (fParam[0] < val && val < fParam[1]) {
        return 1;
    } else {
        return kInvalidI;
    }
}

void TMUXPositionConverter::Print(Option_t *) const {
    const TString indent(' ', gROOT->GetDirLevel());
    printf("OBJ: %s\t%s\n", IsA()->GetName(), GetName());
    printf("%s f(x) = %lf + %lf * x\n", indent.Data(), fParam[0], fParam[1]);
}
