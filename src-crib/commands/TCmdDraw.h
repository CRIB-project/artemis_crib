/**
 * @file    TCmdDraw.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 17:52:08
 * @note    last modified: 2024-08-21 18:19:22
 * @details
 */

#ifndef _TCMDDRAW_H_
#define _TCMDDRAW_H_

#include "TCatCmd.h"

namespace art::crib {
class TCmdDraw;
}

class TF1;

class art::crib::TCmdDraw : public TCatCmd {
  public:
    TCmdDraw();
    ~TCmdDraw();

    static const char *kFuncNameBase;
    static const char *kLabelNameBase;
    Long_t Cmd(vector<TString>) override;
    void Help() override;

  private:
    TCmdDraw(const TCmdDraw &) = delete;
    TCmdDraw &operator=(const TCmdDraw &) = delete;

    ClassDefOverride(TCmdDraw, 1); // Obtain slope graphically
};

#endif // end of #ifndef _TCMDDRAW_H_
