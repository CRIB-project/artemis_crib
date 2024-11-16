void evtloop(TString steering = "", TString NAME = "", TString NUM = "") {
    if (steering == "") {
        std::cerr << "no steering" << std::endl;
        return;
    }

    TString name_process;
    if (NAME == "")
        name_process.Form("");
    else
        name_process.Form("NAME=%s", NAME.Data());

    TString num_process;
    if (NUM == "")
        num_process.Form("");
    else
        num_process.Form("NUM=%s", NUM.Data());

    TString process;
    process.Form("add steering/%s.yaml %s %s", steering.Data(), name_process.Data(), num_process.Data());

    gROOT->ProcessLine(process);
    art::TLoopManager::Instance()->GetLoop()->Resume();

    gROOT->ProcessLine(".q");
}
