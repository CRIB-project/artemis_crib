/**
 * @file    TMUXCalibrationProcessor.h
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 11:08:53
 * @note    last modified: 2024-08-23 21:28:58
 * @details if no valid converter given, this processor does nothing.
 */

#ifndef _CRIB_TMUXCALIBRATIONPROCESSOR_H
#define _CRIB_TMUXCALIBRATIONPROCESSOR_H

#include <TProcessor.h>

namespace art {
class TConverterBase;
} // namespace art

namespace art::crib {
class TMUXCalibrationProcessor;
} // namespace art::crib

class TClonesArray;

class art::crib::TMUXCalibrationProcessor : public TProcessor {
  public:
    // Default constructor
    TMUXCalibrationProcessor();
    ~TMUXCalibrationProcessor();

    void Init(TEventCollection *col) override;
    void Process() override;

  protected:
    TString fInputColName;
    TString fOutputColName;
    TClonesArray **fInData; //!
    TClonesArray *fOutData; //!

    TString fTimingConverterArrayName;
    TString fChargeConverterArrayName;
    TString fPositionConverterArrayName;
    std::vector<TConverterBase *> *fTimingConverterArray;   //!
    std::vector<TConverterBase *> *fChargeConverterArray;   //!
    std::vector<TConverterBase *> *fPositionConverterArray; //!
    Bool_t fInputHasMUXData;
    Bool_t fHasReflection;
    Bool_t fInputIsDigital;

    Bool_t fIsSparse;

  private:
    // Copy constructor (prohibited)
    TMUXCalibrationProcessor(const TMUXCalibrationProcessor &rhs) = delete;
    // Assignment operator (prohibited)
    TMUXCalibrationProcessor &operator=(const TMUXCalibrationProcessor &rhs) = delete;

    ClassDefOverride(TMUXCalibrationProcessor, 1) // processor for calibration of timing and charge data
};

#endif // _TMUXCALIBRATIONPROCESSOR_H
