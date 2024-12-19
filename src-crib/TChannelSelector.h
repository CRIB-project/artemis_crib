/**
 * @file    TChannelSelector.h
 * @brief   extract one channel data
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-12-18 15:39:13
 * @note    last modified: 2024-12-19 23:16:12
 * @details
 */

#ifndef _CRIB_TCHANNELSELECTOR_H_
#define _CRIB_TCHANNELSELECTOR_H_

#include "TProcessor.h"

class TClonesArray;

namespace art {
class TSegmentedData;
class TCategorizedData;
} // namespace art

namespace art::crib {
class TChannelSelector : public TProcessor {
  public:
    TChannelSelector();
    ~TChannelSelector();

    void Init(TEventCollection *col) override;
    void Process() override;

  private:
    Bool_t fDebug;
    TString fSegmentedDataName;
    IntVec_t fSegID;

    TClonesArray **fInData;              //!
    TSegmentedData *fSegmentedData;      //!
    TCategorizedData **fCategorizedData; //!

    TChannelSelector(const TChannelSelector &) = delete;
    TChannelSelector &operator=(const TChannelSelector &) = delete;

    ClassDefOverride(TChannelSelector, 0);
};
} // namespace art::crib

#endif // end of #ifndef _CRIB_TCHANNELSELECTOR_H_
