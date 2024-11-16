/**
 * @file    TNBodyReactionProcessor.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-08-01 13:11:23
 * @note    last modified: 2024-08-23 21:16:24
 * @details
 */

#ifndef _CRIB_TNBODYREACTIONPROCESSOR_H_
#define _CRIB_TNBODYREACTIONPROCESSOR_H_

#include <TGenPhaseSpace.h>
#include <TGraph.h>
#include <TProcessor.h>
#include <TSrim.h> // TSrim library

namespace art::crib {
class TNBodyReactionProcessor;
} // namespace art::crib

class TClonesArray;

class art::crib::TNBodyReactionProcessor : public TProcessor {
  public:
    TNBodyReactionProcessor();
    ~TNBodyReactionProcessor();

    void Init(TEventCollection *col) override;
    void Process() override;

  protected:
    TString fInputColName;
    TString fOutputColName;
    TString fOutputReacColName;
    TClonesArray **fInData;     //!
    TClonesArray *fOutData;     //!
    TClonesArray *fOutReacData; //!

    /// @brief used only initialization at TSrim object
    IntVec_t fBeamNucleus;
    Double_t fBeamEnergy;

    Bool_t fTargetIsGas;
    TString fTargetName;
    Int_t fTargetMassNum;
    Int_t fTargetAtmNum;
    Double_t fTargetThickness;
    Double_t fTargetPressure;

    Int_t fDecayNum;
    IntVec_t fReacMassNum;
    IntVec_t fReacAtmNum;
    DoubleVec_t fExciteLevel;
    TString fCSDataPath;
    Int_t fCSType;

    TGenPhaseSpace event;

    TSrim *srim; /// SRIM table

    const Double_t deg2rad = TMath::DegToRad();
    const Double_t c = 299.792458; // mm/ns

  private:
    //! 1. cross section function:    (x, y) = (beam LAB energy (MeV), arbitrary unit)
    //! 2. convert x using range:     (x, y) = (range (mm), arbitrary unit)
    //! 3. integrate the 2. function: (x, y) = (range (mm), arbitrary unit)
    //! 4. inversed 3. function:      (x, y) = (arbitrary unit, range (mm))
    //! 5. get pos with random number: uniform X -> get Y value
    //! 6. get reac pos with the Y value: init_range - Y => distance
    TGraph *gr_generating_func;     //! 3. function
    TGraph *gr_generating_func_inv; //! 4. function
    void InitGeneratingFunc(void);  //! read from data file and set 3. 4. functions

    /**
     * @fn random generator
     * From beam range and target thickness, get random beam energy just before reaction
     * and distance from target window (for gas target)
     * @param (range) range of the beam
     * @return distance of the beam travel before the reaction
     */
    Double_t GetRandomReactionDistance(Double_t range);

    TLorentzVector GetLossEnergyVector(TLorentzVector vec, Double_t eloss);

    TNBodyReactionProcessor(const TNBodyReactionProcessor &rhs) = delete;
    TNBodyReactionProcessor &operator=(const TNBodyReactionProcessor &rhs) = delete;

    ClassDefOverride(TNBodyReactionProcessor, 1)
};

#endif // end of #ifndef _TNBODYREACTIONPROCESSOR_H_
