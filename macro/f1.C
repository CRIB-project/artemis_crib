void f1() {
    gROOT->ProcessLine("add steering/chkf1.yaml");
    gROOT->ProcessLine("cd 0");
    gROOT->ProcessLine("zone");
}
