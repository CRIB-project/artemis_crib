/**
 * @file    TMUXPositionValidator.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-30 10:02:32
 * @note    last modified: 2025-01-02 22:52:20
 * @details
 */

#ifndef CRIB_TMUXPOSITIONVALIDATOR_H_
#define CRIB_TMUXPOSITIONVALIDATOR_H_

#include <TProcessor.h>

class TClonesArray;

namespace art::crib {
class TMUXPositionValidator : public TProcessor {
  public:
    TMUXPositionValidator();
    ~TMUXPositionValidator();

    void Init(TEventCollection *col) override;
    void Process() override;

  private:
    TString fInputColName;  // name of input collection
    TString fOutputColName; // name of output collection
    TClonesArray *fInData;  //! input
    TClonesArray *fOutData; //! output

    DoubleVec_t fValidPositionRange;

    TMUXPositionValidator(const TMUXPositionValidator &rhs);
    TMUXPositionValidator &operator=(const TMUXPositionValidator &rhs);

    ClassDefOverride(TMUXPositionValidator, 1) // validate time in certain window
};
} // namespace art::crib

#endif // CRIB_TMUXPOSITIONVALIDATOR_H_
