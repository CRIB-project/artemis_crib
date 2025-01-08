/**
 * @file    TEvtNumProcessor.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022?
 * @note    last modified: 2025-01-08 10:27:37
 * @details originally made by J. W. Hwang
 */

#ifndef _CRIB_TEVTNUMPROCESSOR_H_
#define _CRIB_TEVTNUMPROCESSOR_H_

#include "TProcessor.h"

namespace art::crib {
class TEvtNumProcessor;
}

class art::crib::TEvtNumProcessor : public TProcessor {

  public:
    TEvtNumProcessor();
    ~TEvtNumProcessor() override;

    void Init(TEventCollection *col) override;
    void EndOfRun() override;
    void Process() override;

  private:
    Int_t fCurNum;
    Int_t fLimEvNum;
    Bool_t fPriEv;
    Int_t fPriEvNum;

  protected:
    ClassDefOverride(TEvtNumProcessor, 1);
};
#endif // end of #ifdef _TEVTNUMPROCESSOR_H_
