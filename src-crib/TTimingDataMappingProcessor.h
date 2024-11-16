/**
 * @file    TTimingDataMappingProcessor.h
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-07-19 20:20:07
 * @note    last modified: 2024-08-21 17:23:56
 * @details
 */

#ifndef _TTIMINGDATAMAPPINGPROCESSOR_H_
#define _TTIMINGDATAMAPPINGPROCESSOR_H_

#include <TProcessor.h>

namespace art {
class TCategorizedData;
} // namespace art

namespace art::crib {
class TTimingDataMappingProcessor;
} // namespace art::crib

class art::crib::TTimingDataMappingProcessor : public TProcessor {
  public:
    TTimingDataMappingProcessor();
    ~TTimingDataMappingProcessor();

    void Init(TEventCollection *) override;
    void Process() override;

  protected:
    TString fInputColName;
    TString fOutputColName;
    TCategorizedData **fCategorizedData;
    TClonesArray *fOutputArray;

    Int_t fCatID;      // category id
    Int_t fDataTypeID; // typeid for data

    Bool_t fIsSparse;

  private:
    TTimingDataMappingProcessor(const TTimingDataMappingProcessor &) = delete;
    TTimingDataMappingProcessor &operator=(const TTimingDataMappingProcessor &) = delete;

    ClassDefOverride(TTimingDataMappingProcessor, 0) // simple data mapper
};

#endif // _TTIMINGDATAMAPPINGPROCESSOR_H_
