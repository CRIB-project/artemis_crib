/**
 * @file    TMUXPositionValidator.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-30 10:02:32
 * @note    last modified: 2024-08-21 17:47:25
 * @details
 */

#ifndef _CRIB_TMUXPOSITIONVALIDATOR_H_
#define _CRIB_TMUXPOSITIONVALIDATOR_H_

#include <TProcessor.h>

namespace art::crib {
class TMUXPositionValidator;
} // namespace art::crib

class TClonesArray;

class art::crib::TMUXPositionValidator : public TProcessor {
  public:
    TMUXPositionValidator();
    ~TMUXPositionValidator();

    TMUXPositionValidator(const TMUXPositionValidator &rhs);
    TMUXPositionValidator &operator=(const TMUXPositionValidator &rhs);

  protected:
    virtual void Init(TEventCollection *col) override;
    virtual void Process() override;

  private:
    TString fInputName;    // name of input collection
    TString fOutputName;   // name of output collection
    TClonesArray **fInput; //! input
    TClonesArray *fOutput; //! output

    //   Int_t fTimeVariable; // 0 time 1 timestamp
    DoubleVec_t fValidPositionRange;
    Double_t fValidPositionMin;
    Double_t fValidPositionMax;

    ClassDefOverride(TMUXPositionValidator, 1) // validate time in certain window
};

#endif // _TMUXPOSITIONVALIDATOR_H_
