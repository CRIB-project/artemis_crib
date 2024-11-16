/**
 * @file    TCmdXYblow.cc
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 17:38:35
 * @note    last modified: 2024-08-23 21:46:06
 * @details
 */

#include "TCmdXYblow.h"

#include <TCatCmdXval.h>
#include <TCatHistManager.h>
#include <TF1.h>
#include <TH2.h>
#include <TPad.h>

using art::crib::TCmdXYblow;

ClassImp(TCmdXYblow);

const char *TCmdXYblow::kFuncNameBase = "f";
const char *TCmdXYblow::kLabelNameBase = "l";

TCmdXYblow::TCmdXYblow() {
    SetName("xyblow");
    SetTitle("Crop histogram  (using mouse)");
}

TCmdXYblow::~TCmdXYblow() {}

Long_t TCmdXYblow::Cmd(vector<TString> args) {
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

    if (!hist_now->InheritsFrom(TH2::Class())) {
        Info("Cmd", "warning: The current hist does not inherit from TH2 class");
        return 1;
    }

    Int_t nid = (gDirectory->GetList())->GetEntries();
    Run((TH2 *)hist_now, opt);
    Info("Run", "id = %d hist is created", nid);
    TCatHistManager::Instance()->DrawObject(nid, "colz");

    return 1;
}

TH2 *TCmdXYblow::Run(TH2 *h2, Option_t *) {

    Double_t x1 = 0., y1 = 0., x2 = 0., y2 = 0.;
    Info("Run", "click on one corner: ");
    TCatCmdXval::Instance()->Run(&x1, &y1);
    Info("Run", "X1: %g, Y1: %g", x1, y1);

    Info("Run", "click on the other corner: ");
    TCatCmdXval::Instance()->Run(&x2, &y2);
    Info("Run", "X2: %g, Y2: %g", x2, y2);

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

    Int_t x1bin, x2bin, y1bin, y2bin, nbinsx, nbinsy;
    Float_t bc, btotal;

    const TString namesuffix = " blo", titlesuffix = " blo";

    x1bin = h2->GetXaxis()->FindBin(x1);
    x2bin = h2->GetXaxis()->FindBin(x2);
    y1bin = h2->GetYaxis()->FindBin(y1);
    y2bin = h2->GetYaxis()->FindBin(y2);

    nbinsx = (x2bin - x1bin) + 1;
    nbinsy = (y2bin - y1bin) + 1;

    TH2F *hnew = new TH2F("name", "title", nbinsx, x1, x2, nbinsy, y1, y2);

    btotal = 0.;
    for (Int_t nx = x1bin; nx <= x2bin; nx++) {
        for (Int_t ny = y1bin; ny <= y2bin; ny++) {
            bc = h2->GetBinContent(nx, ny);
            hnew->SetBinContent(nx - x1bin, ny - y1bin, bc);
            btotal += bc;
        }
    }

    hnew->SetName(TString(h2->GetName()) + namesuffix);
    hnew->SetTitle(TString(h2->GetTitle()) + titlesuffix);
    hnew->SetEntries(btotal);

    return hnew;
}

void TCmdXYblow::Help() {
    std::cout << "crop some part of the histogram" << std::endl;
}
