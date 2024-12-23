/**
 * @file    TMapSelector.h
 * @brief   extract one catid data
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-12-23 11:56:51
 * @note    last modified: 2024-12-23 11:59:59
 * @details
 */

#ifndef _CRIB_TMAPSELECTOR_H_
#define _CRIB_TMAPSELECTOR_H_

#include "TProcessor.h"

class TClonesArray;

namespace art {
class TSegmentedData;
} // namespace art

namespace art::crib {
class TMapSelector : public TProcessor {
  public:
    TMapSelector();
    ~TMapSelector();

    void Init(TEventCollection *col) override;
    void Process() override;

  private:
    TString fSegmentedDataName;
    TString fOutputColName;

    IntVec_t fSegID; //!

    TSegmentedData *fSegmentedData; //!
    TClonesArray *fOutData;         //!

    TMapSelector(const TMapSelector &) = delete;
    TMapSelector &operator=(const TMapSelector &) = delete;

    ClassDefOverride(TMapSelector, 0);
};
} // namespace art::crib

#endif // end of #ifndef _CRIB_TMAPSELECTOR_H_
