/**
 * @file    TMUXData.cc
 * @brief   Source file for the TMUXData class.
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-07-30 09:49:05
 * @note    last modified: 2025-01-02 13:48:35
 * @details
 */

#include "TMUXData.h"

#include <constant.h>

/// ROOT macro for class implementation
ClassImp(art::crib::TMUXData);

namespace art::crib {
TMUXData::TMUXData()
    : fE1(kInvalidD), fE2(kInvalidD),
      fP1(kInvalidD), fP2(kInvalidD),
      fTiming(kInvalidD), fTVec() {
    TDataObject::SetID(kInvalidI);
}

TMUXData::~TMUXData() = default;

TMUXData::TMUXData(const TMUXData &rhs)
    : TDataObject(rhs),
      fE1(rhs.fE1),
      fE2(rhs.fE2),
      fP1(rhs.fP1),
      fP2(rhs.fP2),
      fTiming(rhs.fTiming),
      fTVec(rhs.fTVec) {
}

TMUXData &TMUXData::operator=(const TMUXData &rhs) {
    if (this != &rhs) {
        TDataObject::operator=(rhs);
        fE1 = rhs.fE1;
        fE2 = rhs.fE2;
        fP1 = rhs.fP1;
        fP2 = rhs.fP2;
        fTiming = rhs.fTiming;
        fTVec = rhs.fTVec;
    }
    return *this;
}

void TMUXData::Copy(TObject &dest) const {
    TDataObject::Copy(dest);
    auto *cobj = dynamic_cast<TMUXData *>(&dest);
    if (cobj) {
        cobj->fE1 = this->GetE1();
        cobj->fE2 = this->GetE2();
        cobj->fP1 = this->GetP1();
        cobj->fP2 = this->GetP2();
        cobj->fTiming = this->GetTrig();
        cobj->fTVec = this->fTVec;
    } else {
        Error("Copy", "Cannot cast TObject to TMUXData.");
    }
}

void TMUXData::Clear(Option_t *opt) {
    TDataObject::Clear(opt);
    TDataObject::SetID(kInvalidI);
    fE1 = kInvalidD;
    fE2 = kInvalidD;
    fP1 = kInvalidD;
    fP2 = kInvalidD;
    fTiming = kInvalidD;
    fTVec.clear();
    fTVec.shrink_to_fit();
}

void TMUXData::PushTiming(Double_t value) {
    fTVec.emplace_back(value);
}

Double_t TMUXData::GetT(Int_t index) const {
    if (index >= 0 && index < static_cast<Int_t>(fTVec.size())) {
        return fTVec[index];
    }
    return kInvalidD;
}

} // namespace art::crib
