/**
 * @file    TMUXData.h
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-07-30 09:48:52
 * @note    last modified: 2024-08-24 10:32:10
 * @details
 */

#ifndef _CRIB_TMUXDATA_H
#define _CRIB_TMUXDATA_H

#include "TDataObject.h"
#include "constant.h"

namespace art::crib {
class TMUXData;
} // namespace art::crib

class art::crib::TMUXData : public TDataObject {
  public:
    typedef enum { kID,
                   kTiming } ESortType;
    typedef enum { kASC,
                   kDESC } ESortOrder;

    TMUXData();
    ~TMUXData();
    TMUXData(const TMUXData &rhs);
    TMUXData &operator=(const TMUXData &rhs);

    Double_t GetE1() const { return fE1; }
    void SetE1(Double_t arg) { fE1 = arg; }
    Double_t GetE2() const { return fE2; }
    void SetE2(Double_t arg) { fE2 = arg; }
    Double_t GetP1() const { return fP1; }
    void SetP1(Double_t arg) { fP1 = arg; }
    Double_t GetP2() const { return fP2; }
    void SetP2(Double_t arg) { fP2 = arg; }

    Double_t GetTrig() const { return fTiming; }
    void SetTrig(Double_t arg) { fTiming = arg; }

    void Copy(TObject &dest) const override;
    void Clear(Option_t *opt = "") override;

    static const Int_t kNRAW = 5; // number (e1, e2, p1, p2, T)

  protected:
    Double_t fE1;
    Double_t fE2;
    Double_t fP1;
    Double_t fP2;
    Double_t fTiming;

    ClassDefOverride(TMUXData, 1)
};

#endif // _TMUXDATA_H
