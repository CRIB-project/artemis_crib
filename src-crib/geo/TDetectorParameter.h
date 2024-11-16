/**
 * @file    TDetectorParameter.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-18 11:32:42
 * @note    last modified: 2024-08-21 17:54:21
 * @details
 */

#ifndef _CRIB_TDETECTORPARAMETER_H_
#define _CRIB_TDETECTORPARAMETER_H_

#include <TParameterObject.h>

namespace art::crib {
class TDetectorParameter;
}

class art::crib::TDetectorParameter : public TParameterObject {
  public:
    TDetectorParameter();
    ~TDetectorParameter();

    TDetectorParameter(const TDetectorParameter &);
    TDetectorParameter &operator=(const TDetectorParameter &rhs);

    TString GetDetName() const { return fDetName; }
    void SetDetName(TString str) { fDetName = str; }

    Int_t GetN() const { return fN; }
    void SetN(Int_t arg) { fN = arg; }

    TString GetMaterial(Int_t id) const { return fMaterial[id]; }
    StringVec_t GetMaterial() const { return fMaterial; }
    void SetMaterial(StringVec_t vec) { fMaterial = vec; }
    Double_t GetThickness(Int_t id) const { return fThickness[id]; }
    void SetThickness(DoubleVec_t vec) { fThickness = vec; }
    Double_t GetPedestal(Int_t id) const { return fPedestal[id]; }
    void SetPedestal(DoubleVec_t vec) { fPedestal = vec; }

    Double_t GetCenterRotPos(Int_t id) const { return fCenterRot[id]; }
    DoubleVec_t GetCenterRotPos() const { return fCenterRot; }
    void SetCenterRotPos(DoubleVec_t vec) { fCenterRot = vec; }
    Double_t GetOffset(Int_t id) const { return fOffset[id]; }
    DoubleVec_t GetOffset() const { return fOffset; }
    void SetOffset(DoubleVec_t vec) { fOffset = vec; }

    Double_t GetSize(Int_t id) const { return fSize[id]; }
    DoubleVec_t GetSize() const { return fSize; }
    void SetSize(DoubleVec_t vec) { fSize = vec; }
    Int_t GetStripNum(Int_t id) const { return fStrip[id]; }
    IntVec_t GetStripNum() const { return fStrip; }
    void SetStripNum(IntVec_t vec) { fStrip = vec; }

    Double_t GetDistance() const { return fDistance; }
    void SetDistance(Double_t val) { fDistance = val; }
    Double_t GetAngle() const { return fAngle; }
    void SetAngle(Double_t val) { fAngle = val; }
    Double_t GetMaxRadius() const { return fMaxRadius; }
    void SetMaxRadius(Double_t val) { fMaxRadius = val; }

  protected:
    TString fDetName;
    Int_t fN; //! number of SSDs

    StringVec_t fMaterial;  //!
    DoubleVec_t fThickness; //!
    DoubleVec_t fPedestal;  //! if fCharge is below this, treat as 0 MeV

    DoubleVec_t fCenterRot; //!
    DoubleVec_t fOffset;    //!

    DoubleVec_t fSize; //!
    IntVec_t fStrip;   //!

    Double_t fDistance;
    Double_t fAngle; // radian
    Double_t fMaxRadius;

  private:
    ClassDef(TDetectorParameter, 1) // ppac parameter holder
};

#endif // end of #ifndef _TDETECTORPARAMETER_H_
