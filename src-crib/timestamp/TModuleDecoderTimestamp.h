/**
 * @file    TModuleDecoderTimestamp.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022?
 * @note    last modified: 2024-08-21 18:11:43
 * @details
 */

#ifndef _CRIB_TMODULEDECODERTIMESTAMP_h_
#define _CRIB_TMODULEDECODERTIMESTAMP_h_

#include "TModuleDecoder.h"

namespace art::crib {
class TModuleDecoderTimestamp;
}

class art::crib::TModuleDecoderTimestamp : public TModuleDecoder {
  public:
    static const int kID = 8;
    TModuleDecoderTimestamp();
    virtual ~TModuleDecoderTimestamp();
    Int_t Decode(char *buffer, const Int_t &size, TObjArray *seg) override;

  protected:
    TObjArray *fHitData; // array to tomprally store the data for the aggregation

    ClassDefOverride(TModuleDecoderTimestamp, 0) // timestamp decoder
};
#endif // _TMODULEDECODERTIMESTAMP_h_
