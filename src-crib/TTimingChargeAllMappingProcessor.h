/**
 * @file    TTimingChargeAllMappingProcessor.h
 * @brief   from TTimingChargeMappingProcessor, both E and T
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022?
 * @note    last modified: 2024-08-21 17:24:11
 * @details
 */

#ifndef _CRIB_TTIMINGCHARGEALLMAPPINGPROCESSOR_H
#define _CRIB_TTIMINGCHARGEALLMAPPINGPROCESSOR_H

#include <TProcessor.h>

namespace art {
class TCategorizedData;
} // namespace art

namespace art::crib {
class TTimingChargeAllMappingProcessor;
} // namespace art::crib

class TClonesArray;

class art::crib::TTimingChargeAllMappingProcessor : public TProcessor {

  public:
    // Default constructor
    TTimingChargeAllMappingProcessor();
    ~TTimingChargeAllMappingProcessor();

    void Init(TEventCollection *) override;
    void Process() override;

  protected:
    TString fInputColName;
    TString fOutputColName;
    TCategorizedData **fCategorizedData; //!
    TClonesArray *fPlastic;              //!

    Int_t fCatID;               // default: 21 (spla in anapaw)
    Int_t fChargeType;          // 0: V1190 width (default), 1: QDC (V792 etc)
    Int_t fChargeTypeID;        // used only when fChargeType is QDC
    Int_t fTimingTypeID;        // Typeid for timing (& width when fChargeType is V1190width)
    Bool_t fTrailingComesFirst; // F: TL->TT (default), T: TT->TL (for QTC etc.)

    Bool_t fIsSparse;

  private:
    // Copy constructor (prohibited)
    TTimingChargeAllMappingProcessor(const TTimingChargeAllMappingProcessor &) = delete;
    // Assignment operator (prohibited)
    TTimingChargeAllMappingProcessor &operator=(const TTimingChargeAllMappingProcessor &) = delete;

    void MapEdgedTime(const TObjArray *);
    void MapPairedTimeCharge(const TObjArray *);
    void MapTimeCharge(const TObjArray *, const TObjArray *);

    enum EChargeTypeID { kWIDTH,
                         kQDC,
                         kHINP,
    };

    ClassDefOverride(TTimingChargeAllMappingProcessor, 0) // processor for mapping timine and charge data
};

#endif // TTIMINGCHARGEALLMAPPINGPROCESSOR_H
