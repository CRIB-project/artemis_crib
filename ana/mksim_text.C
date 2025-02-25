void mksim_text() {
    TString art_home = "/home/okawa/art_analysis/develop/develop";
    TString output_dir = "/home/okawa/art_analysis/develop/develop/ana2/yield/sim";
    Int_t max_level = 40;
    for (auto ex = 0; ex <= max_level; ex++) {
        std::cout << "  processing (a, p" << ex << ")..." << std::endl;

        TString finname = Form("%s/output/sim/ap/p%d.root", art_home.Data(), ex);
        TFile *fin = TFile::Open(finname, "read");
        if (!fin || fin->IsZombie()) {
            std::cerr << "Error opening " << finname << "!" << std::endl;
            return;
        }

        TTree *tree = (TTree *)fin->Get("tree");
        if (!tree) {
            std::cerr << "Error getting TTree in " << finname << "!" << std::endl;
            fin->Close();
            return;
        }

        TString outputFileName = Form("%s/ap_%02d.txt", output_dir.Data(), ex);
        std::ofstream out(outputFileName.Data());
        if (!out.is_open()) {
            std::cerr << "Failed to open output file: " << outputFileName << std::endl;
            fin->Close();
            continue;
        }

        TClonesArray *branch = new TClonesArray("art::crib::TReactionInfo");
        TClonesArray *branch_tel = new TClonesArray("art::crib::TTelescopeData");
        tree->SetBranchAddress("reaction", &branch);
        tree->SetBranchAddress("light", &branch_tel);

        // TTreeのエントリ数を取得
        Long64_t nEntries = tree->GetEntries();
        int index = 0;
        for (Long64_t i = 0; i < nEntries; ++i) {
            tree->GetEntry(i);
            if (branch_tel->GetEntriesFast() == 0)
                continue;
            auto *reconstData = (art::crib::TReactionInfo *)branch->At(0);
            auto *telData = (art::crib::TTelescopeData *)branch_tel->At(0);
            double energy = reconstData->GetEnergy();
            double excited = reconstData->GetExEnergy();
            double energy_l = telData->E();
            if (energy_l < 5.0)
                continue;

            // データをテキストファイルに書き出し
            out << index << ", " << energy << ", " << excited << ", " << energy_l << std::endl;
            index++;
        }

        out.close();
        fin->Close();
    }
}
