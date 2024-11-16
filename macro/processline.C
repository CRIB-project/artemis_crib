void processline(TString line){
  gROOT->ProcessLine(line.Data());
  gROOT->ProcessLine(".q");
}
