/**
 * @file    TModuleData.h
 * @brief   inherit from TModuleInfo
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-08 17:51:01
 * @note    last modified: 2024-08-21 17:32:39
 * @details
 */

#ifndef _CRIB_TMODULEDATA_H_
#define _CRIB_TMODULEDATA_H_

#include <TModuleInfo.h>

namespace art::crib {
class TModuleData;
} // namespace art::crib

class art::crib::TModuleData : public art::TModuleInfo {
  public:
    TModuleData(const TModuleInfo &info);
    ~TModuleData();

    TModuleData(const TModuleData &rhs);
    TModuleData &operator=(const TModuleData &rhs);

    Int_t GetNCh() const { return fNCh; }
    void SetCh(Int_t Nch) {
        fNCh = Nch;
    };

    Int_t GetMod() const { return fMod; }
    void SetMod(Int_t mod) {
        fMod = mod;
    };

    std::vector<Int_t> fData1D;
    std::vector<std::vector<Int_t>> fData2D;

  protected:
    Int_t fNCh;
    Int_t fMod;

  private:
    ClassDef(TModuleData, 2) // module information
};

#endif // _TMODULEDATA_H_
