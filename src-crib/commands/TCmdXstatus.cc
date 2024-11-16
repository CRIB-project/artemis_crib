/**
 * @file    TCmdXstatus.cc
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 10:54:46
 * @note    last modified: 2024-08-23 21:44:35
 * @details
 */

#include "TCmdXstatus.h"

#include <TCatCmdXval.h>
#include <TCatHistManager.h>
#include <TCmdComment.h>
#include <TF1.h>
#include <TH2.h>
#include <TLine.h>
#include <TPad.h>

using art::crib::TCmdXstatus;

ClassImp(TCmdXstatus);

const char *TCmdXstatus::kFuncNameBase = "f";
const char *TCmdXstatus::kLabelNameBase = "l";

TCmdXstatus::TCmdXstatus() {
    SetName("xstatus");
    SetTitle("get status (using mouse)");
}

TCmdXstatus::~TCmdXstatus() {}

Long_t TCmdXstatus::Cmd(vector<TString>) {
    if (gPad == nullptr) {
        Info("Cmd", "warning: no pad");
        return 1;
    }

    TObject *hist_now = gPad->GetPrimitive("htemp");
    if (!hist_now) {
        hist_now = TCatHistManager::Instance()->GetCurrent();
        if (!hist_now || !hist_now->InheritsFrom("TH1")) {
            Info("Cmd", "warning: no hist, please check using ls command");
            return 1;
        }
    }

    if (!hist_now->InheritsFrom(TH2::Class())) {
        Info("Cmd", "warning: The current hist does not inherit from TH2 class");
        return 1;
    }

    Double_t x1 = 0., y1 = 0., x2 = 0., y2 = 0.;
    Info("Cmd", "click on one corner: ");
    TCatCmdXval::Instance()->Run(&x1, &y1);
    Info("Cmd", "X1: %g, Y1: %g", x1, y1);

    Info("Cmd", "click on the other corner: ");
    TCatCmdXval::Instance()->Run(&x2, &y2);
    Info("Cmd", "X2: %g, Y2: %g", x2, y2);

    Double_t temp;
    if (x1 > x2) {
        temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if (y1 > y2) {
        temp = y1;
        y1 = y2;
        y2 = temp;
    }

    static TLine *lines[4];
    for (Int_t i = 0; i < 4; ++i) {
        if (lines[i]) {
            delete lines[i];
        }
    }

    lines[0] = new TLine(x1, y1, x2, y1);
    lines[1] = new TLine(x1, y2, x2, y2);
    lines[2] = new TLine(x1, y1, x1, y2);
    lines[3] = new TLine(x2, y1, x2, y2);

    for (Int_t i = 0; i < 4; ++i) {
        lines[i]->SetLineColor(kRed);
        lines[i]->Draw();
    }

    Int_t x1bin, x2bin, y1bin, y2bin;
    x1bin = dynamic_cast<TH2 *>(hist_now)->GetXaxis()->FindBin(x1);
    x2bin = dynamic_cast<TH2 *>(hist_now)->GetXaxis()->FindBin(x2);
    y1bin = dynamic_cast<TH2 *>(hist_now)->GetYaxis()->FindBin(y1);
    y2bin = dynamic_cast<TH2 *>(hist_now)->GetYaxis()->FindBin(y2);

    Int_t selected = (Int_t)(dynamic_cast<TH2 *>(hist_now)->Integral(x1bin, x2bin, y1bin, y2bin));
    Int_t total = (Int_t)(dynamic_cast<TH2 *>(hist_now)->Integral());
    Double_t ratio = 0.0;

    std::cout << "------------------" << std::endl;
    std::cout << "selected = " << selected << ", total = " << total << std::endl;
    if (total != 0) {
        ratio = (Double_t)selected / total;
        std::cout << "ratio  = " << ratio << " (" << ratio * 100 << "%)" << std::endl;
        TCmdComment::Instance()->Run(TString::Format("[xstatus] %d/%d: ratio=%g", selected, total, ratio));
    } else {
        std::cout << "ratio  = " << "NaN" << std::endl;
        TCmdComment::Instance()->Run(TString::Format("[xstatus] %d/%d: ratio=NaN", selected, total));
    }

    return 1;
}

void TCmdXstatus::Help() {
    std::cout << "count and ratio" << std::endl;
}
