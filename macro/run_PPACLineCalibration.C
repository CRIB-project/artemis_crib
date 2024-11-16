// Arguments of PPACLineCalibration
//    h2         : PPAC x vs y histogram
//    PPACID     : CRIB PPAC Number
//    PPACpos    : 1 = PPACa (first layer is Y), 2 = PPACb (first layer is X from alpha source)
//    x_off      : x offset of alpha source from the PPAC center
//    y_off      : y offset of alpha source from the PPAC center
//    distance   : distance between alpha source and mask
//    xref       : reflectx in dlppac.yaml (0: no reflection 1: has reflection)

void run_PPACLineCalibration() {
    gStyle->SetOptStat(0);

    const TString ARTEMIS_WORKDIR = gSystem->pwd();

    gROOT->ProcessLine("fcd 0");
    gROOT->ProcessLine("zone");

    // F3aPPAC(#2) calib0326
    gROOT->ProcessLine("tree->Draw(\"f3appac.fY:f3appac.fX>>hxy(500,-50.,50., 500,-50.,50.)\",\"\",\"colz\")");
    gROOT->ProcessLine(".x " + ARTEMIS_WORKDIR + "/macro/PPACLineCalibration.C(hxy, 2, 1, 0.0, 2.2, 92.0, 1)");
}
