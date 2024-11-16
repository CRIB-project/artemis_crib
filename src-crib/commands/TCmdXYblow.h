/**
 * @file    TCmdXYblow.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 17:37:11
 * @note    last modified: 2024-08-21 18:23:04
 * @details
 */

#ifndef _TCMDXYBLOW_H_
#define _TCMDXYBLOW_H_

#include <TCatCmd.h>

namespace art::crib {
class TCmdXYblow;
}

class TH2;

class art::crib::TCmdXYblow : public TCatCmd {
  public:
    TCmdXYblow();
    ~TCmdXYblow();

    static const char *kFuncNameBase;
    static const char *kLabelNameBase;
    Long_t Cmd(vector<TString>) override;
    TH2 *Run(TH2 *h2, Option_t *);
    void Help() override;

  private:
    TCmdXYblow(const TCmdXYblow &) = delete;            // undefined
    TCmdXYblow &operator=(const TCmdXYblow &) = delete; // undefined

    ClassDefOverride(TCmdXYblow, 1)
};

#endif // end of #ifndef _TCMDXYBLOW_H_
