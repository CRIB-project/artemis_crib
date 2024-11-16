/**
 * @file    TCatCmdLoopStop.cc
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 17:29:21
 * @note    last modified: 2024-08-21 18:16:22
 * @details
 */

#include "TCatCmdLoopStop.h"
#include <TLoopManager.h>

using art::crib::TCatCmdLoopStop;

ClassImp(TCatCmdLoopStop);

TCatCmdLoopStop::TCatCmdLoopStop() {
    SetName("stop");
    SetTitle("stop loop (same with suspend)");
}
TCatCmdLoopStop::~TCatCmdLoopStop() {}

TCatCmdLoopStop *TCatCmdLoopStop::Instance() {
    static TCatCmdLoopStop instance;
    return &instance;
}

Long_t TCatCmdLoopStop::Cmd(vector<TString> args) {
    art::TLoopManager *lm = art::TLoopManager::Instance();
    switch (args.size()) {
    case 1:
        lm->Suspend(0);
        break;
    default:
        break;
    }
    return 1;
}

void TCatCmdLoopStop::Help() {}
