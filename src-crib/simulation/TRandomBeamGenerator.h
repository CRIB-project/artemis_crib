/**
 * @file    TRandomBeamGenerator.h
 * @brief   position and angle random beam generator
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-09 17:10:35
 * @note    last modified: 2024-09-03 17:18:53
 * @details
 */

#ifndef _CRIB_TRANDOMBEAMGENERATOR_H_
#define _CRIB_TRANDOMBEAMGENERATOR_H_

#include <TProcessor.h>

namespace art::crib {
class TRandomBeamGenerator;
}

class TClonesArray;

class art::crib::TRandomBeamGenerator : public TProcessor {
  public:
    TRandomBeamGenerator();
    ~TRandomBeamGenerator();

    void Init(TEventCollection *) override;
    void Process() override;

  protected:
    TString fOutputColName;
    TString fOutputTrackColName;

    TClonesArray *fOutData;      //!
    TClonesArray *fOutTrackData; //!

    Int_t fMassNum;
    Int_t fAtmNum;
    Int_t fChargeNum;
    Double_t fBeamEnergy;

    DoubleVec_t fInitialPosition;

    Double_t fXsigma;
    Double_t fYsigma;
    Double_t fAsigma;
    Double_t fBsigma;
    Double_t fEsigma;

  private:
    Double_t fMass; /// beam particle mass (MeV)

    // Copy constructor (prohibited)
    TRandomBeamGenerator(const TRandomBeamGenerator &rhs) = delete;
    // Assignment operator (prohibited)
    TRandomBeamGenerator &operator=(const TRandomBeamGenerator &rhs) = delete;

    ClassDefOverride(TRandomBeamGenerator, 2);
};

#endif // end of #ifndef _TRANDOMBEAMGENERATOR_H_
