/**
 * @file    TBranchCopyProcessor.h
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 16:06:58
 * @note    last modified: 2024-08-23 20:47:46
 * @details
 */

#ifndef _CRIB_TBRANCHCOPYPROCESSOR_H_
#define _CRIB_TBRANCHCOPYPROCESSOR_H_

#include <TProcessor.h>

namespace art::crib {
class TBranchCopyProcessor;
}

class TClonesArray;

class art::crib::TBranchCopyProcessor : public TProcessor {
  public:
    TBranchCopyProcessor();
    ~TBranchCopyProcessor();

    void Init(TEventCollection *col) override;
    void Process() override;

  protected:
    TString fInputColName;
    TString fOutputColName;
    TClonesArray **fInData; //!
    TClonesArray *fOutData; //!

  private:
    TBranchCopyProcessor(const TBranchCopyProcessor &) = delete;
    TBranchCopyProcessor &operator=(const TBranchCopyProcessor &) = delete;

    ClassDefOverride(TBranchCopyProcessor, 0); // simple data mapper
};

#endif // end of #ifndef _TBRANCHCOPYPROCESSOR_H_
