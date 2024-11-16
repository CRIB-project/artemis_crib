/**
 * @file    TParticleInfo.cc
 * @brief   particle information class
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-05-08 18:07:50
 * @note    last modified: 2024-08-23 21:17:03
 * @details
 */

#include "TParticleInfo.h"

#include <constant.h>

using art::crib::TParticleInfo;

ClassImp(TParticleInfo);

TParticleInfo::TParticleInfo()
    : fMassNumber(kInvalidI), fAtomicNumber(kInvalidI), fCharge(kInvalidI),
      fEnergy(kInvalidD), fCurrentZ(kInvalidD), fTime(kInvalidD),
      fTheta_cm(kInvalidD), fPhi_cm(kInvalidD) {
    TDataObject::SetID(kInvalidI);
    SetTrack(0., 0., 0., 0., 0.);
    fVec.SetXYZT(0., 0., 0., 0.);
}

TParticleInfo::~TParticleInfo() {}

TParticleInfo::TParticleInfo(const TParticleInfo &rhs)
    : TDataObject(rhs),
      fMassNumber(rhs.fMassNumber),
      fAtomicNumber(rhs.fAtomicNumber),
      fCharge(rhs.fCharge),
      fEnergy(rhs.fEnergy),
      fCurrentZ(rhs.fCurrentZ),
      fTime(rhs.fTime),
      fTrack(rhs.fTrack),
      fVec(rhs.fVec),
      fTheta_cm(rhs.fTheta_cm),
      fPhi_cm(rhs.fPhi_cm) {
}

TParticleInfo &TParticleInfo::operator=(const TParticleInfo &rhs) {
    if (this != &rhs) {
        ((TParticleInfo &)rhs).Copy(*this);
    }
    return *this;
}

void TParticleInfo::Copy(TObject &dest) const {
    TDataObject::Copy(dest);
    TParticleInfo &cobj = *(TParticleInfo *)&dest;

    cobj.fMassNumber = this->GetMassNumber();
    cobj.fAtomicNumber = this->GetAtomicNumber();
    cobj.fCharge = this->GetCharge();
    cobj.fEnergy = this->GetEnergy();
    cobj.fCurrentZ = this->GetCurrentZ();
    cobj.fTime = this->GetDurationTime();
    cobj.fTrack = this->GetTrack();
    cobj.fVec = this->GetLorentzVector();
    cobj.fTheta_cm = this->GetThetaCM();
    cobj.fPhi_cm = this->GetPhiCM();
}

void TParticleInfo::Clear(Option_t *opt) {
    TDataObject::Clear(opt);
    TDataObject::SetID(kInvalidI);

    fMassNumber = kInvalidI;
    fAtomicNumber = kInvalidI;
    fCharge = kInvalidI;
    fEnergy = kInvalidD;
    fCurrentZ = kInvalidD;
    fTime = kInvalidD;
    fTheta_cm = kInvalidD;
    fPhi_cm = kInvalidD;

    SetTrack(0., 0., 0., 0., 0.);
    fVec.SetXYZT(0., 0., 0., 0.);
}
