void mkRawTexts() {
    // 出入力するデータファイルの名前の定義
    std::vector<TString> inputFileNames = {
        "/home/okawa/art_analysis/si26a/okawa/output/coin/si26/high_si26_all.root",
        "/home/okawa/art_analysis/si26a/okawa/output/coin/si26/bg_si26_all.root",
    };

    std::vector<TString> outputFileBases = {
        "/home/okawa/art_analysis/develop/develop/ana2/raw/phys",
        "/home/okawa/art_analysis/develop/develop/ana2/raw/bg",
    };

    // 陽子イベントのtreeをテキストファイルに書き出す
    for (auto i = 0; i < 2; i++) {
        // 入力ファイルを開く
        TFile *inputFile = TFile::Open(inputFileNames[i], "read");
        if (!inputFile || inputFile->IsZombie()) {
            std::cerr << "Error: Could not open input file " << inputFileNames[i] << std::endl;
            return;
        }

        // TTreeオブジェクトを取得
        TTree *inputTree = (TTree *)inputFile->Get("tree");
        if (!inputTree) {
            std::cerr << "Error: Could not find TTree tree in file " << inputFileNames[i] << std::endl;
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

        // 各ブランチごとに出力ファイルを開く
        std::ofstream outTel1File(Form("%s/tel1_data.txt", outputFileBases[i].Data()));
        std::ofstream outTel2File(Form("%s/tel2_data.txt", outputFileBases[i].Data()));
        std::ofstream outTel3File(Form("%s/tel3_data.txt", outputFileBases[i].Data()));
        std::ofstream outTel4File(Form("%s/tel4_data.txt", outputFileBases[i].Data()));
        std::ofstream outTel5File(Form("%s/tel5_data.txt", outputFileBases[i].Data()));

        if (!outTel1File.is_open() ||
            !outTel2File.is_open() ||
            !outTel3File.is_open() ||
            !outTel4File.is_open() ||
            !outTel5File.is_open()) {
            std::cerr << "Error opening output files." << std::endl;
            inputFile->Close();
            return;
        }

        // フィルタ条件を設定してエントリを選択
        std::vector<int> indexes = {0, 0, 0, 0, 0};
        Long64_t nEntries = inputTree->GetEntries();
        std::cout << "Processing " << inputFileNames[i] << ", total entries = " << nEntries << std::endl;
        for (Long64_t j = 0; j < nEntries; j++) {
            // プログレスバーの表示
            if (j % (nEntries / 100) == 0) { // 1%ごとに更新
                int pos = (j * 50) / nEntries;
                std::cout << "\r[";

                for (int k = 0; k < 50; ++k) {
                    if (k < pos)
                        std::cout << "=";
                    else if (k == pos)
                        std::cout << ">";
                    else
                        std::cout << " ";
                }
                std::cout << "] " << int((j * 100.0) / nEntries) << "% " << std::flush;
            }
            if (j == nEntries - 1) {
                std::cout << std::endl;
            }

            inputTree->GetEntry(j);

            int n_entries_tel1 = tel1->GetEntriesFast();
            int n_entries_tel2 = tel2->GetEntriesFast();
            int n_entries_tel3 = tel3->GetEntriesFast();
            int n_entries_tel4 = tel4->GetEntriesFast();
            int n_entries_tel5 = tel5->GetEntriesFast();

            // tel1のprotonの処理
            if (n_entries_tel1 > 0) {
                auto *inData = (art::crib::TTelescopeData *)tel1->At(0);

                // check if proton or not
                double e1 = inData->E();
                double e1_1 = inData->E(1);
                double e1_2 = inData->E(2);
                double t1 = inData->T(2);
                bool is_tel1_proton = (e1_1 > 0.01) && (abs(sqrt(e1_1 * e1) - 5.3) < 0.5 && abs(t1 + 172.5) < 27.5 && e1_2 > 0.1);
                if (is_tel1_proton) {
                    outTel1File << indexes[0] << " " << e1 << " " << t1 << "\n";
                    indexes[0]++;
                }
            }

            // tel2のprotonの処理
            if (n_entries_tel2 > 0) {
                auto *inData = (art::crib::TTelescopeData *)tel2->At(0);

                // check if proton or not
                double e2 = inData->E();
                double e2_1 = inData->E(1);
                double e2_2 = inData->E(2);
                double t2 = inData->T(2);
                bool is_tel2_proton = (e2_1 > 0.01) && (abs(sqrt(e2_1 * e2) - 5.45) < 0.55 && abs(t2 + 170.0) < 50.0 && e2_2 > 0.1);
                if (is_tel2_proton) {
                    outTel2File << indexes[1] << " " << e2 << " " << t2 << "\n";
                    indexes[1]++;
                }
            }

            // tel3のprotonの処理
            if (n_entries_tel3 > 0) {
                auto *inData = (art::crib::TTelescopeData *)tel3->At(0);

                // check if proton or not
                double e3 = inData->E();
                double e3_1 = inData->E(1);
                double e3_2 = inData->E(2);
                double t3 = inData->T(2);
                bool is_tel3_proton = (e3_1 > 0.01) && (abs(sqrt(e3_1 * e3) - 7.7) < 1.0 && abs(t3 + 190.0) < 50.0 && e3_2 > 0.1);
                if (is_tel3_proton) {
                    outTel3File << indexes[2] << " " << e3 << " " << t3 << "\n";
                    indexes[2]++;
                }
            }

            // tel4のprotonの処理
            if (n_entries_tel4 > 0) {
                auto *inData = (art::crib::TTelescopeData *)tel4->At(0);

                // check if proton or not
                double e4 = inData->E();
                double e4_0 = inData->E(0);
                double e4_1 = inData->E(1);
                double t4 = inData->T(1);
                bool is_tel4_proton = (e4_1 > 0.01) && ((abs(sqrt(e4_0 * e4) - 2.5) < 1.0) || (e4_0 < 0.1 && e4 > 3.0)) && (abs(t4 + 190.0) < 30.0);
                if (is_tel4_proton) {
                    outTel4File << indexes[3] << " " << e4 << " " << t4 << "\n";
                    indexes[3]++;
                }
            }

            // tel5のprotonの処理
            if (n_entries_tel5 > 0) {
                auto *inData = (art::crib::TTelescopeData *)tel5->At(0);

                // check if proton or not
                double e5 = inData->E();
                double e5_0 = inData->E(0);
                double e5_1 = inData->E(1);
                double t5 = inData->T(1);
                bool is_tel5_proton = (e5_1 > 0.01) && ((abs(sqrt(e5_0 * e5) - 2.9) < 0.7) || (e5_0 < 0.1 && e5 > 3.0)) && (abs(t5 + 190.0) < 30.0);
                if (is_tel5_proton) {
                    outTel5File << indexes[4] << " " << e5 << " " << t5 << "\n";
                    indexes[4]++;
                }
            }
        }

        // ファイルを閉じる
        outTel1File.close();
        outTel2File.close();
        outTel3File.close();
        outTel4File.close();
        outTel5File.close();
        inputFile->Close();
    }
}
