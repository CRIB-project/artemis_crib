/**
 * @file    TCatCmdLoopStop.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 17:28:09
 * @note    last modified: 2024-08-21 18:18:40
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
    ~TCatCmdLoopStop();

    static TCatCmdLoopStop *Instance();

    Long_t Cmd(vector<TString> args) override;

    void Help() override;

    ClassDefOverride(TCatCmdLoopStop, 1);
};

#endif // end of #ifndef _TCATCMDLOOPSTOP_H_
