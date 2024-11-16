/*
   @File name     : xstatus.C
   @description   :
   @Author        : Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
   @Created date  : 2023-12-18 17:17:55
   @Last modified : 2023-12-18 17:18:07
*/

Double_t xstatus(TH2F *h, Double_t x1, Double_t x2, Double_t y1, Double_t y2) {
    Double_t tmp;
    if (x1 > x2) {
        tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y1 > y2) {
        tmp = y1;
        y1 = y2;
        y2 = tmp;
    }

    static TLine *lines[4];
    lines[0] = new TLine(x1, y1, x2, y1);
    lines[1] = new TLine(x1, y2, x2, y2);
    lines[2] = new TLine(x1, y1, x1, y2);
    lines[3] = new TLine(x2, y1, x2, y2);
    for (Int_t i = 0; i < 4; ++i) {
        lines[i]->SetLineColor(kRed);
        lines[i]->Draw();
    }

    Int_t x1bin, x2bin, y1bin, y2bin;
    x1bin = h->GetXaxis()->FindBin(x1);
    x2bin = h->GetXaxis()->FindBin(x2);
    y1bin = h->GetYaxis()->FindBin(y1);
    y2bin = h->GetYaxis()->FindBin(y2);

    Int_t selected = (Int_t)h->Integral(x1bin, x2bin, y1bin, y2bin);
    Int_t total = (Int_t)h->GetEntries();

    gROOT->ProcessLine(Form("com [xstatus] %d/%d = %lf", selected, total, (Double_t)selected / total));

    return (Double_t)selected / total;
}