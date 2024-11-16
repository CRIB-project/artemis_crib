/**
 * @file    TEvtNumProcessor.cc
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022?
 * @note    last modified: 2024-08-23 20:49:16
 * @details originally made by J. W. Hwang
 */

#include "TEvtNumProcessor.h"

using art::crib::TEvtNumProcessor;

ClassImp(TEvtNumProcessor);

TEvtNumProcessor::TEvtNumProcessor() : fCurNum(0), fLimEvNum(0), fPriEv(kFALSE), fPriEvNum(100) {
    RegisterProcessorParameter("EventNumLimit", "the limit of the event number",
                               fLimEvNum, 0);
    RegisterOptionalParameter("PrintEvent", "print the current event number",
                              fPriEv, kFALSE);
    RegisterProcessorParameter("PrintEventNum", "event number of frequency",
                               fPriEvNum, 100);
}

TEvtNumProcessor::~TEvtNumProcessor() {}

void TEvtNumProcessor::Init(TEventCollection *) {}

void TEvtNumProcessor::EndOfRun() {
    std::cout << "  Event Number: " << fCurNum << std::endl;
}

void TEvtNumProcessor::Process() {
    fCurNum++;
    if (fPriEv && !(fCurNum % fPriEvNum)) {
        std::cout << "\r  Event Number: " << fCurNum;
        std::cout.flush();
    }

    if (fLimEvNum && fCurNum >= fLimEvNum) {
        std::cout << "\r  Event Number: " << fCurNum << std::endl;
        Info("Process", "The number of event is limited to %d.", fCurNum);
        SetStopLoop();
    }
}
