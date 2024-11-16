// Arguments of AlphaCalibration
//    h2         : x: SSD fID, y: raw ADC channel
//    foutpath   : output parameter file path after prm/.
//               : NOTE: "telx/ch2MeV_dEX.dat", "telx/ch2MeV_dEY.dat", "telx/ch2MeV_E.dat" NEEDED
//    alphaID    : 2: alpha-2(4.780, 5.480, 5.795), 3: alpha-3(3.148, 5.462, 5.771)
//    width      : fitting width

void run_AlphaCalibration() {
    const TString ARTEMIS_WORKDIR = gSystem->pwd();

    gROOT->ProcessLine("fcd 0");
    gROOT->ProcessLine("zone");

    gStyle->SetOptStat(0);

    // example
    gROOT->ProcessLine("tree->Draw(\"tel1_dEX_raw.fCharge:tel1_dEX_raw.fID>>h11(16,-0.5,15.5, 300,300.,3000.)\",\"\",\"colz\")");
    gROOT->ProcessLine(".x " + ARTEMIS_WORKDIR + "/macro/AlphaCalibration.C(h11, \"tel1/ch2MeV_dEX.dat\",3,15.)");
    // gROOT->ProcessLine("tree->Draw(\"tel1_dEX_cal.fCharge:tel1_dEX_cal.fID>>h16(16,-0.5,15.5, 200,2.,6.)\",\"\",\"colz\")");
    // gROOT->ProcessLine("lgz");
    // gROOT->ProcessLine("artcanvas->Print(\"" + ARTEMIS_WORKDIR + "/figure/calib/tel1/ch2MeV_dEX/all_cal.png\")");
}
