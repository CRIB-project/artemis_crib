/////////////////////////////////////////////////////////////////////////////
// dE E plot calibration using SRIM
// input:
//     ion_name             : name of beam ion like "26Si14" used in SRIBlib
//     telescope_name       : telescope branch name like "tel1"
//     NdE                  : number of dE detector
//                              (if we use two layer as dE detector)
//
// to be updated...
/////////////////////////////////////////////////////////////////////////////

#include <SRIMData.h>
#include <SRIMtable.h>
#include <cstdlib>
#include <yaml-cpp/yaml.h>

TGraph *GetdEEplot(TString ion_name, TString telescope_name, Int_t NdE = 1) {
    // prepare thickenss vector of telescope
    const TString ARTEMIS_WORKDIR = gSystem->pwd();
    const char *EXP_NAME = std::getenv("EXP_NAME");
    TString geo_file = Form("%s/prm/geo/%s.yaml", ARTEMIS_WORKDIR.Data(), EXP_NAME);

    FileStat_t info;
    if (gSystem->GetPathInfo(geo_file.Data(), info) != 0) {
        std::cerr << Form("File %s does not exist.", geo_file.Data()) << std::endl;
        return nullptr;
    }

    YAML::Node yaml_all = YAML::LoadFile(geo_file.Data());
    YAML::Node det_prm = yaml_all["conposition"]["detector"].as<YAML::Node>();

    std::vector<Double_t> thickness;
    for (Int_t i = 0; i < det_prm.size(); i++) {
        TString name = det_prm[i]["name"].as<std::string>();
        if (name == telescope_name) {
            thickness = det_prm[i]["thickness"].as<std::vector<Double_t>>();
            break;
        }
    }
    if (thickness.size() == 0) {
        std::cerr << Form("We cannot find %s thickness Node", telescope_name.Data()) << std::endl;
        return nullptr;
    }

    // calculation
    SRIMData *srimdata = new SRIMData();
    SRIMtable *table = srimdata->GetTable(ion_name, "Si");
    if (!table) {
        std::cerr << "ion_name need like \"1H1\", \"4He2\" format" << std::endl;
        return nullptr;
    }

    TGraph *gr = new TGraph();
    Int_t index = 0;
    for (Double_t inE = 0.0; inE < 30.0; inE += 0.1) {
        Double_t current_E = inE;
        Double_t dE = 0.0;
        Double_t Etotal = 0.0;

        for (Int_t i = 0; i < thickness.size(); i++) {
            Double_t after_energy = table->GetNewE(current_E, thickness[i]);
            Double_t energy_deposit = current_E - after_energy;
            Etotal += energy_deposit;
            if (i < NdE) {
                dE += energy_deposit;
            }
            current_E = after_energy;

            if (current_E < 0.01) {
                gr->SetPoint(index, dE, Etotal);
                index++;
                break;
            }

            if (i == thickness.size() - 1) {
                gr->SetPoint(index, dE, Etotal);
                index++;
                break;
            }
        }
    }
    gr->SetTitle(Form("%s dE-E plot with SRIM;total energy (MeV);dE energy (MeV)", telescope_name.Data()));
    gr->SetLineColor(kRed);
    gr->SetLineWidth(2);
    gr->SetMarkerColor(kRed);
    gr->SetMarkerSize(2);
    gr->SetMarkerStyle(8);

    return gr;
}