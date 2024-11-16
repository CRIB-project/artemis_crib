/**
 * @file    TTSData.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022?
 * @note    last modified: 2024-08-24 10:29:05
 * @details
 */

#ifndef _TTSDATA_H_
#define _TTSDATA_H_

#include "TDataObject.h"
#include "constant.h"

namespace art::crib {
class TTSData;
} // namespace art::crib

class art::crib::TTSData : public TDataObject {
  public:
    typedef enum { kID,
                   kTiming } ESortType;
    typedef enum { kASC,
                   kDESC } ESortOrder;

    TTSData();
    ~TTSData();

    TTSData(const TTSData &rhs);
    TTSData &operator=(const TTSData &rhs);

    ULong64_t GetTS() const { return fTS; }
    void SetTS(ULong64_t arg) { fTS = arg; }
    Double_t GetTScal() const { return fTScal; }
    void SetTScal(Double_t arg) { fTScal = arg; }

    void Copy(TObject &dest) const override;
    void Clear(Option_t *opt = "") override;

  protected:
    ULong64_t fTS;
    Double_t fTScal;

    ClassDefOverride(TTSData, 1)
};

#endif // _TTSDATA_H_
