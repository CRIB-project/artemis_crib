/**
 * @file    TCatCmdLoopStop.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 17:28:09
 * @note    last modified: 2025-01-08 10:29:24
 * @details
 */

#ifndef _CRIB_TCATCMDLOOPSTOP_H_
#define _CRIB_TCATCMDLOOPSTOP_H_

#include <TCatCmd.h>

namespace art::crib {
class TCatCmdLoopStop;
} // namespace art::crib

class art::crib::TCatCmdLoopStop : public TCatCmd {
  protected:
    TCatCmdLoopStop();

  public:
    ~TCatCmdLoopStop() override;

    static TCatCmdLoopStop *Instance();

    Long_t Cmd(vector<TString> args) override;

    void Help() override;

    ClassDefOverride(TCatCmdLoopStop, 1);
};

#endif // end of #ifndef _TCATCMDLOOPSTOP_H_
