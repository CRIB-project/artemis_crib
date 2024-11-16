/**
 * @file    TScalerMonitorProcessor.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-28 14:23:16
 * @note    last modified: 2024-08-23 20:54:16
 * @details
 */

#ifndef _CRIB_TSCALERMONITORPROCESSOR_H_
#define _CRIB_TSCALERMONITORPROCESSOR_H_

#include "TProcessor.h"
#include <TCanvas.h>
#include <TGraph.h>
#include <TScalerData.h>

namespace art {
class TScalerData;
} // namespace art

namespace art::crib {
class TScalerMonitorProcessor;
} // namespace art::crib

class art::crib::TScalerMonitorProcessor : public TProcessor {

  public:
    TScalerMonitorProcessor();
    ~TScalerMonitorProcessor();

    void Init(TEventCollection *col) override;
    void Process() override;

    void ProcessShort(Long_t now);
    void ProcessLong(Long_t now);

    const static Int_t SCALER_CH = 32;

  private:
    TCanvas *main;
    TPad *pad_s;
    TPad *pad_l;

    TString fInputColName;
    TScalerData **fInData; //!

    Int_t fShortInterval; // seconds
    Int_t fShortDuration; // seconds
    Int_t fLongInterval;  // seconds
    Int_t fLongDuration;  // seconds

    IntVec_t fClock;       // [ch, Hz]
    StringVec_t fChannels; // channel info

    Bool_t fIsFirst;

    // for short data member
    Long_t fStartShort;
    Long_t fCurrentShort;
    UInt_t fShortScatot[SCALER_CH] = {};  // scaler value
    UInt_t fShortScadiff[SCALER_CH] = {}; // scaler difference
    TGraph *fGr_s[SCALER_CH];             // live time chart for 32ch
    Int_t fShortCount;                    // TGraph point number

    // for long data member
    Long_t fStartLong;
    Long_t fCurrentLong;
    UInt_t fLongScatot[SCALER_CH] = {};  // scaler value
    UInt_t fLongScadiff[SCALER_CH] = {}; // scaler difference
    TGraph *fGr_l[SCALER_CH];            // live time chart for 32ch
    Int_t fLongCount;                    // TGraph point number

    IntVec_t fDisplayCh;               // display channels
    StringVec_t fNames;                // channel names
    Double_t fFactors[SCALER_CH] = {}; // channel factors

    IntVec_t fColor_list = {
        kRed,
        kBlue,
        kGreen,
        kMagenta,
        kViolet,
        kPink,
        kBlack,
        kCyan,
        kOrange,
        kSpring,
        kTeal,
        kAzure,
        30,
        31,
        32,
        33,
        34,
        35,
        36,
        37,
        38,
        39,
        40,
        41,
        42,
        43,
        44,
        45,
        46,
        47,
        48,
        49};

  protected:
    ClassDefOverride(TScalerMonitorProcessor, 1);
};
#endif // end of #ifdef _TSCALERMONITORPROCESSOR_H_
