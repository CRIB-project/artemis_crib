/**
 * @file    TCmdXblow.cc
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 17:46:27
 * @note    last modified: 2024-08-23 21:40:12
 * @details
 */

#include "TCmdXblow.h"

#include <TCatCmdXval.h>
#include <TCatHistManager.h>
#include <TF1.h>
#include <TH2.h>
#include <TPad.h>

using art::crib::TCmdXblow;

ClassImp(TCmdXblow);

const char *TCmdXblow::kFuncNameBase = "f";
const char *TCmdXblow::kLabelNameBase = "l";

TCmdXblow::TCmdXblow() {
    SetName("xblow");
    SetTitle("Crop 1D histogram  (using mouse)");
}

TCmdXblow::~TCmdXblow() {}

Long_t TCmdXblow::Cmd(vector<TString> args) {
    const TString opt = args.size() > 2 ? args[2] : "";

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

    if (hist_now->InheritsFrom(TH2::Class())) {
        Info("Cmd", "warning: The current hist does not inherit from TH1 class (TH2)");
        return 1;
    }

    Int_t nid = (gDirectory->GetList())->GetEntries();
    Run((TH1 *)hist_now);
    Info("Run", "id = %d hist is created", nid);
    TCatHistManager::Instance()->DrawObject(nid);

    return 1;
}

TH1 *TCmdXblow::Run(TH1 *h1) {

    Double_t x1 = 0., y1 = 0., x2 = 0., y2 = 0.;
    Info("Run", "click on the lowest edge: ");
    TCatCmdXval::Instance()->Run(&x1, &y1);

    Info("Run", "click on the highest edge: ");
    TCatCmdXval::Instance()->Run(&x2, &y2);

    Double_t temp;
    if (x1 > x2) {
        temp = x1;
        x1 = x2;
        x2 = temp;
    }
    Info("Run", "X1: %g, X2: %g", x1, x2);

    Int_t x1bin, x2bin, nbinsx;
    Float_t bc, btotal;

    const TString namesuffix = " blo", titlesuffix = " blo";

    x1bin = h1->GetXaxis()->FindBin(x1);
    x2bin = h1->GetXaxis()->FindBin(x2);

    nbinsx = (x2bin - x1bin) + 1;

    TH1F *hnew = new TH1F("name", "title", nbinsx, x1, x2);

    btotal = 0.;
    for (Int_t nx = x1bin; nx <= x2bin; nx++) {
        bc = h1->GetBinContent(nx);
        hnew->SetBinContent(nx - x1bin, bc);
        btotal += bc;
    }

    hnew->SetName(TString(h1->GetName()) + namesuffix);
    hnew->SetTitle(TString(h1->GetTitle()) + titlesuffix);
    hnew->SetEntries(btotal);

    return hnew;
}

void TCmdXblow::Help() {
    std::cout << "crop some part of the histogram" << std::endl;
}
