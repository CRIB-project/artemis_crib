/**
 * @file    TCatCmdTCutG.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 18:33:54
 * @note    last modified: 2024-08-21 18:18:11
 * @details
 */

#ifndef _CRIB_TCATCMDTCUTG_H_
#define _CRIB_TCATCMDTCUTG_H_

#include <TCatCmd.h>

namespace art::crib {
class TCatCmdTCutG;
} // namespace art::crib

class TPad;

class art::crib::TCatCmdTCutG : public TCatCmd {
  protected:
    TCatCmdTCutG();
    Double_t fX;
    Double_t fY;
    Int_t fEventtype;
    Bool_t fisFirst;

  public:
    ~TCatCmdTCutG();

    static const char *kFuncNameBase;
    static const char *kLabelNameBase;
    static TCatCmdTCutG *Instance();
    Long_t Cmd(vector<TString>) override;
    Long_t Run(TPad *pad, Double_t *x = nullptr, Double_t *y = nullptr);
    Long_t Run(Double_t *x = nullptr, Double_t *y = nullptr);
    void GetEvent();
    void Help() override;

    ClassDefOverride(TCatCmdTCutG, 1);
};

#endif // end of #ifndef _TCATCMDTCUTG_H_
