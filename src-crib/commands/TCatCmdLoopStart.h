/**
 * @file    TCatCmdLoopStart.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 17:24:46
 * @note    last modified: 2024-08-21 18:14:23
 * @details
 */

#ifndef _CRIB_TCATCMDLOOPSTART_H_
#define _CRIB_TCATCMDLOOPSTART_H_

#include <TCatCmd.h>

namespace art::crib {
class TCatCmdLoopStart;
} // namespace art::crib

class art::crib::TCatCmdLoopStart : public TCatCmd {
  protected:
    TCatCmdLoopStart();

  public:
    ~TCatCmdLoopStart();

    static TCatCmdLoopStart *Instance();

    Long_t Cmd(vector<TString> args) override;

    void Help() override;

    ClassDefOverride(TCatCmdLoopStart, 1);
};
#endif // end of #ifdef _TCATCMDLOOPSTART_H_
