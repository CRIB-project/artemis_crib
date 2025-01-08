/**
 * @file    TMapSelector.h
 * @brief   extract one catid data
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-12-23 11:56:51
 * @note    last modified: 2025-01-08 10:27:57
 * @details
 */

#ifndef _CRIB_TMAPSELECTOR_H_
#define _CRIB_TMAPSELECTOR_H_

#include "TProcessor.h"

class TClonesArray;

namespace art {
class TCategorizedData;
} // namespace art

namespace art::crib {
class TMapSelector : public TProcessor {
  public:
    TMapSelector();
    ~TMapSelector() override;

    void Init(TEventCollection *col) override;
    void Process() override;

  private:
    TString fCategorizedDataName;
    TString fOutputColName;

    IntVec_t fCatID; //!

    TCategorizedData *fCategorizedData; //!
    TClonesArray *fOutData;             //!

    TMapSelector(const TMapSelector &) = delete;
    TMapSelector &operator=(const TMapSelector &) = delete;

    ClassDefOverride(TMapSelector, 1);
};
} // namespace art::crib

#endif // end of #ifndef _CRIB_TMAPSELECTOR_H_
