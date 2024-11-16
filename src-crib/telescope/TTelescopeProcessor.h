/**
 * @file    TTelescopeProcessor.h
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-17 16:53:01
 * @note    last modified: 2024-08-21 17:51:51
 * @details if no valid converter given, this processor does nothing.
 *          it assume we use DSSSD
 */

#ifndef _CRIB_TTELESCOPEPROCESSOR_H_
#define _CRIB_TTELESCOPEPROCESSOR_H_

#include <TProcessor.h>

namespace art::crib {
class TTelescopeProcessor;
class TDetectorParameter;
class TTargetParameter;
} // namespace art::crib

class TClonesArray;

class art::crib::TTelescopeProcessor : public TProcessor {
  public:
    // Default constructor
    TTelescopeProcessor();
    ~TTelescopeProcessor();

    void Init(TEventCollection *col) override;
    void Process() override;

    static const Int_t DEFAULT_SSD_MAX_NUMBER = 4;

  protected:
    TString fInputColName1; //! from X strip SSD
    TString fInputColName2; //! from Y strip SSD
    TString fInputColName3; //! from thick SSD array
    TString fOutputColName; //! output object

    TClonesArray **fInData1; //! TTimingChargeData array from X strip SSD
    TClonesArray **fInData2; //! TTimingChargeData array from Y strip SSD
    TClonesArray **fInData3; //! TTimingChargeData array from thick SSDs
    TClonesArray *fOutData;  //! TTelescopeData array

    Bool_t fIsDSSSD;
    Bool_t fUseRandom;
    Bool_t fInputHasData;

    // from parameter file
    TString fDetPrmName;
    TString fTargetPrmName;
    TClonesArray **fDetParameters;      //! all parameter info of TDetectorParameter
    TClonesArray **fTargetParameters;   //! all parameter info of TTargetParameter
    TDetectorParameter *fDetParameter;  //! one (this telescope) detector parameter
    TTargetParameter *fTargetParameter; //! one (default index 0) target parameter

    Bool_t fHasDetPrm;
    Bool_t fHasTargetPrm;

    Int_t fTelID;

  private:
    // Copy constructor (prohibited)
    TTelescopeProcessor(const TTelescopeProcessor &rhs) = delete;
    // Assignment operator (prohibited)
    TTelescopeProcessor &operator=(const TTelescopeProcessor &rhs) = delete;

    ClassDefOverride(TTelescopeProcessor, 2) // processor for calibration of timing and charge data
};

#endif // _TTELESCOPEPROCESSOR_H_
