/**
 * @file    TCmdXfitg.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 17:52:08
 * @note    last modified: 2024-08-21 18:21:59
 * @details
 */

#ifndef _TCMDXFITG_H_
#define _TCMDXFITG_H_

#include "TCatCmd.h"

namespace art::crib {
class TCmdXfitg;
}

class TF1;

class art::crib::TCmdXfitg : public TCatCmd {
  public:
    TCmdXfitg();
    ~TCmdXfitg();

    static const char *kFuncNameBase;
    static const char *kLabelNameBase;
    Long_t Cmd(vector<TString>) override;
    void Help() override;

  private:
    TCmdXfitg(const TCmdXfitg &) = delete;            // undefined
    TCmdXfitg &operator=(const TCmdXfitg &) = delete; // undefined

    ClassDefOverride(TCmdXfitg, 1); // Obtain slope graphically
};

#endif // end of #ifndef _TCMDXFITG_H_
