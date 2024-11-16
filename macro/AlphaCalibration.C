/**
 * @file    AlphaCalibration.C
 * @brief   alpha calibration macro for CRIB expereriment
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-12-21 14:20:03
 * @note    updated by Zhang Qian
 */

void AlphaCalibration(TH2 *h2 = NULL, TString foutpath = "test/test.dat", const Int_t alphaID = 0, const Double_t width = 15.0) {
    TString ARTEMIS_WORKDIR = gSystem->pwd();
    TString output_prm = ARTEMIS_WORKDIR + "/prm/" + foutpath;
    TString output_fig_base = ARTEMIS_WORKDIR + "/figure/calib/" + foutpath(0, foutpath.Last('.'));
    TString output_raw_path = output_fig_base + "/raw";
    TString output_calibration_path = output_fig_base + "/calibration";

    if (!h2) {
        std::cout << "Alpha calibration" << std::endl;
        std::cout << "Usage: AlphaCalibration(TH2* h2, TString foutpath=\"/test/test\", const Int_t alphaID=0, const Double_t width = 15.0\");" << std::endl;
        std::cout << "\th2       : 2D histogram, x: SSD fID, y: raw ADC channel" << std::endl;
        std::cout << "\tfoutpath : output parameter file path after prm/." << std::endl;
        std::cout << "\t         : NOTE: \"telx/ch2MeV_dEX.dat\", \"telx/ch2MeV_dEY.dat\", \"telx/ch2MeV_E.dat\" NEEDED" << std::endl;
        std::cout << "\talphaID  : alpha-2 (4.780, 5.480, 5.795), alpha-3 (3.148, 5.462, 5.771)" << std::endl;
        std::cout << "\twidth    : fitting width" << std::endl;
        return;
    }

    if (gSystem->ChangeDirectory(output_raw_path)) {
        gSystem->ChangeDirectory(ARTEMIS_WORKDIR);
    } else {
        gSystem->mkdir(output_raw_path, kTRUE);
    }

    if (gSystem->ChangeDirectory(output_calibration_path)) {
        gSystem->ChangeDirectory(ARTEMIS_WORKDIR);
    } else {
        gSystem->mkdir(output_calibration_path, kTRUE);
    }

    gROOT->ProcessLine("zone");
    h2->Draw("colz");
    gROOT->ProcessLine(Form("artcanvas->Print(\"%s/all_raw.png\")", output_fig_base.Data()));

    Double_t E_alpha[3];
    if (alphaID == 2) {
        E_alpha[0] = 4.780;
        E_alpha[1] = 5.480;
        E_alpha[2] = 5.795;
    } else if (alphaID == 3) {
        E_alpha[0] = 3.148;
        E_alpha[1] = 5.462;
        E_alpha[2] = 5.771;
    } else {
        std::cerr << "--ERROR: invalid alpha ID" << std::endl;
        return;
    }

    if (foutpath == "test/test.dat") {
        std::cerr << "--Warning: output file name is DEFAULT" << std::endl;
    }

    std::ofstream fout(output_prm.Data());
    if (!fout) {
        std::cerr << "--ERROR: cannot open " << output_prm << std::endl;
        return;
    }
    std::cout << "--Info: " << output_prm << " is created" << std::endl;
    fout << "# prm/" << foutpath << " is created from AlphaCalibration.C" << endl;
    fout << "# offset gain" << endl;

    // start searching the peaks
    Int_t ch_num = h2->GetNbinsX();
    TSpectrum *spec = new TSpectrum(3, 0.8);
    TF1 *f[3];
    for (Int_t i = 0; i < 3; ++i) {
        f[i] = new TF1(Form("f[%d]", i), "gaus(0)");
    }

    for (Int_t i = 0; i < ch_num; ++i) {
        Double_t x[3] = {0.0};
        TH1 *h1 = h2->ProjectionY(Form("h%03d", i), i + 1, i + 1);
        if (!h1) {
            std::cout << "--Warning: h1 is NULL (ch " << i + 1 << " )" << std::endl;
            fout << "0.0 0.0" << endl;
            continue;
        }
        gROOT->ProcessLine("zone");
        h1->Draw();
        Int_t nfound = spec->Search(h1, 1, "", 0.05);
        if (nfound != 3) {
            fout << "0.0 0.0" << endl;
            continue;
        }
        Double_t *xpeaks = spec->GetPositionX();
        std::sort(xpeaks, xpeaks + 3);

        for (Int_t j = 0; j < 3; ++j) {
            f[j]->SetRange(xpeaks[j] - width, xpeaks[j] + width);
            f[j]->SetParameters(200, xpeaks[j], 5.0); // constant, mean, sigma
            f[j]->SetParLimits(2, 0, 30);
            h1->Fit(f[j], "rq");
            x[j] = f[j]->GetParameter(1);
            f[j]->Draw("same");
        }

        gStyle->SetOptFit(0);
        gROOT->ProcessLine(Form("artcanvas->Print(\"%s/ch%d.png\")", output_raw_path.Data(), i));

        // calibration line
        TGraphErrors *gr1 = new TGraphErrors();
        for (Int_t j = 0; j < 3; ++j) {
            gr1->SetPoint(j, f[j]->GetParameter(1), E_alpha[j]);
            gr1->SetPointError(j, f[j]->GetParError(1), 0.0);
        }

        TF1 *line1 = new TF1("line1", "pol1");
        line1->SetParameters(1.0, 0.4); // offset, gain
        gStyle->SetOptFit(0);
        gr1->Fit(line1, "q");
        gROOT->ProcessLine("zone");
        gr1->Draw("ape");
        gr1->SetMarkerStyle(8);
        gr1->SetMarkerColor(1);
        gr1->SetMarkerSize(1);
        line1->Draw("same");
        gROOT->ProcessLine(Form("artcanvas->Print(\"%s/ch%d.png\")", output_calibration_path.Data(), i));

        fout << line1->GetParameter(0) << " " << line1->GetParameter(1) << endl;

        // check residual
        TGraphErrors *gr2 = new TGraphErrors();
        gStyle->SetOptFit(1);
        gr2->SetMarkerStyle(8);
        gr2->SetMarkerColor(1);
        gr2->SetMarkerSize(1);
        for (Int_t j = 0; j < 3; ++j) {
            gr2->SetPoint(j, E_alpha[j], line1->Eval(f[j]->GetParameter(1)) - E_alpha[j]);
            gr2->SetPointError(j, 0.0, TMath::Abs(line1->GetParameter(1) * f[j]->GetParError(1)));
        }

        TF1 *line2 = new TF1("line2", "pol1");
        line2->SetParameters(0.0, 0.1); // offset, gain
        gr2->Fit(line2, "q");

        gROOT->ProcessLine("zone");
        gr2->Draw("ape");
        line2->Draw("same");

        gROOT->ProcessLine(Form("artcanvas->Print(\"%s/ch%d_res.png\")", output_calibration_path.Data(), i));
    }
    fout.close();
}