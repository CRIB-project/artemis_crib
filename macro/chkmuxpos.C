#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

void chkmuxpos(TH1F *h1, const TString &telname, const TString &sidename) {
    const char *artwork_dir = getenv("ARTEMIS_WORKDIR");
    if (!artwork_dir) {
        std::cerr << "Error: ARTEMIS_WORKDIR environment variable is not set.\n";
        return;
    }

    TString file_name = Form("%s/prm/%s/pos_%s/current",
                             artwork_dir, telname.Data(), sidename.Data());
    std::ifstream fin(file_name.Data());
    if (!fin) {
        std::cerr << "Error: File not found: " << file_name << "\n";
        return;
    }

    std::vector<std::vector<double>> boundaries;
    TString line;
    while (line.ReadLine(fin)) {
        line = line.Strip(TString::kBoth);
        if (line.BeginsWith("#") || line.IsNull()) {
            continue;
        }

        Ssiz_t hashIndex = line.Index("#");
        if (hashIndex != kNPOS) {
            line.Remove(hashIndex);
        }
        line = line.Strip(TString::kBoth);

        line.ReplaceAll(",", " ");
        line.ReplaceAll("\t", " ");
        line.ReplaceAll(" +", " ");

        std::unique_ptr<TObjArray> tokens(line.Tokenize(" "));
        if (!tokens) {
            continue;
        }

        std::vector<double> line_vec;
        for (int i = 0; i < tokens->GetEntries(); ++i) {
            auto *token = dynamic_cast<TObjString *>(tokens->At(i));
            if (!token) {
                std::cerr << "Invalid token at index " << i << "\n";
                continue;
            }

            TString valueStr = token->GetString();
            if (!valueStr.IsFloat()) {
                std::cerr << "Warning: Skipping invalid value: " << valueStr << "\n";
                continue;
            }
            line_vec.emplace_back(valueStr.Atof());
        }
        boundaries.emplace_back(std::move(line_vec));
    }
    fin.close();

    if (boundaries.empty() || boundaries[0].empty()) {
        std::cerr << "Error: No valid boundaries found in the file.\n";
        return;
    }

    // Draw histogram
    h1->Draw();

    // Process the first line
    const size_t num_lines = boundaries[0].size();
    double ymax = h1->GetMaximum();
    for (size_t i = 0; i < num_lines; ++i) {
        auto line = std::make_unique<TLine>(boundaries[0][i], 0.0, boundaries[0][i], ymax);
        line->SetLineColor(kRed);
        line->DrawClone();
    }
}
