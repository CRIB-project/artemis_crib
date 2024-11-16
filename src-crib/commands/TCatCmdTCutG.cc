/**
 * @file    TCatCmdTCutG.cc
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 18:34:21
 * @note    last modified: 2024-08-23 21:35:53
 * @details
 */

#include "TCatCmdTCutG.h"

#include <TCatHistManager.h>
#include <TCutG.h>
#include <TFile.h>
#include <TH2.h>
#include <TLine.h>
#include <TPad.h>
#include <iostream>

using art::crib::TCatCmdTCutG;

ClassImp(TCatCmdTCutG);

TCatCmdTCutG::TCatCmdTCutG() : fX(0.), fY(0.), fEventtype(0), fisFirst(true) {
    SetName("tcutg");
    SetTitle("get TCutG object (using mouse)");
}

TCatCmdTCutG::~TCatCmdTCutG() {}

TCatCmdTCutG *TCatCmdTCutG::Instance() {
    static TCatCmdTCutG instance;
    return &instance;
}

Long_t TCatCmdTCutG::Cmd(vector<TString>) {
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

    TString xname = ((TH2 *)hist_now)->GetXaxis()->GetTitle();
    TString yname = ((TH2 *)hist_now)->GetYaxis()->GetTitle();
    if (xname == "" || yname == "") {
        TString title = ((TH2 *)hist_now)->GetTitle();
        yname = title(0, title.First(":"));
        if (title.Contains(' ')) {
            TString tmp = title(title.First(":") + 1, title.Length());
            xname = tmp(0, tmp.First(' '));
        } else
            xname = title(title.First(":") + 1, title.Length());
    }

    Info("Cmd", "Xaxis name : %s  Yaxis name : %s", xname.Data(), yname.Data());

    Double_t ix = 0.0, iy = 0.0, x = 0.0, y = 0.0, bx = 0.0, by = 0.0;
    Int_t i = 0;
    std::vector<TLine *> lines;
    TCutG *cutg = new TCutG();
    cutg->SetVarX(xname);
    cutg->SetVarY(yname);

    Info("Cmd", "When you have finished specifying the area (last point), double-click on it.");
    fisFirst = true;
    fEventtype = 0;
    while (fEventtype != kButton1Double) {
        TCatCmdTCutG::Instance()->Run(&x, &y);
        Info("Cmd", "(x, y) = (%lf, %lf)", x, y);
        if (!fisFirst) {
            TLine *line = new TLine(bx, by, x, y);
            lines.push_back(line);
            lines[i - 1]->SetLineColor(kRed);
            lines[i - 1]->SetLineWidth(2);
            lines[i - 1]->Draw("same");
        }
        if (fisFirst) {
            ix = x;
            iy = y;
            fisFirst = false;
        }
        gPad->Update();
        cutg->SetPoint(i, x, y);
        bx = x;
        by = y;
        i++;
    }
    cutg->RemovePoint(i - 1);
    cutg->SetPoint(i - 1, ix, iy);

    for (Size_t l = 0; l < lines.size(); l++) {
        delete lines[l];
    }
    cutg->SetLineColor(kRed);
    cutg->SetLineWidth(2);
    cutg->Draw("l same");
    usleep(10000);
    gPad->Update();

    TString input;
    std::cout << "if you want to save it, input the TCutG name [name/exit] ";
    std::cin >> input;
    if (input == "exit" || input == "") {
        Info("Cmd", "exit, not saved");
        return 1;
    }

    cutg->SetName(input);
    TFile *file = new TFile("gate/" + input + ".root", "recreate");
    Info("Cmd", "Created gate/%s.root", input.Data());
    cutg->Write();
    file->Close();

    return 1;
}

Long_t TCatCmdTCutG::Run(TPad *pad, Double_t *x, Double_t *y) {
    if (!pad)
        return 0;
    pad->cd();

    pad->AddExec("ex_setxy", "TCatCmdTCutG::Instance()->GetEvent()");
    if (TObject *const obj = gPad->WaitPrimitive("setxy")) {
        gPad->GetListOfPrimitives()->Remove(obj);
        delete obj;
    } else {
        fX = fY = 0;
        return 0;
    }

    if (!x && !y) {
        // display coordinates when both of x and y are nullptr.
        printf("[xval] X: %f, Y: %f\n", fX, fY);
    } else {
        if (x)
            *x = fX;
        if (y)
            *y = fY;
    }

    return 1;
}

Long_t TCatCmdTCutG::Run(Double_t *x, Double_t *y) {
    return Run((TPad *)gPad, x, y);
}

void TCatCmdTCutG::GetEvent() {
    const Int_t event = gPad->GetEvent();
    const Int_t px = gPad->GetEventX();
    const Double_t xx = gPad->AbsPixeltoX(px);
    const Double_t x = gPad->PadtoX(xx);
    const Int_t py = gPad->GetEventY();
    const Double_t yy = gPad->AbsPixeltoY(py);
    const Double_t y = gPad->PadtoY(yy);

    if (!fisFirst) {
        TLine *line = new TLine(fX, fY, x, y);
        line->SetLineColor(kBlack);
        line->Draw("same");
        gPad->Update();
        delete line;
    }

    if (event == kButton1Double) {
        fEventtype = event;
        return;
    } else if (event != kButton1Up)
        return;

    gPad->DeleteExec("ex_setxy");

    fX = x;
    fY = y;
    gPad->GetListOfPrimitives()->AddLast(new TNamed("setxy", "setxy"));
}

void TCatCmdTCutG::Help() {
    std::cout << "make TCutG object" << std::endl;
}
