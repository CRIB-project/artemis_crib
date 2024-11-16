/**
 * @file    TTelescopeData.cc
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-17 17:14:39
 * @note    last modified: 2024-08-23 21:02:18
 * @details
 */

#include "TTelescopeData.h"

using art::crib::TTelescopeData;

ClassImp(TTelescopeData);

TTelescopeData::TTelescopeData()
    : fTelID(kInvalidI), fXID(kInvalidI), fYID(kInvalidI), fNE(0),
      fdE(0.0), fdEX(0.0), fdEY(0.0), fE(0.0), fEtotal(0.0),
      fXTiming(kInvalidD), fYTiming(kInvalidD), fTheta_L(kInvalidD) {
    TDataObject::SetID(kInvalidI);
    fPos.SetXYZ(kInvalidD, kInvalidD, kInvalidD);
    fEnergyArray.clear();
    fEnergyArray.shrink_to_fit();
    fTimingArray.clear();
    fTimingArray.shrink_to_fit();
}

TTelescopeData::~TTelescopeData() {
}

TTelescopeData::TTelescopeData(const TTelescopeData &rhs)
    : TDataObject(rhs),
      fPos(rhs.fPos),
      fTelID(rhs.fTelID),
      fXID(rhs.fXID),
      fYID(rhs.fYID),
      fNE(rhs.fNE),
      fdE(rhs.fdE),
      fdEX(rhs.fdEX),
      fdEY(rhs.fdEY),
      fE(rhs.fE),
      fEtotal(rhs.fEtotal),
      fXTiming(rhs.fXTiming),
      fYTiming(rhs.fYTiming),
      fTheta_L(rhs.fTheta_L),
      fEnergyArray(rhs.fEnergyArray),
      fTimingArray(rhs.fTimingArray) {
}

TTelescopeData &TTelescopeData::operator=(const TTelescopeData &rhs) {
    if (this != &rhs) {
        ((TTelescopeData &)rhs).Copy(*this);
    }
    return *this;
}

void TTelescopeData::Copy(TObject &dest) const {
    TDataObject::Copy(dest);
    TTelescopeData &cobj = *(TTelescopeData *)&dest;

    cobj.fPos = this->GetPosition();

    cobj.fTelID = this->GetTelID();
    cobj.fXID = this->GetXID();
    cobj.fYID = this->GetYID();
    cobj.fNE = this->GetN();

    cobj.fdE = this->GetdE();
    cobj.fdEX = this->GetdEX();
    cobj.fdEY = this->GetdEY();

    cobj.fE = this->GetE();
    cobj.fEtotal = this->GetEtotal();
    cobj.fXTiming = this->GetTelXTiming();
    cobj.fYTiming = this->GetTelYTiming();

    cobj.fTheta_L = this->GetTheta_L();

    cobj.fEnergyArray = this->GetEnergyArray();
    cobj.fTimingArray = this->GetTimingArray();
}

void TTelescopeData::Clear(Option_t *opt) {
    TDataObject::Clear(opt);
    TDataObject::SetID(kInvalidI);

    fPos.SetXYZ(kInvalidD, kInvalidD, kInvalidD);

    fTelID = kInvalidI;
    fXID = kInvalidI;
    fYID = kInvalidI;
    fNE = 0;

    fdE = 0.0;
    fdEX = 0.0;
    fdEY = 0.0;

    fE = 0.0;
    fEtotal = 0.0;
    fXTiming = kInvalidD;
    fYTiming = kInvalidD;

    fTheta_L = kInvalidD;

    fEnergyArray.clear();
    fEnergyArray.shrink_to_fit();
    fTimingArray.clear();
    fTimingArray.shrink_to_fit();
}
