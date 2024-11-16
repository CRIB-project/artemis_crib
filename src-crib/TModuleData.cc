/**
 * @file    TModuleData.cc
 * @brief   inherit from TModuleInfo
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-08 17:49:53
 * @note    last modified: 2024-08-23 20:52:27
 * @details
 */

#include "TModuleData.h"

using art::crib::TModuleData;

ClassImp(TModuleData);

TModuleData::TModuleData(const TModuleInfo &info)
    : art::TModuleInfo(info), fNCh(0), fMod(-1) {
}

TModuleData::~TModuleData() {
}

TModuleData::TModuleData(const TModuleData &rhs)
    : art::TModuleInfo(rhs) {
}

TModuleData &TModuleData::operator=(const TModuleData &rhs) {
    if (this != &rhs) {
    }
    return *this;
}
