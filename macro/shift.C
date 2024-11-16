// usage: $ a 'macro/shift.C("run","0100")'
// or
// usage: $ a
//      : artemis [] .x macro/shift.C("run","0100")

#include "chkmuxpos.C"
#include "xfitg.C"
#include "xstatus.C"

void shift(TString RUNNAME = "", TString RUNNUM = "") {
    TString dummy = "dummy";

    gROOT->ProcessLine(Form("add steering/shift.yaml NAME=%s NUM=%s", RUNNAME.Data(), RUNNUM.Data()));
    art::TLoopManager::Instance()->GetLoop()->Resume();

    gROOT->ProcessLine("cd");

    // get mwdca/b information, position and efficiency
    gROOT->ProcessLine("zone 2 2");
    gROOT->ProcessLine("ht 2/7 colz");
    gROOT->ProcessLine("ht 2/9");
    gROOT->ProcessLine("ht 3/7 colz");
    gROOT->ProcessLine("ht 3/9");
    gPad->Update();

    //    gROOT->ProcessLine("save");
    std::cout << "is it OK? [y/n, exit] ";
    std::cin >> dummy;
    if (dummy == "n") {
        // std::cout << "please recalibrate the MUX position" << std::endl;
    }
    if (dummy == "exit")
        return;

    /// get mwdcs tracking information
    /// (Okawa: from 1/26 we don't use mwdca data, it is almost no meaning)
    /// only MWDCb data
    gROOT->ProcessLine("cd mwdcb");
    gROOT->ProcessLine("zone 2 2");
    gROOT->ProcessLine("ht 7 colz");
    gROOT->ProcessLine("ht 12");
    gPad->Update();
    gROOT->ProcessLine("xf");
    gROOT->ProcessLine("ht 13");
    gPad->Update();
    gROOT->ProcessLine("xf");
    gPad->Update();

    /// previous one
    // gROOT->ProcessLine("cd 0");
    // gROOT->ProcessLine("zone 2 2");
    // gROOT->ProcessLine("ht 5 colz");
    // gROOT->ProcessLine("ht 4 colz");
    // gROOT->ProcessLine("ht 16");
    // gPad->Update();
    // gROOT->ProcessLine("xf");
    // gROOT->ProcessLine("ht 17");
    // gPad->Update();
    // gROOT->ProcessLine("xf");
    // gPad->Update();
    ////    gROOT->ProcessLine("save");

    std::cout << "is it OK? [y/n, exit] ";
    std::cin >> dummy;
    if (dummy == "n") {
        // std::cout << "please recalibrate the MUX position" << std::endl;
    }
    if (dummy == "exit")
        return;

    /// get MWDCa and MWDCb Charge information
    /// (Okawa: from 1/26 we don't use mwdca data, so commented out the dca part)
    gROOT->ProcessLine("cd");
    gROOT->ProcessLine("zone 2 2");
    // gROOT->ProcessLine("cd 2");
    // gROOT->ProcessLine("ht 4 colz");
    // gROOT->ProcessLine("ht 2");
    // gPad->Update();
    // gROOT->ProcessLine("xf");
    // gROOT->ProcessLine("cd");
    gROOT->ProcessLine("cd 3");
    gROOT->ProcessLine("ht 4 colz");
    gROOT->ProcessLine("ht 2");
    auto dcb_charge_ht = static_cast<TH1 *>(TCatHistManager::Instance()->GetObject(2));
    std::cout << "\033[1m===========Overflow check===============\033[m" << std::endl;
    std::cout << "Entries = " << dcb_charge_ht->GetEntries()
              << ", Overflow = " << dcb_charge_ht->GetBinContent(dcb_charge_ht->GetNbinsX() + 1)
              << ", ratio = " << dcb_charge_ht->GetBinContent(dcb_charge_ht->GetNbinsX() + 1) / dcb_charge_ht->GetEntries()
              << std::endl;
    std::cout << std::endl;

    gPad->Update();
    gROOT->ProcessLine("xf");
    gPad->Update();
    //    gROOT->ProcessLine("save");
    std::cout << "is it OK? [y/n, exit] ";
    std::cin >> dummy;
    if (dummy == "n") {
        // std::cout << "please recalibrate the MUX position" << std::endl;
    }
    if (dummy == "exit")
        return;

    /// get telescope data
    gROOT->ProcessLine("cd");
    gROOT->ProcessLine("zone 2 3");
    gROOT->ProcessLine("ht 5/0 colz");
    gROOT->ProcessLine("lgz");
    gROOT->ProcessLine("ht 5/1 colz");
    gROOT->ProcessLine("lgz");
    gROOT->ProcessLine("ht 7/0 colz");
    gROOT->ProcessLine("lgz");
    gROOT->ProcessLine("ht 7/1 colz");
    gROOT->ProcessLine("lgz");
    gROOT->ProcessLine("ht 9/0 colz");
    gROOT->ProcessLine("lgz");
    gROOT->ProcessLine("ht 9/1 colz");
    gROOT->ProcessLine("lgz");
    gPad->Update();
    //    gROOT->ProcessLine("save");
    std::cout << "is it OK? [y/n, exit] ";
    std::cin >> dummy;
    if (dummy == "n") {
        std::cout << "please recalibrate the MUX position" << std::endl;
    }
    if (dummy == "exit")
        return;

    gROOT->ProcessLine("cd");
    gROOT->ProcessLine("zone 1 3");
    gROOT->ProcessLine("ht 5/2 colz");
    gROOT->ProcessLine("lgz");
    gROOT->ProcessLine("ht 7/2 colz");
    gROOT->ProcessLine("lgz");
    gROOT->ProcessLine("ht 9/2 colz");
    gROOT->ProcessLine("lgz");
    gPad->Update();
    //    gROOT->ProcessLine("save");
    std::cout << "is it OK? [y/n, exit] ";
    std::cin >> dummy;
    if (dummy == "n") {
        std::cout << "please recalibrate the MUX position" << std::endl;
    }
    if (dummy == "exit")
        return;

    gROOT->ProcessLine("cd");
    gROOT->ProcessLine("zone 2 3");
    gROOT->ProcessLine("ht 11/0 colz");
    gROOT->ProcessLine("ht 11/3 colz");
    gROOT->ProcessLine("ht 13/0 colz");
    gROOT->ProcessLine("ht 13/3 colz");
    gROOT->ProcessLine("ht 15/0 colz");
    gROOT->ProcessLine("ht 15/3 colz");
    gPad->Update();
    //    gROOT->ProcessLine("save");

    // print out the METIS
    //    std::cout << "print out the Si HV condition? [y/n, exit]";
    //    std::cin >> dummy;
    //    if(dummy == "y"){
    //        TString command;
    //        command.Form("source ./macro/printHV.sh %s%s",RUNNAME.Data(), RUNNUM.Data());
    //        system(command.Data());
    //    }
    //    if(dummy == "exit") return;

    // beam quality check

    // mwdc check

    // SSD check

    // MUX position check
    //    gROOT->ProcessLine("cd 10");
    //
    //    TH1F* hmux=(TH1F*)gDirectory->GetList()->At(0);
    //    chkmuxpos(hmux, "prm/ssd/tel4/pos_dEX/current");
    //    gPad->Update();
    //    std::cout << "is it OK? [y/n, exit] ";
    //    std::cin >> dummy;
    //    if(dummy == "n"){
    //        std::cout << "please recalibrate the MUX position" << std::endl;
    //    }
    //    if(dummy == "exit") return;
    //
    //    hmux=(TH1F*)gDirectory->GetList()->At(1);
    //    chkmuxpos(hmux, "prm/ssd/tel4/pos_dEY/current");
    //    gPad->Update();
    //    std::cout << "is it OK? [y/n, exit] ";
    //    std::cin >> dummy;
    //    if(dummy == "n"){
    //        std::cout << "please recalibrate the MUX position" << std::endl;
    //    }
    //    if(dummy == "exit") return;
    //
    //    hmux=(TH1F*)gDirectory->GetList()->At(2);
    //    chkmuxpos(hmux, "prm/ssd/tel5/pos_dEX/current");
    //    gPad->Update();
    //    std::cout << "is it OK? [y/n, exit] ";
    //    std::cin >> dummy;
    //    if(dummy == "n"){
    //        std::cout << "please recalibrate the MUX position" << std::endl;
    //    }
    //    if(dummy == "exit") return;
    //
    //    hmux=(TH1F*)gDirectory->GetList()->At(3);
    //    chkmuxpos(hmux, "prm/ssd/tel5/pos_dEY/current");
    //    gPad->Update();
    //    std::cout << "is it OK? [y/n, exit] ";
    //    std::cin >> dummy;
    //    if(dummy == "n"){
    //        std::cout << "please recalibrate the MUX position" << std::endl;
    //    }
    //    if(dummy == "exit") return;
    //
    //    hmux=(TH1F*)gDirectory->GetList()->At(4);
    //    chkmuxpos(hmux, "prm/ssd/tel6/pos_dEX/current");
    //    gPad->Update();
    //    std::cout << "is it OK? [y/n, exit] ";
    //    std::cin >> dummy;
    //    if(dummy == "n"){
    //        std::cout << "please recalibrate the MUX position" << std::endl;
    //    }
    //    if(dummy == "exit") return;
    //
    //    hmux=(TH1F*)gDirectory->GetList()->At(5);
    //    chkmuxpos(hmux, "prm/ssd/tel6/pos_dEY/current");
    //    gPad->Update();
    //    std::cout << "is it OK? [y/n, exit] ";
    //    std::cin >> dummy;
    //    if(dummy == "n"){
    //        std::cout << "please recalibrate the MUX position" << std::endl;
    //    }
    //    if(dummy == "exit") return;
}