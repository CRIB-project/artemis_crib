void scaler_info(TTree *tr) {
    const char *names[32] = {
        "Ungated",
        "Gated",
        "Beam x SSD",
        "Beam / n ",
        "F3PPACa",
        "F3PPACb",
        "SSD-OR",
        "Clock 10Hz",
        "-",
        "Pileup",
        "Beam",
        "-",
        "MWDCa cathode",
        "MWDCb cathode",
        "-",
        "SSD?",
        "N/A",
        "N/A",
        "N/A",
        "N/A",
        "N/A",
        "N/A",
        "N/A",
        "N/A",
        "N/A",
        "N/A",
        "N/A",
        "N/A",
        "N/A",
        "N/A",
        "N/A",
        "N/A"};

    Long64_t nEntries = tr->GetEntries();
    tr->Draw("scatot.fData", "", "goff", 1, nEntries - 1);
    std::cout << "======================================================\n";
    std::cout << " Scaler Information\n";
    std::cout << "======================================================\n";

    for (Int_t i = 0; i < 32; i++) {
        std::cout << Form(" %-20s: %15lld\n", names[i], static_cast<Long64_t>(tr->GetV1()[i]));
    }

    std::cout << "======================================================\n";
}