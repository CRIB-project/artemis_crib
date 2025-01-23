/**
 * @file    TTreeBeamGenerator.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-09 15:57:38
 * @note    last modified: 2025-01-10 11:09:06
 * @details
 */

#ifndef CRIB_TTREEBEAMGENERATOR_H_
#define CRIB_TTREEBEAMGENERATOR_H_

#include <TProcessor.h>

class TClonesArray;

namespace art::crib {
class TTreeBeamGenerator : public TProcessor {
  public:
    TTreeBeamGenerator();
    ~TTreeBeamGenerator() override;

    void Init(TEventCollection *) override;
    void Process() override;

  private:
    TString fInputColName{"track"};
    TString fOutputColName{"beam"};
    TClonesArray **fInData{nullptr}; //!
    TClonesArray *fOutData{nullptr}; //!

    Int_t fMassNum{0};
    Int_t fAtmNum{0};
    Int_t fChargeNum{0};
    Double_t fBeamEnergy{100.0};
    Double_t fEsigma{1.0};
    Double_t fMass{0.0};

    // Copy constructor (prohibited)
    TTreeBeamGenerator(const TTreeBeamGenerator &rhs) = delete;
    // Assignment operator (prohibited)
    TTreeBeamGenerator &operator=(const TTreeBeamGenerator &rhs) = delete;

    ClassDefOverride(TTreeBeamGenerator, 2);
};
} // namespace art::crib

#endif // end of #ifndef CRIB_TTREEBEAMGENERATOR_H_
