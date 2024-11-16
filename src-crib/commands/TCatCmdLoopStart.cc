/**
 * @file    TCatCmdLoopStart.cc
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 17:23:44
 * @note    last modified: 2024-08-21 18:20:58
 * @details
 */

#include "TCatCmdLoopStart.h"
#include <TLoopManager.h>

using art::crib::TCatCmdLoopStart;

ClassImp(TCatCmdLoopStart);

TCatCmdLoopStart::TCatCmdLoopStart() {
    SetName("start");
    SetTitle("start loop (same with resume");
}
TCatCmdLoopStart::~TCatCmdLoopStart() {}

TCatCmdLoopStart *TCatCmdLoopStart::Instance() {
    static TCatCmdLoopStart instance;
    return &instance;
}

Long_t TCatCmdLoopStart::Cmd(vector<TString> args) {
    art::TLoopManager *lm = art::TLoopManager::Instance();
    switch (args.size()) {
    case 1:
        lm->Resume(0);
        break;
    default:
        break;
    }
    return 1;
}

void TCatCmdLoopStart::Help() {}
