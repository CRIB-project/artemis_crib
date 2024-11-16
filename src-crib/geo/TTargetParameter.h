/**
 * @file    TTargetParameter.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-17 22:14:55
 * @note    last modified: 2024-08-21 17:53:40
 * @details
 */

#ifndef _CRIB_TTARGETPARAMETER_H_
#define _CRIB_TTARGETPARAMETER_H_

#include <TParameterObject.h>

namespace art::crib {
class TTargetParameter;
}

class art::crib::TTargetParameter : public TParameterObject {
  public:
    TTargetParameter();
    ~TTargetParameter();

    TTargetParameter(const TTargetParameter &);
    TTargetParameter &operator=(const TTargetParameter &rhs);

    TString GetTargetName() const { return fName; }
    void SetTargetName(TString val) { fName = val; }
    Bool_t IsGas() const { return fIsGas; }
    void SetIsGasState(Bool_t val) { fIsGas = val; }
    Double_t GetZ() const { return fZ; }
    void SetZ(Double_t val) { fZ = val; }
    Double_t GetThickness() const { return fThickness; }
    void SetThickness(Double_t val) { fThickness = val; }

  protected:
    TString fName;       //! target name
    Bool_t fIsGas;       //! gas target or not
    Double_t fZ;         //! z position (not use in gas target, mm)
    Double_t fThickness; //! thickness of the target (mm)

  private:
    ClassDef(TTargetParameter, 0)
};

#endif // end of #ifndef _TTARGETPARAMETER_H_
