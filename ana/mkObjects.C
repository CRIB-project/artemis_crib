void mkObjects() {
    TString art_home = "/home/okawa/art_analysis/develop/develop";
    TString art_si_home = "/home/okawa/art_analysis/si26a/okawa";
    Bool_t do_exp_hist = false;
    Bool_t do_exp_hist_with_cut = false;
    Bool_t do_window_cal_graph = false;
    Bool_t do_window_resolution_hist = false;
    Int_t max_level = 40;
    Bool_t do_apx_cal_graph = false;
    Bool_t do_apx_sim_graph = false;
    Bool_t do_apx_sim_hist = false;
    Bool_t do_a2p_sim_hist = true;

    // experimental data with low energy
    if (do_exp_hist) {
        std::cout << "making objects... experimental data" << std::endl;

        TString finname = Form("%s/raw_proton.root", art_home.Data());
        TFile *fin = TFile::Open(finname, "read");
        if (!fin || fin->IsZombie()) {
            std::cerr << "Error: opening " << finname << "!" << std::endl;
            return;
        }

        std::vector<TH2F *> hists;
        for (auto i = 0; i < 5; i++) {
            TString histname = Form("tel%d_proton", i + 1);
            TH2F *hist = dynamic_cast<TH2F *>(fin->Get(histname));
            if (!hist) {
                std::cerr << "Error: Could not find histogram " << histname << " in file " << finname << std::endl;
                fin->Close();
                return;
            }
            hist->SetName(Form("tel%d", i + 1));
            hist->SetTitle(Form("tel%d;timing (ns);energy (MeV)", i + 1));
            hists.emplace_back(hist);
        }

        TString foutname = Form("%s/ana2/hist/exp/energy_timing.root", art_home.Data());
        TFile *fout = TFile::Open(foutname, "recreate");
        if (!fout || fout->IsZombie()) {
            std::cerr << "Error: Could not open output file " << foutname << std::endl;
            fin->Close();
            return;
        }

        fout->cd();
        for (auto ht : hists) {
            ht->Write();
        }

        fout->Close();
        fin->Close();

        hists.clear();

        std::cout << "  TH2F has been saved to " << foutname << std::endl;
    }

    // exoerimental data without low energy
    if (do_exp_hist_with_cut) {
        std::cout << "making objects... experimental data with cut" << std::endl;

        TString runnames[2] = {"high", "bg"};
        // physics
        for (auto iname = 0; iname < 2; iname++) {
            TString finname = Form("%s/output/coin/si26/%s_si26_proton.root",
                                   art_si_home.Data(), runnames[iname].Data());
            TFile *fin = TFile::Open(finname, "read");
            if (!fin || fin->IsZombie()) {
                std::cerr << "Error: opening " << finname << "!" << std::endl;
                return;
            }

            TTree *tree = (TTree *)fin->Get("tree");
            if (!tree) {
                std::cerr << "Error getting TTree in " << finname << "!" << std::endl;
                fin->Close();
                return;
            }

            TString foutname = Form("%s/ana2/hist/exp/%s_proton.root",
                                    art_home.Data(), runnames[iname].Data());
            TFile *fout = new TFile(foutname, "recreate");
            if (!fout || fout->IsZombie()) {
                std::cerr << "Error creating output ROOT file at " << foutname << "!" << std::endl;
                fin->Close();
                return;
            }

            for (auto i = 0; i < 5; i++) {
                TString histname = Form("tel%d", i + 1);
                TH1F *hist = new TH1F(histname, histname, 25, 0., 25.);

                tree->Draw(Form("tel.E()>>%s", histname.Data()), Form("tel.fTelID == %d", i + 1), "goff");
                fout->cd();
                hist->Write();

                delete hist;
            }

            fout->Close();
            fin->Close();

            std::cout << "    TH1F has been saved to " << foutname << std::endl;
        }
    }

    // elastic scattering without resolution
    if (do_window_cal_graph) {
        std::cout << "making objects... elastic scattering without resolution" << std::endl;

        TString finname = Form("%s/output/sim/window/window_cal.root", art_home.Data());
        TFile *fin = TFile::Open(finname, "read");
        if (!fin || fin->IsZombie()) {
            std::cerr << "Error opening " << finname << "!" << std::endl;
            return;
        }

        TTree *tree = (TTree *)fin->Get("tree");
        if (!tree) {
            std::cerr << "Error getting TTree!" << std::endl;
            fin->Close();
            return;
        }

        TClonesArray *branch = new TClonesArray("art::crib::TTelescopeData");
        tree->SetBranchAddress("light", &branch);

        std::vector<TGraph *> graphs;
        std::vector<int> indexes;
        // five telescopes
        for (auto i = 0; i < 5; i++) {
            TGraph *gr = new TGraph();
            gr->SetTitle(Form("tel%d;timing (ns);energy (MeV)", i + 1));
            gr->SetName(Form("tel%d", i + 1));
            graphs.emplace_back(gr);
            indexes.emplace_back(0);
        }
        for (int i = 0, n = tree->GetEntriesFast(); i < n; i++) {
            tree->GetEntry(i);
            if (branch->GetEntriesFast() == 0)
                continue;
            auto *telData = (art::crib::TTelescopeData *)branch->At(0);
            double energy = telData->E();
            if (energy == 0)
                continue;
            double timing = telData->T();
            int telid = telData->GetTelID();
            if (telid == 0)
                continue;
            // int xid = telData->GetXID();
            // int yid = telData->GetYID();
            // if (xid != 7 || yid != 7)
            //     continue;
            graphs[telid - 1]->SetPoint(indexes[telid - 1], timing, energy);
            indexes[telid - 1]++;
        }

        TString foutname = Form("%s/ana2/graph/window.root", art_home.Data());
        TFile *fout = new TFile(foutname, "recreate");
        if (!fout || fout->IsZombie()) {
            std::cerr << "Error creating output ROOT file at " << foutname << "!" << std::endl;
            fin->Close();
            return;
        }

        fout->cd();
        for (auto gr : graphs) {
            gr->Write();
        }

        fout->Close();
        fin->Close();

        for (auto gr : graphs) {
            delete gr;
        }
        graphs.clear();

        std::cout << "  TGraph has been saved to " << foutname << std::endl;
    }

    // elastic scattering with resolution
    if (do_window_resolution_hist) {
        std::cout << "making objects... elastic scattering with resolution" << std::endl;

        TString finname = Form("%s/output/sim/window/window_sim.root", art_home.Data());
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

        TString foutname = Form("%s/ana2/hist/sim/window.root", art_home.Data());
        TFile *fout = new TFile(foutname, "recreate");
        if (!fout || fout->IsZombie()) {
            std::cerr << "Error creating output ROOT file at " << foutname << "!" << std::endl;
            fin->Close();
            return;
        }

        for (auto i = 0; i < 5; i++) {
            TString histname = Form("tel%d", i + 1);
            TH2F *hist = new TH2F(histname, histname, 100, -50., 50., 100, 0., 25.);

            tree->Draw(Form("light.E():light.T()>>%s", histname.Data()), Form("light.fTelID == %d", i + 1), "goff");
            fout->cd();
            hist->Write();

            delete hist;
        }

        fout->Close();
        fin->Close();

        std::cout << "  TH2F has been saved to " << foutname << std::endl;
    }

    // simulation (a, px) result without resolution
    if (do_apx_cal_graph) {
        std::cout << "making objects... (a, px) simulation without resolution" << std::endl;
        for (auto ex = 0; ex <= max_level; ex++) {
            std::cout << "  processing (a, p" << ex << ")..." << std::endl;

            TString finname = Form("%s/output/sim/ap/p%d_cal.root", art_home.Data(), ex);
            TFile *fin = TFile::Open(finname, "read");
            if (!fin || fin->IsZombie()) {
                std::cerr << "Error opening " << finname << "!" << std::endl;
                return;
            }

            TTree *tree = (TTree *)fin->Get("tree");
            if (!tree) {
                std::cerr << "Error getting TTree!" << std::endl;
                fin->Close();
                return;
            }

            TClonesArray *branch = new TClonesArray("art::crib::TTelescopeData");
            tree->SetBranchAddress("light", &branch);

            std::vector<TGraph *> graphs;
            std::vector<int> indexes;
            // five telescopes
            for (auto i = 0; i < 5; i++) {
                TGraph *gr = new TGraph();
                gr->SetTitle(Form("tel%d;timing (ns);energy (MeV)", i + 1));
                gr->SetName(Form("tel%d", i + 1));
                graphs.emplace_back(gr);
                indexes.emplace_back(0);
            }
            for (int i = 0, n = tree->GetEntriesFast(); i < n; i++) {
                tree->GetEntry(i);
                int n_entry = branch->GetEntriesFast();
                if (n_entry == 0)
                    continue;
                else if (n_entry > 1) {
                    std::cerr << "branch entries = " << n_entry << std::endl;
                    continue;
                }
                auto *telData = (art::crib::TTelescopeData *)branch->At(0);
                double energy = telData->E();
                if (energy == 0)
                    continue;
                double timing = telData->T();
                int telid = telData->GetTelID();
                if (telid == 0)
                    continue;

                // int xid = telData->GetXID();
                // int yid = telData->GetYID();
                // if ((xid != 7 && xid != 8) || (yid != 7 && yid != 8))
                //     continue;
                graphs[telid - 1]->SetPoint(indexes[telid - 1], timing, energy);
                indexes[telid - 1]++;
            }

            TString foutname = Form("%s/ana2/graph/ap%d.root", art_home.Data(), ex);
            TFile *fout = new TFile(foutname, "recreate");
            if (!fout || fout->IsZombie()) {
                std::cerr << "Error creating output ROOT file at " << foutname << "!" << std::endl;
                fin->Close();
                return;
            }

            fout->cd();
            for (auto gr : graphs) {
                gr->Write();
            }

            fout->Close();
            fin->Close();

            for (auto gr : graphs) {
                delete gr;
            }
            graphs.clear();
            indexes.clear();

            std::cout << "    TGraph has been saved to " << foutname << std::endl;
        }
    }

    if (do_apx_sim_graph) {
        std::vector max_loop{
            369825,
            546332,
            620854,
            487708,
            527047,
            505060,
            426753,
            375070,
            380118,
            214014,
            368450,
            320237,
            327410,
            182265,
            285197,
            308782,
            313237,
            266271,
            253947,
            236735,
            143230,
            240979,
            126523,
            153468,
            105208,
            194836,
            154592,
            163905,
            149799,
            128257,
            126312,
            67785,
            120589,
            51026,
            54923,
            91304,
            72198,
            31023,
            70637,
            66889,
            29826};

        std::cout << "making objects... (a, px) simulation with resolution" << std::endl;
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
                std::cerr << "Error getting TTree!" << std::endl;
                fin->Close();
                return;
            }

            TClonesArray *branch = new TClonesArray("art::crib::TTelescopeData");
            tree->SetBranchAddress("light", &branch);

            std::vector<TGraph *> graphs;
            std::vector<int> indexes;
            // five telescopes
            for (auto i = 0; i < 5; i++) {
                TGraph *gr = new TGraph();
                gr->SetTitle(Form("tel%d;timing (ns);energy (MeV)", i + 1));
                gr->SetName(Form("tel%d", i + 1));
                graphs.emplace_back(gr);
                indexes.emplace_back(0);
            }
            for (int i = 0, n = max_loop[ex]; i < n; i++) {
                tree->GetEntry(i);
                int n_entry = branch->GetEntriesFast();
                if (n_entry == 0)
                    continue;
                else if (n_entry > 1) {
                    std::cerr << "branch entries = " << n_entry << std::endl;
                    continue;
                }
                auto *telData = (art::crib::TTelescopeData *)branch->At(0);
                double energy = telData->E();
                if (energy == 0)
                    continue;
                double timing = telData->T();
                int telid = telData->GetTelID();
                if (telid == 0)
                    continue;

                // int xid = telData->GetXID();
                // int yid = telData->GetYID();
                // if ((xid != 7 && xid != 8) || (yid != 7 && yid != 8))
                //     continue;
                graphs[telid - 1]->SetPoint(indexes[telid - 1], timing, energy);
                indexes[telid - 1]++;
            }

            TString foutname = Form("%s/ana2/graph/real/ap%d.root", art_home.Data(), ex);
            TFile *fout = new TFile(foutname, "recreate");
            if (!fout || fout->IsZombie()) {
                std::cerr << "Error creating output ROOT file at " << foutname << "!" << std::endl;
                fin->Close();
                return;
            }

            fout->cd();
            for (auto gr : graphs) {
                gr->Write();
            }

            fout->Close();
            fin->Close();

            for (auto gr : graphs) {
                delete gr;
            }
            graphs.clear();
            indexes.clear();

            std::cout << "    TGraph has been saved to " << foutname << std::endl;
        }
    }

    if (do_apx_sim_hist) {
        std::cout << "making objects... (a, px) simulation" << std::endl;
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

            TString foutname = Form("%s/ana2/hist/sim/ap%d.root", art_home.Data(), ex);
            TFile *fout = new TFile(foutname, "recreate");
            if (!fout || fout->IsZombie()) {
                std::cerr << "Error creating output ROOT file at " << foutname << "!" << std::endl;
                fin->Close();
                return;
            }

            for (auto i = 0; i < 5; i++) {
                TString histname = Form("tel%d", i + 1);
                TH2F *hist = new TH2F(histname, histname, 100, -50., 50., 100, 0., 25.);

                tree->Draw(Form("light.E():light.T()>>%s", histname.Data()), Form("light.fTelID == %d", i + 1), "goff");
                fout->cd();
                hist->Write();

                delete hist;
            }

            fout->Close();
            fin->Close();

            std::cout << "    TH2F has been saved to " << foutname << std::endl;
        }
    }

    if (do_a2p_sim_hist) {
        std::cout << "making objects... (a, 2p) simulation data" << std::endl;

        TString finname = Form("%s/output/sim/a2p/si26a2p.root", art_home.Data());
        TFile *fin = TFile::Open(finname, "read");
        if (!fin || fin->IsZombie()) {
            std::cerr << "Error: opening " << finname << "!" << std::endl;
            return;
        }

        TTree *tree = (TTree *)fin->Get("tree");
        if (!tree) {
            std::cerr << "Error getting TTree in " << finname << "!" << std::endl;
            fin->Close();
            return;
        }

        TString foutname = Form("%s/ana2/hist/a2p/ground.root", art_home.Data());
        TFile *fout = new TFile(foutname, "recreate");
        if (!fout || fout->IsZombie()) {
            std::cerr << "Error creating output ROOT file at " << foutname << "!" << std::endl;
            fin->Close();
            return;
        }

        for (auto i = 0; i < 5; i++) {
            TString histname = Form("tel%d", i + 1);
            TH1F *hist = new TH1F(histname, histname, 100, 0., 25.);

            tree->Draw(Form("light1.E()>>%s", histname.Data()), Form("light1.fTelID == %d", i + 1), "goff");
            fout->cd();
            hist->Write();

            delete hist;
        }

        fout->Close();
        fin->Close();

        std::cout << "    TH1F has been saved to " << foutname << std::endl;
    }
}
