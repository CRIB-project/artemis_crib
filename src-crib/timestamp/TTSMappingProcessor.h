/**
 * @file    TTSMappingProcessor.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022?
 * @note    last modified: 2025-01-08 10:37:21
 * @details
 */

#ifndef _CRIB_TTSMAPPINGPROCESSOR_H_
#define _CRIB_TTSMAPPINGPROCESSOR_H_

#include <TProcessor.h>

namespace art {
class TCategorizedData;
} // namespace art

namespace art::crib {
class TTSMappingProcessor;
} // namespace art::crib

class art::crib::TTSMappingProcessor : public TProcessor {
  public:
    TTSMappingProcessor();
    ~TTSMappingProcessor() override;

    void Init(TEventCollection *) override;
    void Process() override;

  protected:
    TString fInputColName;
    TString fOutputColName;
    TCategorizedData **fCategorizedData;
    TClonesArray *fOutputArray;

    Int_t fCatID;      // category id
    Int_t fDataTypeID; // typeid for data

  private:
    TTSMappingProcessor(const TTSMappingProcessor &) = delete;
    TTSMappingProcessor &operator=(const TTSMappingProcessor &) = delete;

    ClassDefOverride(TTSMappingProcessor, 0) // simple data mapper
};

#endif // _TTSMAPPINGPROCESSOR_H_
