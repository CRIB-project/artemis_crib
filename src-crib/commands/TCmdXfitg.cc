/**
 * @file    TCmdXfitg.cc
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 17:51:08
 * @note    last modified: 2024-08-23 21:42:52
 * @details
 */

#include "TCmdXfitg.h"

#include <TCatCmdXval.h>
#include <TCatHistManager.h>
#include <TCmdComment.h>
#include <TF1.h>
#include <TH2.h>
#include <TPad.h>

using art::crib::TCmdXfitg;

ClassImp(TCmdXfitg);

const char *TCmdXfitg::kFuncNameBase = "f";
const char *TCmdXfitg::kLabelNameBase = "l";

TCmdXfitg::TCmdXfitg() {
    SetName("xfitg");
    SetTitle("fit using gausian (using mouse)");
}

TCmdXfitg::~TCmdXfitg() {}

Long_t TCmdXfitg::Cmd(vector<TString>) {
    if (gPad == nullptr) {
        Info("Cmd", "warning: no pad");
        return 1;
    }

    // K.Okawa 2022/9/27 add
    TObject *hist_now = gPad->GetPrimitive("htemp");
    if (!hist_now) {
        hist_now = TCatHistManager::Instance()->GetCurrent();
        if (!hist_now || !hist_now->InheritsFrom("TH1")) {
            Info("Cmd", "warning: no hist, please check using ls command");
            // std::cout << "  warning: no hist, please check using ls command" << std::endl;
            return 1;
        }
    }

    if (hist_now->InheritsFrom(TH2::Class())) {
        Info("Cmd", "warning: The current hist does not inherit from TH1 class (TH2)");
        // std::cout << "  warning: The current hist does not inherit from TH1 class (TH2)" << std::endl;
        return 1;
    }

    Double_t x1 = 0., y1 = 0., x2 = 0., y2 = 0.;
    Info("Cmd", "click on the lowest edge: ");
    TCatCmdXval::Instance()->Run(&x1, &y1);

    // static TLine *line = nullptr;
    // line = new TLine(x1, gPad->GetUymin(), x1, gPad->GetUymax());
    // line->SetLineColor(kBlack);
    // line->Draw();

    Info("Cmd", "click on the highest edge: ");
    TCatCmdXval::Instance()->Run(&x2, &y2);

    Double_t temp;
    if (x1 > x2) {
        temp = x1;
        x1 = x2;
        x2 = temp;
    }
    Info("Cmd", "X1: %g, X2: %g", x1, x2);
    // std::cout << " X1: " << x1 << ", X2: " << x2 << std::endl;

    // if(line) delete line;

    TF1 *func = new TF1(hist_now->GetName(), "gaus(0)", x1, x2);
    func->SetParameters(((y1 + y2) / 2) * (x2 - x1), (x1 + x2) / 2., (x2 - x1) / 4.);
    func->SetParLimits(2, 0, 1000. * (x2 - x1));
    dynamic_cast<TH1 *>(hist_now)->Fit(func, "", "", x1, x2);
    func->Draw("same");

    TCmdComment::Instance()->Run(
        TString::Format("[xfitg] mean=%g: FWHM=%g", func->GetParameter(1), 2.35 * func->GetParameter(2)));
    // K.Okawa end

    return 1;
}

void TCmdXfitg::Help() {
    std::cout << "this fit method use x1 x2 value and also use y1 y2 value to determine the first fitting "
                 "parameters.\nso if the fitting does not work, please try to click around the center of y axis"
              << std::endl;
}
