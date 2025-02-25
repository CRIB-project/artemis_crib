std::vector<TString> GetRootFilesInDirectory(const char *dir) {
    void *dirp = gSystem->OpenDirectory(dir);
    const char *entry;
    std::vector<TString> rootFiles;

    while ((entry = gSystem->GetDirEntry(dirp)) != nullptr) {
        TString filename(entry);
        if (filename.EndsWith(".root")) {
            rootFiles.push_back(TString(dir) + "/" + filename);
        }
    }
    gSystem->FreeDirectory(dirp);
    return rootFiles;
}

TString GetOutputFilePathInParentDir(const TString &filePath) {
    TString dirName = gSystem->DirName(filePath);   // ファイルのディレクトリパス
    TString parentDir = gSystem->DirName(dirName);  // 親ディレクトリのパス
    TString baseName = gSystem->BaseName(filePath); // ファイル名 (例: data.root)
    baseName.ReplaceAll(".root", ".txt");           // 拡張子を .txt に変更

    // 親ディレクトリ + 新しいテキストファイル名
    return parentDir + "/" + baseName;
}

void mkyield_text() {
    const char *directory = "/home/okawa/art_analysis/develop/develop/ana2/yield/40/raw";
    // rootファイル一覧を取得
    std::vector<TString> rootFiles = GetRootFilesInDirectory(directory);

    // rootファイルを一つずつ処理
    for (const auto &fileName : rootFiles) {
        TFile *file = TFile::Open(fileName);
        if (!file || file->IsZombie()) {
            std::cerr << "Failed to open file: " << fileName << std::endl;
            continue;
        }

        // TTreeオブジェクトを取得（"tree"という名前のTTreeを想定）
        TTree *tree = nullptr;
        file->GetObject("tree", tree);
        if (!tree) {
            std::cerr << "TTree 'tree' not found in file: " << fileName << std::endl;
            file->Close();
            continue;
        }

        // 出力するテキストファイル名を生成（拡張子を除いたROOTファイル名.txtを親ディレクトリに作成）
        TString outputFileName = GetOutputFilePathInParentDir(fileName);

        std::ofstream out(outputFileName.Data());
        if (!out.is_open()) {
            std::cerr << "Failed to open output file: " << outputFileName << std::endl;
            file->Close();
            continue;
        }

        TClonesArray *branch = new TClonesArray("art::crib::TReactionInfo");
        TClonesArray *branch_tel = new TClonesArray("art::crib::TTelescopeData");
        tree->SetBranchAddress("reconst", &branch);
        tree->SetBranchAddress("telescope", &branch_tel);

        // TTreeのエントリ数を取得
        Long64_t nEntries = tree->GetEntries();
        int index = 0;
        for (Long64_t i = 0; i < nEntries; ++i) {
            tree->GetEntry(i);
            if (branch->GetEntriesFast() == 0)
                continue;
            auto *reconstData = (art::crib::TReactionInfo *)branch->At(0);
            auto *telData = (art::crib::TTelescopeData *)branch_tel->At(0);
            double energy = reconstData->GetEnergy();
            double excited = reconstData->GetExEnergy();
            double energy_l = telData->E();

            // データをテキストファイルに書き出し
            out << index << ", " << energy << ", " << excited << ", " << energy_l << std::endl;
            index++;
        }

        // ファイルを閉じる
        out.close();
        file->Close();
    }
}
