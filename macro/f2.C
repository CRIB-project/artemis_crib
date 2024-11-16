void f2() {
    gROOT->ProcessLine("add steering/chkf2.yaml");
    gROOT->ProcessLine("cd 0");
    gROOT->ProcessLine("zone");
}
