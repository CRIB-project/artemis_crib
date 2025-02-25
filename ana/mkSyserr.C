void ProcessROOTFilesAndCreate2DHistograms(int numFiles, const std::vector<int> &nEntry) {
    // 二次元ヒストグラムの作成（2つ）
    TH2F *hist1 = new TH2F("Ecm", "Ecm", 100, 0.0, 8.0, 100, -3.0, 3.0);                 // 100x100ビン、範囲(0-100, 0-100)
    TH2F *hist2 = new TH2F("Position", "Position", 100, -50.0, 400.0, 100, -50.0, 50.0); // 50x50ビン、範囲(-50-50, -50-50)

    for (int i = 0; i < numFiles; i++) {
        std::string fileName = "ana2/syserror/p" + std::to_string(i) + ".root";
        TFile *inputFile = TFile::Open(fileName.c_str());
        std::cout << "processing... " << fileName << std::endl;
        if (inputFile && !inputFile->IsZombie()) {
            TTree *tree = (TTree *)inputFile->Get("tree"); // 実際のTTreeの名前に置き換え
            if (tree) {
                int entries = std::min(nEntry[i], (int)tree->GetEntries()); // nEntry[i]の数だけエントリーを取得
                // 各エントリーに対して指定された変数を取得し、ヒストグラムに追加
                TClonesArray *reaction = new TClonesArray("art::crib::TReactionInfo");
                TClonesArray *reconst = new TClonesArray("art::crib::TReactionInfo");

                tree->SetBranchAddress("reaction", &reaction);
                tree->SetBranchAddress("reconst", &reconst);

                for (int j = 0; j < entries; ++j) {
                    tree->GetEntry(j);
                    int n_reconst = reconst->GetEntriesFast();
                    if (n_reconst == 0)
                        continue;
                    auto *reactionData = (art::crib::TReactionInfo *)reaction->At(0);
                    auto *reconstData = (art::crib::TReactionInfo *)reconst->At(0);

                    double ecm = reconstData->GetEnergy();
                    double pos = reconstData->GetZ();
                    double ecm_error = reactionData->GetEnergy() - ecm;
                    double pos_error = reactionData->GetZ() - pos;

                    hist1->Fill(ecm, ecm_error); // ヒストグラム1にエントリーを追加
                    hist2->Fill(pos, pos_error); // ヒストグラム2にエントリーを追加
                }
            } else {
                std::cerr << "Error: TTree not found in " << fileName << std::endl;
            }
            inputFile->Close();
        } else {
            std::cerr << "Error: Unable to open file " << fileName << std::endl;
        }
    }

    // 新しい二次元ヒストグラムを保存するためのファイル作成
    TFile *outputFile = new TFile("ana2/hist/syserror/syserror.root", "recreate");
    hist1->Write(); // 二次元ヒストグラム1を書き込み
    hist2->Write(); // 二次元ヒストグラム2を書き込み
    outputFile->Close();

    // メモリの解放
    delete hist1;
    delete hist2;
}

void mkSyserr() {
    int numFiles = 41;
    std::vector<int> nEntry = {
        2844811,
        4202560,
        4775803,
        3751604,
        4054210,
        3885084,
        3282722,
        2885158,
        2923986,
        1646264,
        2834237,
        2463361,
        2518545,
        1402040,
        2193825,
        2375246,
        2409519,
        2048240,
        1953438,
        1821041,
        1101771,
        1853686,
        973254,
        1180530,
        809299,
        1498739,
        1189171,
        1260809,
        1152307,
        986594,
        971634,
        521424,
        927612,
        392512,
        422487,
        702338,
        555372,
        238644,
        543368,
        514535,
        229437};

    ProcessROOTFilesAndCreate2DHistograms(numFiles, nEntry);
}
