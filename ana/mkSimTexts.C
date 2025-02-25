void mkSimTexts() {
    // Windowの散乱のデータ
    TString windowFileName = "/home/okawa/art_analysis/develop/develop/output/sim/window/window_cal.root";
    TString windowFileBase = "/home/okawa/art_analysis/develop/develop/ana2/raw/window";

    // apxの散乱データ
    std::vector<TString> apxFileNames = {};
    TString apxFileBase = "/home/okawa/art_analysis/develop/develop/output/sim/ap";
    for (auto i = 0; i < 41; i++) {
        apxFileNames.emplace_back(Form("%s/p%d_cal.root", apxFileBase.Data(), i));
    }
    TString apxOutFileBase = "/home/okawa/art_analysis/develop/develop/ana2/raw/apx";

    // Windowの理論曲線のテキストファイルを作る
    {
        // 入力ファイルを開く
        TFile *inputFile = TFile::Open(windowFileName, "read");
        if (!inputFile || inputFile->IsZombie()) {
            std::cerr << "Error: Could not open input file " << windowFileName << std::endl;
            return;
        }

        // TTreeオプジェクトを取得
        TTree *inputTree = (TTree *)inputFile->Get("tree");
        if (!inputTree) {
            std::cerr << "Error: Could not find TTree tree in file " << windowFileName << std::endl;
            inputFile->Close();
            return;
        }

        TClonesArray *branch = new TClonesArray("art::crib::TTelescope");
        inputTree->SetBranchAddress("light", &branch);

        // 各telescopeごとに出力ファイルを作成
        std::ofstream outTel1File(Form("%s/tel1.txt", windowFileBase.Data()));
        std::ofstream outTel2File(Form("%s/tel2.txt", windowFileBase.Data()));
        std::ofstream outTel3File(Form("%s/tel3.txt", windowFileBase.Data()));
        std::ofstream outTel4File(Form("%s/tel4.txt", windowFileBase.Data()));
        std::ofstream outTel5File(Form("%s/tel5.txt", windowFileBase.Data()));

        if (!outTel1File.is_open() ||
            !outTel2File.is_open() ||
            !outTel3File.is_open() ||
            !outTel4File.is_open() ||
            !outTel5File.is_open()) {
            std::cerr << "Error opening output files." << std::endl;
            inputFile->Close();
            return;
        }

        // イベントループ
        std::vector<int> indexes = {0, 0, 0, 0, 0};
        long long nEntries = inputTree->GetEntries();
        std::cout << "Processing " << windowFileName << ", total entires = " << nEntries << std::endl;
        for (long long i = 0; i < nEntries; i++) {
            // プログレスバーの表示
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
            if (i == nEntries - 1) {
                std::cout << std::endl;
            }

            inputTree->GetEntry(i);
            if (branch->GetEntriesFast() == 0)
                continue;

            auto *telData = (art::crib::TTelescopeData *)branch->At(0);
            double energy = telData->E();
            if (energy < 0.01)
                continue;
            double timing = telData->T();
            int telid = telData->GetTelID();
            if (telid == 0)
                continue;

            // int xid = telData->GetXID();
            // int yid = telData->GetYID();
            // if (xid != 7 || yid != 7)
            //     continue;

            if (telid == 1) {
                outTel1File << indexes[0] << " " << energy << " " << timing << "\n";
                indexes[0]++;
            } else if (telid == 2) {
                outTel2File << indexes[1] << " " << energy << " " << timing << "\n";
                indexes[1]++;
            } else if (telid == 3) {
                outTel3File << indexes[2] << " " << energy << " " << timing << "\n";
                indexes[2]++;
            } else if (telid == 4) {
                outTel4File << indexes[3] << " " << energy << " " << timing << "\n";
                indexes[3]++;
            } else if (telid == 5) {
                outTel5File << indexes[4] << " " << energy << " " << timing << "\n";
                indexes[4]++;
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

    // apxの理論曲線のテキストファイルを作る
    {
        for (auto i = 0; i < 41; i++) {
            TFile *inputFile = TFile::Open(apxFileNames[i], "read");
            if (!inputFile || inputFile->IsZombie()) {
                std::cerr << "Error: Could not open input file " << apxFileNames[i] << std::endl;
                return;
            }

            TTree *inputTree = (TTree *)inputFile->Get("tree");
            if (!inputTree) {
                std::cerr << "Error: Could not find TTree tree in file " << apxFileNames[i] << std::endl;
                inputFile->Close();
                return;
            }

            TClonesArray *branch = new TClonesArray("art::crib::TTelescopeData");
            inputTree->SetBranchAddress("light", &branch);

            std::ofstream outTel1File(Form("%s/tel1/ap%d.txt", apxOutFileBase.Data(), i));
            std::ofstream outTel2File(Form("%s/tel2/ap%d.txt", apxOutFileBase.Data(), i));
            std::ofstream outTel3File(Form("%s/tel3/ap%d.txt", apxOutFileBase.Data(), i));
            std::ofstream outTel4File(Form("%s/tel4/ap%d.txt", apxOutFileBase.Data(), i));
            std::ofstream outTel5File(Form("%s/tel5/ap%d.txt", apxOutFileBase.Data(), i));

            if (!outTel1File.is_open() ||
                !outTel2File.is_open() ||
                !outTel3File.is_open() ||
                !outTel4File.is_open() ||
                !outTel5File.is_open()) {
                std::cerr << "Error opening output files." << std::endl;
                inputFile->Close();
                return;
            }

            std::vector<int> indexes = {0, 0, 0, 0, 0};
            long long nEntries = inputTree->GetEntries();
            std::cout << "Processing " << apxFileNames[i] << ", total entries = " << nEntries << std::endl;
            for (long long j = 0; j < nEntries; j++) {
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
                if (branch->GetEntriesFast() == 0)
                    continue;

                auto *telData = (art::crib::TTelescopeData *)branch->At(0);
                double energy = telData->E();
                if (energy < 0.01)
                    continue;
                double timing = telData->T();
                int telid = telData->GetTelID();
                if (telid == 0)
                    continue;

                // int xid = telData->GetXID();
                // int yid = telData->GetYID();
                // if (xid != 7 || yid != 7)
                //     continue;

                if (telid == 1) {
                    outTel1File << indexes[0] << " " << energy << " " << timing << "\n";
                    indexes[0]++;
                } else if (telid == 2) {
                    outTel2File << indexes[1] << " " << energy << " " << timing << "\n";
                    indexes[1]++;
                } else if (telid == 3) {
                    outTel3File << indexes[2] << " " << energy << " " << timing << "\n";
                    indexes[2]++;
                } else if (telid == 4) {
                    outTel4File << indexes[3] << " " << energy << " " << timing << "\n";
                    indexes[3]++;
                } else if (telid == 5) {
                    outTel5File << indexes[4] << " " << energy << " " << timing << "\n";
                    indexes[4]++;
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
}
