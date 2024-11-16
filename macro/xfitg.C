/*
   @File name     : xfitg.C
   @description   : gaussian fit
   @Author        : Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
   @Created date  : 2023-12-18 17:19:02
   @Last modified : 2023-12-18 17:19:08
*/

Double_t xfitg(TH1F *h, Double_t x1, Double_t x2) {
    Double_t tmp;
    if (x1 > x2) {
        tmp = x1;
        x1 = x2;
        x2 = tmp;
    }

    TF1 *func = new TF1(h->GetName(), "gaus(0)", x1, x2);
    func->SetParameters(h->GetBinContent(h->GetXaxis()->FindBin((x1 + x2) / 2.)), (x1 + x2) / 2., (x2 - x1) / 4.);
    h->Fit(func, "", "", x1, x2);
    func->Draw("same");

    gROOT->ProcessLine(Form("com [xfitg] meam=%g: FWHM=%g", func->GetParameter(1), 2.35 * func->GetParameter(2)));

    return func->GetParameter(1);
}