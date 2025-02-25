void simEventLoop(TString steering = "", TString output = "", TString excited = "", TString cs_file = "") {
    if (steering == "") {
        std::cerr << "no steering" << std::endl;
        return;
    }

    TString output_process;
    if (output == "")
        output_process.Form("");
    else
        output_process.Form("OUTPUTFILE=%s", output.Data());

    TString excited_process;
    if (excited == "")
        excited_process.Form("");
    else
        excited_process.Form("EX_ENERGY=%s", excited.Data());

    TString cs_process;
    if (cs_file == "")
        cs_process.Form("");
    else
        cs_process.Form("CSFILE=%s", cs_file.Data());

    TString process;
    process.Form("add steering/%s.yaml %s %s %s",
                 steering.Data(), output_process.Data(), excited_process.Data(), cs_process.Data());

    gROOT->ProcessLine(process);
    art::TLoopManager::Instance()->GetLoop()->Resume();

    gROOT->ProcessLine(".q");
}
