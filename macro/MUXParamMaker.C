/**
 * @file    MUXParamMaker.C
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-12-23 15:16:39
 * @note    last modified: 2025-01-04 15:48:38
 * @details
 */

#include <cstdlib>
#include <fstream>
#include <memory>
#include <vector>

void Usage() {
    std::cout << "======================================================================\n"
              << "MUX position calibration\n"
              << "Usage: MUXParamMaker.C arguments:\n"
              << "    h1       : [TH1 *] create position histogram before using this\n"
              << "    telname  : [TString] telescope name (ex. tel1, tel2)\n"
              << "    sidename : [TString] should be dEX or dEY\n"
              << "    runname  : [TString] runname used for output filename\n"
              << "    runnum   : [TString] runnum used for output filename\n"
              << "    peaknum  : [int] the number of peaks, default = 16\n"
              << "======================================================================\n";
}

void MUXParamMaker(TH1 *h1 = nullptr,
                   const TString &telname = "telx",
                   const TString &sidename = "dE",
                   const TString &runname = "run",
                   const TString &runnum = "0000",
                   const int peaknum = 16) {
    // Fit range
    const double RANGE_OFFSET = 10.0;

    const char *artwork_dir = getenv("ARTEMIS_WORKDIR");
    if (!artwork_dir) {
        std::cerr << "Error: ARTEMIS_WORKDIR environment variable is not set.\n";
        return;
    }

    if (!h1) {
        std::cerr << "Error: Histogram is not provided.\n";
        Usage();
        return;
    }

    if (telname == "telx" || sidename == "dE") {
        std::cerr << "Error: telname and sidename must be specified.\n";
        Usage();
        return;
    }

    std::cout << "Info: Process MUXParamMakeer.C\n"
              << "      Parameters:\n"
              << "        h1 = " << h1->GetName() << "\n"
              << "        telname = " << telname << "\n"
              << "        sidename = " << sidename << "\n"
              << "        runname = " << runname << "\n"
              << "        runnum = " << runnum << "\n"
              << "        peaknum = " << peaknum << "\n";

    TString outFileName = Form("%s/prm/%s/pos_%s/%s%s.dat",
                               artwork_dir, telname.Data(), sidename.Data(),
                               runname.Data(), runnum.Data());

    std::ofstream fout(outFileName.Data());
    if (!fout) {
        std::cerr << " Error: Failed to create file " << outFileName << "\n ";
        return;
    }
    std::cout << "Info: Creating file " << outFileName << "\n";
    // header comment
    fout << "# Created using MUXParamMaker.C\n";

    auto spec = std::make_unique<TSpectrum>(peaknum, 1);
    int nfound = spec->Search(h1, 2, "", 0.001);
    if (nfound != peaknum) {
        std::cerr << "Error: Expected " << peaknum << " peaks, but found " << nfound << ".\n";
        for (int i = 0; i < peaknum; ++i) {
            fout << "0.0";
            if (i < peaknum - 1) {
                fout << ", ";
            }
        }
        fout << "\n";
        return;
    }

    double *xpeaks = spec->GetPositionX();
    std::sort(xpeaks, xpeaks + peaknum);

    std::vector<double> peaks_pos(peaknum, 0.0);
    std::vector<std::unique_ptr<TF1>> functions(peaknum);
    for (int i = 0; i < peaknum; ++i) {
        TString func_name = Form("f[%d]", i);
        functions[i] = std::make_unique<TF1>(func_name.Data(), "gaus(0)");
        functions[i]->SetRange(xpeaks[i] - RANGE_OFFSET, xpeaks[i] + RANGE_OFFSET);
        functions[i]->SetParameters(100, xpeaks[i], 1);
        functions[i]->SetParLimits(2, 0, 100);
        h1->Fit(functions[i].get(), "rq");
        peaks_pos[i] = functions[i]->GetParameter(1);
        functions[i]->DrawClone("same");
    }

    fout << peaks_pos[0] - RANGE_OFFSET << ", ";
    for (int i = 1; i < peaknum; ++i) {
        fout << (peaks_pos[i - 1] + peaks_pos[i]) / 2.0 << ", ";
    }
    fout << peaks_pos[peaknum - 1] + RANGE_OFFSET << "\n";
}
