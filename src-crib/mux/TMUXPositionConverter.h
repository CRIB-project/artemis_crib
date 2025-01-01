/**
 * @file    TMUXPositionConverter.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 11:50:01
 * @note    last modified: 2025-01-01 20:30:45
 * @details
 */

#ifndef _CRIB_TMUXPOSITIONCONVERTER_H
#define _CRIB_TMUXPOSITIONCONVERTER_H

#include <TConverterBase.h>

namespace art::crib {
class TMUXPositionConverter : public TConverterBase {
  public:
    TMUXPositionConverter();
    ~TMUXPositionConverter();

    Double_t Convert(Double_t val) const override;
    void Print(Option_t *) const override;
    Bool_t LoadString(const TString &str) override;

  private:
    std::vector<Double_t> fParams;

    ClassDefOverride(TMUXPositionConverter, 2); // converter with first-degree polynomial
};
} // namespace art::crib

#endif // TMUXPOSITIONCONVERTER_H
