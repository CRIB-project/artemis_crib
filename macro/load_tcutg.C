#include <glob.h>

TCutG *getTCutObject(TString filepath, TString tcutname) {
    FileStat_t info;
    if (gSystem->GetPathInfo(filepath, info) != 0) {
        std::cerr << "Error in <userlogon.C>: cannot open: " << filepath << std::endl;
        return NULL;
    }
    TFile *file = new TFile(filepath, "read");

    TCutG *cut = (TCutG *)file->Get(tcutname);
    if (!cut) {
        std::cerr << "Error in <userlogon.C>: cannot find TCutG object: " << tcutname << std::endl;
        return NULL;
    }

    file->Close();
    std::cout << "[Info] TCutG object \"" << cut->GetName() << "\" is loaded \t(VarX:" << cut->GetVarX()
              << " VarY:" << cut->GetVarY() << ")" << std::endl;
    return cut;
}

void getAllTCutObject() {
    glob_t globbuf;
    glob("gate/*.root", 0, NULL, &globbuf);
    for (Int_t i = 0; i < globbuf.gl_pathc; i++) {
        TString filepath = (globbuf.gl_pathv[i]);
        TString file = filepath(filepath.First("/") + 1, filepath.Length());
        TString gate = file(0, file.First("."));
        getTCutObject(filepath, gate);
    }
    globfree(&globbuf);
}
