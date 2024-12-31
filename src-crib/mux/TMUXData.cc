/**
 * @file    TMUXData.cc
 * @brief   Source file for the TMUXData class.
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-07-30 09:49:05
 * @note    last modified: 2024-12-31 11:40:41
 * @details
 */

#include "TMUXData.h"

#include <constant.h>

ClassImp(art::crib::TMUXData);

namespace art::crib {
TMUXData::TMUXData()
    : fE1(kInvalidD), fE2(kInvalidD),
      fP1(kInvalidD), fP2(kInvalidD),
      fTiming(kInvalidD) {
    TDataObject::SetID(kInvalidI);
}

TMUXData::~TMUXData() = default;

TMUXData::TMUXData(const TMUXData &rhs)
    : TDataObject(rhs),
      fE1(rhs.fE1),
      fE2(rhs.fE2),
      fP1(rhs.fP1),
      fP2(rhs.fP2),
      fTiming(rhs.fTiming) {
}

TMUXData &TMUXData::operator=(const TMUXData &rhs) {
    if (this != &rhs) {
        TDataObject::operator=(rhs);
        fE1 = rhs.fE1;
        fE2 = rhs.fE2;
        fP1 = rhs.fP1;
        fP2 = rhs.fP2;
        fTiming = rhs.fTiming;
    }
    return *this;
}

void TMUXData::Copy(TObject &dest) const {
    TDataObject::Copy(dest);
    auto &cobj = static_cast<TMUXData &>(dest);
    cobj.fE1 = this->GetE1();
    cobj.fE2 = this->GetE2();
    cobj.fP1 = this->GetP1();
    cobj.fP2 = this->GetP2();
    cobj.fTiming = this->GetTrig();
}

void TMUXData::Clear(Option_t *opt) {
    TDataObject::Clear(opt);
    TDataObject::SetID(kInvalidI);
    fE1 = kInvalidD;
    fE2 = kInvalidD;
    fP1 = kInvalidD;
    fP2 = kInvalidD;
    fTiming = kInvalidD;
}

} // namespace art::crib
