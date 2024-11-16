void run_MUXParamMaker() {
    const TString ARTEMIS_WORKDIR = gSystem->pwd();

    const TString RUNNAME = "run";
    const TString RUNNUM = "0015";

    gROOT->ProcessLine("fcd 0");
    gROOT->ProcessLine("zone");

    // tel1 dEX : RUN0015
    gROOT->ProcessLine("tree->Draw(\"tel1dEX_raw.fP1>>h1(700,2900.,3600.)\")");
    gROOT->ProcessLine(".x " + ARTEMIS_WORKDIR + "/macro/MUXParamMaker.C(h1, \"prm/tel1/pos_dEX/" + RUNNAME + RUNNUM + ".dat\")");
    gROOT->ProcessLine("artcanvas->Print(\"" + ARTEMIS_WORKDIR + "/figure/calib/tel1/pos_dEX/" + RUNNAME + RUNNUM + ".png\")");

    // tel1 dEY : RUN0015
    gROOT->ProcessLine("tree->Draw(\"tel1dEY_raw.fP1>>h2(700,2400.,3100.)\")");
    gROOT->ProcessLine(".x " + ARTEMIS_WORKDIR + "/macro/MUXParamMaker.C(h2, \"prm/tel1/pos_dEY/" + RUNNAME + RUNNUM + ".dat\")");
    gROOT->ProcessLine("artcanvas->Print(\"" + ARTEMIS_WORKDIR + "/figure/calib/tel1/pos_dEY/" + RUNNAME + RUNNUM + ".png\")");

    // gROOT->ProcessLine(".q");
}
