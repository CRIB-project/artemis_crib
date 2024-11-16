/**
 * @file    TParticleInfo.h
 * @brief   particle information class
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-05-08 18:07:32
 * @note    last modified: 2024-08-24 10:31:15
 * @details
 */

#ifndef _TPARTICLEINFO_H_
#define _TPARTICLEINFO_H_

#include <TLorentzVector.h>
#include <TTrack.h>

namespace art::crib {
class TParticleInfo;
}

class art::crib::TParticleInfo : public TDataObject {
  public:
    typedef enum { kID,
                   kTiming } ESortType;
    typedef enum { kASC,
                   kDESC } ESortOrder;

    TParticleInfo();
    ~TParticleInfo();

    TParticleInfo(const TParticleInfo &rhs);
    TParticleInfo &operator=(const TParticleInfo &rhs);

    void SetMassNumber(Int_t val) { fMassNumber = val; }
    Int_t GetMassNumber() const { return fMassNumber; }
    void SetAtomicNumber(Int_t val) { fAtomicNumber = val; }
    Int_t GetAtomicNumber() const { return fAtomicNumber; }
    void SetCharge(Int_t val) { fCharge = val; }
    Int_t GetCharge() const { return fCharge; }

    void SetCurrentZ(Double_t val) { fCurrentZ = val; }
    Double_t GetCurrentZ() const { return fCurrentZ; }
    void SetEnergy(Double_t val) { fEnergy = val; }
    Double_t GetEnergy() const { return fEnergy; }
    void SetZeroTime() { fTime = 0; }
    void AddTime(Double_t val) { fTime += val; }
    Double_t GetDurationTime() const { return fTime; }

    void SetLorentzVector(Double_t x, Double_t y, Double_t z, Double_t t) { fVec.SetXYZT(x, y, z, t); }
    void SetLorentzVector(TLorentzVector val) { fVec = val; }
    TLorentzVector GetLorentzVector() const { return fVec; }

    void SetTrack(Double_t x, Double_t y, Double_t z, Double_t a, Double_t b) {
        fTrack.SetPos(x, y, z);
        fTrack.SetAngle(a, b);
    }
    TTrack GetTrack() const { return fTrack; }

    void SetThetaCM(Double_t val) { fTheta_cm = val; }
    Double_t GetThetaCM() const { return fTheta_cm; }
    void SetPhiCM(Double_t val) { fPhi_cm = val; }
    Double_t GetPhiCM() const { return fPhi_cm; }

    void Copy(TObject &dest) const override;
    void Clear(Option_t *opt = "") override;

  protected:
    Int_t fMassNumber;
    Int_t fAtomicNumber;
    Int_t fCharge;

    Double_t fEnergy;   // kinetic energy in LAB system
    Double_t fCurrentZ; // current Z position
    Double_t fTime;     // Duration time (ns)

    TTrack fTrack;       // tracking information in LAB system
    TLorentzVector fVec; // lorentz vector (px, py, pz, E) of this particle

    Double_t fTheta_cm; // theta angle (deg) in CM system
    Double_t fPhi_cm;   // phi angle (deg) in CM system

    ClassDefOverride(TParticleInfo, 1);
};

#endif // end of #ifndef _TPARTICLEINFO_H_
