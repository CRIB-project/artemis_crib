void chkmuxpos(TH1F *h, TString filepath) {
    Double_t pos_low[16];
    Double_t pos_high[16];

    std::ifstream fin(filepath.Data());
    if (!fin) {
        std::cerr << "Error: there is no file: " << filepath << std::endl;
        std::exit(1);
    }
    std::string str;
    Int_t itr = 0;
    while (std::getline(fin, str)) {
        if (str[0] == '#')
            continue;
        std::istringstream s(str);
        s >> pos_low[itr] >> pos_high[itr];
        itr++;
    }
    fin.close();

    h->Draw();
    static TLine *lines[17];
    Double_t ymax = h->GetMaximum();
    for (Int_t i = 0; i < 16; i++) {
        lines[i] = new TLine(pos_low[i], 0.0, pos_low[i], ymax);
        lines[i]->SetLineColor(kRed);
        lines[i]->Draw();
    }
    lines[16] = new TLine(pos_high[15], 0.0, pos_high[15], ymax);
    lines[16]->SetLineColor(kRed);
    lines[16]->Draw();
}