void f3() {
    gROOT->ProcessLine("add steering/chkf3.yaml");
    gROOT->ProcessLine("cd 0");
    gROOT->ProcessLine("zone");
}
