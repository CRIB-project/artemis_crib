void chka2p_events(const char *inputFileName = "/home/okawa/art_analysis/si26a/okawa/output/coin/si26/bg_si26_all.root",
                   const char *outputFileName = "/home/okawa/art_analysis/develop/develop/ana2/rootfile/bg_coin.root") {
    // 入力ファイルを開く
    TFile *inputFile = TFile::Open(inputFileName, "read");
    if (!inputFile || inputFile->IsZombie()) {
        std::cerr << "Error: Could not open input file " << inputFileName << std::endl;
        return;
    }

    // TTreeオブジェクトを取得
    TTree *inputTree = (TTree *)inputFile->Get("tree");
    if (!inputTree) {
        std::cerr << "Error: Could not find TTree tree in file " << inputFileName << std::endl;
        inputFile->Close();
        return;
    }

    // 必要な変数の設定
    TClonesArray *tel1 = new TClonesArray("art::crib::TTelescopeData");
    TClonesArray *tel2 = new TClonesArray("art::crib::TTelescopeData");
    TClonesArray *tel3 = new TClonesArray("art::crib::TTelescopeData");
    TClonesArray *tel4 = new TClonesArray("art::crib::TTelescopeData");
    TClonesArray *tel5 = new TClonesArray("art::crib::TTelescopeData");
    inputTree->SetBranchAddress("tel1", &tel1);
    inputTree->SetBranchAddress("tel2", &tel2);
    inputTree->SetBranchAddress("tel3", &tel3);
    inputTree->SetBranchAddress("tel4", &tel4);
    inputTree->SetBranchAddress("tel5", &tel5);

    // 出力ファイルを開く
    TFile *outputFile = new TFile(outputFileName, "recreate");

    // 新しいTTreeを作成
    TTree *outputTree = new TTree("tree", "tree");

    // 新しいブランチの変数を定義
    TClonesArray *array = new TClonesArray("art::crib::TTelescopeData", 2);
    outputTree->Branch("protons", &array);

    // フィルタ条件を設定してエントリを選択
    int index = 0;
    Long64_t nEntries = inputTree->GetEntries();
    for (Long64_t i = 0; i < nEntries; i++) {
        if (i % (nEntries / 100) == 0) { // 1%ごとに更新
            int pos = (i * 50) / nEntries;
            std::cout << "\r[";

            for (int j = 0; j < 50; ++j) {
                if (j < pos)
                    std::cout << "=";
                else if (j == pos)
                    std::cout << ">";
                else
                    std::cout << " ";
            }
            std::cout << "] " << int((i * 100.0) / nEntries) << "% " << std::flush;
        }

        inputTree->GetEntry(i);

        // TClonesArrayをクリア
        array->Clear();

        int n_entries_tel1 = tel1->GetEntriesFast();
        int n_entries_tel2 = tel2->GetEntriesFast();
        int n_entries_tel3 = tel3->GetEntriesFast();
        int n_entries_tel4 = tel4->GetEntriesFast();
        int n_entries_tel5 = tel5->GetEntriesFast();

        int n_total_tel = n_entries_tel1 + n_entries_tel2 + n_entries_tel3 + n_entries_tel4 + n_entries_tel5;
        if (n_total_tel <= 1) {
            continue;
        }

        // get telescope objects
        bool isfirst = true;
        art::crib::TTelescopeData *firstData = nullptr;
        art::crib::TTelescopeData *secondData = nullptr;
        if (isfirst && n_entries_tel1 == 1) {
            firstData = (art::crib::TTelescopeData *)tel1->At(0);

            // check if proton or not
            double e1 = firstData->E();
            double e1_1 = firstData->E(1);
            double e1_2 = firstData->E(2);
            double t1 = firstData->T(2);
            bool is_tel1_proton = (e1_1 > 0.01) && (abs(sqrt(e1_1 * e1) - 5.3) < 0.5 && abs(t1 + 172.5) < 27.5 && e1_2 > 0.1);
            if (!is_tel1_proton) {
                continue;
            }
            isfirst = false;
        }

        if (isfirst && n_entries_tel2 == 1) {
            firstData = (art::crib::TTelescopeData *)tel2->At(0);

            // check if proton or not
            double e2 = firstData->E();
            double e2_1 = firstData->E(1);
            double e2_2 = firstData->E(2);
            double t2 = firstData->T(2);
            bool is_tel2_proton = (e2_1 > 0.01) && (abs(sqrt(e2_1 * e2) - 5.45) < 0.55 && abs(t2 + 170.0) < 50.0 && e2_2 > 0.1);
            if (!is_tel2_proton) {
                continue;
            }
            isfirst = false;
        } else if (!isfirst && n_entries_tel2 == 1) {
            secondData = (art::crib::TTelescopeData *)tel2->At(0);

            // check if proton or not
            double e2 = secondData->E();
            double e2_1 = secondData->E(1);
            double e2_2 = secondData->E(2);
            double t2 = secondData->T(2);
            bool is_tel2_proton = (e2_1 > 0.01) && (abs(sqrt(e2_1 * e2) - 5.45) < 0.55 && abs(t2 + 170.0) < 50.0 && e2_2 > 0.1);
            if (!is_tel2_proton) {
                continue;
            }
        }

        if (isfirst && n_entries_tel3 == 1) {
            firstData = (art::crib::TTelescopeData *)tel3->At(0);

            // check if proton or not
            double e3 = firstData->E();
            double e3_1 = firstData->E(1);
            double e3_2 = firstData->E(2);
            double t3 = firstData->T(2);
            bool is_tel3_proton = (e3_1 > 0.01) && (abs(sqrt(e3_1 * e3) - 7.7) < 1.0 && abs(t3 + 190.0) < 50.0 && e3_2 > 0.1);
            if (!is_tel3_proton) {
                continue;
            }
            isfirst = false;
        } else if (!isfirst && n_entries_tel3 == 1) {
            secondData = (art::crib::TTelescopeData *)tel3->At(0);

            // check if proton or not
            double e3 = secondData->E();
            double e3_1 = secondData->E(1);
            double e3_2 = secondData->E(2);
            double t3 = secondData->T(2);
            bool is_tel3_proton = (e3_1 > 0.01) && (abs(sqrt(e3_1 * e3) - 7.7) < 1.0 && abs(t3 + 190.0) < 50.0 && e3_2 > 0.1);
            if (!is_tel3_proton) {
                continue;
            }
        }

        if (isfirst && n_entries_tel4 == 1) {
            firstData = (art::crib::TTelescopeData *)tel4->At(0);

            // check if proton or not
            double e4 = firstData->E();
            double e4_0 = firstData->E(0);
            double e4_1 = firstData->E(1);
            double t4 = firstData->T(1);
            bool is_tel4_proton = (e4_1 > 0.01) && ((abs(sqrt(e4_0 * e4) - 2.5) < 1.0) || (e4_0 < 0.1 && e4 > 3.0)) && (abs(t4 + 190.0) < 30.0);
            if (!is_tel4_proton) {
                continue;
            }

            isfirst = false;
        } else if (!isfirst && n_entries_tel4 == 1) {
            secondData = (art::crib::TTelescopeData *)tel4->At(0);

            // check if proton or not
            double e4 = secondData->E();
            double e4_0 = secondData->E(0);
            double e4_1 = secondData->E(1);
            double t4 = secondData->T(1);
            bool is_tel4_proton = (e4_1 > 0.01) && ((abs(sqrt(e4_0 * e4) - 2.5) < 1.0) || (e4_0 < 0.1 && e4 > 3.0)) && (abs(t4 + 190.0) < 30.0);
            if (!is_tel4_proton) {
                continue;
            }
        }

        if (!isfirst && n_entries_tel5 == 1) {
            secondData = (art::crib::TTelescopeData *)tel5->At(0);

            // check if proton or not
            double e5 = secondData->E();
            double e5_0 = secondData->E(0);
            double e5_1 = secondData->E(1);
            double t5 = secondData->T(1);
            bool is_tel5_proton = (e5_1 > 0.01) && ((abs(sqrt(e5_0 * e5) - 2.9) < 0.7) || (e5_0 < 0.1 && e5 > 3.0)) && (abs(t5 + 190.0) < 30.0);
            if (!is_tel5_proton) {
                continue;
            }
        }

        if (!firstData && !secondData) {
            std::cerr << "could not get coin data" << std::endl;
        }

        index++;
        std::cout << "coin hit: " << index << std::endl;

        new ((*array)[0]) art::crib::TTelescopeData(*firstData);
        new ((*array)[1]) art::crib::TTelescopeData(*secondData);

        // 新しいデータを出力TTreeに追加
        outputTree->Fill();
    }

    // 出力ファイルに保存
    outputTree->Write();
    outputFile->Close();
    inputFile->Close();

    // メモリの解放
    delete array;
}
