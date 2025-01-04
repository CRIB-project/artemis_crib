void run_MUXParamMaker() {
    const TString ARTEMIS_WORKDIR = gSystem->pwd();

    const TString RUNNAME = "run";
    const TString RUNNUM = "0155";

    gROOT->ProcessLine("fcd 0");
    gROOT->ProcessLine("zone");

    gROOT->ProcessLine("tree->Draw(\"tel4dEX_raw.fP1>>h1(500,3900.,4400.)\")");
    gROOT->ProcessLine(".x " + ARTEMIS_WORKDIR + "/macro/MUXParamMaker.C(h1, \"tel4\", \"dEX\", \"" + RUNNAME + "\", \"" + RUNNUM + "\", 16)");

    // gROOT->ProcessLine(".q");
}
