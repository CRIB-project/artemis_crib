/**
 * @file    TMUXCalibrationProcessor.h
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 11:08:53
 * @note    last modified: 2025-01-02 18:31:59
 * @details if no valid converter given, this processor does nothing.
 */

#ifndef CRIB_TMUXCALIBRATIONPROCESSOR_H
#define CRIB_TMUXCALIBRATIONPROCESSOR_H

#include <TProcessor.h>

class TClonesArray;

namespace art::crib {
class TMUXCalibrationProcessor : public TProcessor {
  public:
    // Default constructor
    TMUXCalibrationProcessor();
    ~TMUXCalibrationProcessor();

    void Init(TEventCollection *col) override;
    void Process() override;

  protected:
    TString fInputColName;
    TString fOutputColName;
    TClonesArray *fInData;  //!
    TClonesArray *fOutData; //!

    TString fTimingConverterArrayName;
    TString fChargeConverterArrayName;
    TString fPositionConverterArrayName;

    TClonesArray *fTimingConverterArray;   //!
    TClonesArray *fChargeConverterArray;   //!
    TClonesArray *fPositionConverterArray; //!

    Bool_t fHasReflection;
    Bool_t fInputIsDigital;

  private:
    // Copy constructor (prohibited)
    TMUXCalibrationProcessor(const TMUXCalibrationProcessor &rhs) = delete;
    // Assignment operator (prohibited)
    TMUXCalibrationProcessor &operator=(const TMUXCalibrationProcessor &rhs) = delete;

    ClassDefOverride(TMUXCalibrationProcessor, 2); // processor for calibration of timing and charge data
};
} // namespace art::crib

#endif // CRIB_TMUXCALIBRATIONPROCESSOR_H
