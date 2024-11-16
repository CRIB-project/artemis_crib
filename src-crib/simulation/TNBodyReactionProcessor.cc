/**
 * @file    TNBodyReactionProcessor.cc
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-08-01 22:36:36
 * @note    last modified: 2024-10-30 16:22:33
 * @details for (angle) constant cross section
 */

#include "TNBodyReactionProcessor.h"

#include "../reconst/TReactionInfo.h"
#include "TParticleInfo.h"
#include <Mass.h> // TSrim library
#include <TRandom.h>
#include <filesystem>
#include <fstream>
#include <regex>

using art::crib::TNBodyReactionProcessor;

ClassImp(TNBodyReactionProcessor);

TNBodyReactionProcessor::TNBodyReactionProcessor()
    : fInData(nullptr), fOutData(nullptr), fOutReacData(nullptr), srim(nullptr) {
    RegisterInputCollection("InputCollection", "input branch (collection) name", fInputColName, TString("input"));
    RegisterOutputCollection("OutputCollection", "output branch (collection) name", fOutputColName,
                             TString("reaction_particles"));
    RegisterOutputCollection("OutputReactionCollection", "output branch (collection) name", fOutputReacColName,
                             TString("reaction"));

    // beam information
    IntVec_t init_i_vec;
    RegisterProcessorParameter("BeamNucleus", "beam nucleus (Z, A)", fBeamNucleus, init_i_vec);
    RegisterProcessorParameter("BeamEnergy", "initial beam energy used for initializing TSrim object",
                               fBeamEnergy, 0.0);

    // Target information
    RegisterProcessorParameter("TargetIsGas", "Bool, target is targer or not", fTargetIsGas, false);
    RegisterProcessorParameter("TargetName", "name of target material", fTargetName, TString(""));
    RegisterProcessorParameter("TargetMassNum", "Target mass number", fTargetMassNum, 0);
    RegisterProcessorParameter("TargetAtomicNum", "Target atomic number", fTargetAtmNum, 0);
    RegisterProcessorParameter("TargetThickness", "Target mass", fTargetThickness, 0.0);
    RegisterProcessorParameter("TargetPressure", "Target presure", fTargetPressure, 0.0);

    // reaction particles information
    DoubleVec_t init_d_vec;
    RegisterProcessorParameter("DecayParticleNum", "reaction particle number", fDecayNum, 1);
    RegisterProcessorParameter("ReactionMassNum", "reaction particles mass number", fReacMassNum, init_i_vec);
    RegisterProcessorParameter("ReactionAtomicNum", "reaction particles atomic number", fReacAtmNum, init_i_vec);
    RegisterProcessorParameter("ExciteLevel", "energy level for id=0 nucleus", fExciteLevel, init_d_vec);
    RegisterProcessorParameter("CrossSectionPath", "path to the cross section data file", fCSDataPath, TString(""));
    RegisterProcessorParameter("CrossSectionType",
                               "energy format, 0: LAB energy like TALYS, 1: LAB at inverse kinematics, 2: Ecm", fCSType, 0);
}

TNBodyReactionProcessor::~TNBodyReactionProcessor() {
    delete fOutData;
    delete fOutReacData;
    delete srim;
    delete gr_generating_func;
    delete gr_generating_func_inv;
    fOutData = nullptr;
    fOutReacData = nullptr;
    srim = nullptr;
    gr_generating_func = nullptr;
    gr_generating_func_inv = nullptr;
}

void TNBodyReactionProcessor::Init(TEventCollection *col) {
    Info("Init", "%s => %s, decay num: %d, reaction info => %s",
         fInputColName.Data(), fOutputColName.Data(), fDecayNum, fOutputReacColName.Data());

    if (fDecayNum != (Int_t)fReacMassNum.size() ||
        fDecayNum != (Int_t)fReacAtmNum.size() ||
        fDecayNum != (Int_t)fExciteLevel.size()) {
        SetStateError("in input yaml, DecayParticleNum and the size of other elements are different");
        return;
    }

    if (fBeamNucleus.size() != 2) {
        SetStateError("fBeamNucleus format need (Z, A): ex (2, 4)");
        return;
    }

    for (Int_t i = 0; i < fDecayNum; i++) {
        Info("Init", "reaction products: id=%d, %s (A=%d, Z=%d), Ex = %lf", i,
             amdc::GetEl(fReacAtmNum[i]).c_str(), fReacMassNum[i], fReacAtmNum[i], fExciteLevel[i]);
    }

    fInData = reinterpret_cast<TClonesArray **>(col->GetObjectRef(fInputColName.Data()));
    if (!fInData) {
        SetStateError(TString::Format("input not found: %s", fInputColName.Data()));
        return;
    }
    const TClass *const cl = (*fInData)->GetClass();
    if (!cl->InheritsFrom(art::crib::TParticleInfo::Class())) {
        SetStateError("contents of input array must inherit from art::crib::TParticleInfo");
        return;
    }

    fOutData = new TClonesArray("art::crib::TParticleInfo");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);

    fOutReacData = new TClonesArray("art::crib::TReactionInfo");
    fOutReacData->SetName(fOutputReacColName);
    col->Add(fOutputReacColName, fOutReacData, fOutputIsTransparent);

    const char *tsrim_path = std::getenv("TSRIM_DATA_HOME");
    if (!tsrim_path) {
        SetStateError("TSRIM_DATA_HOME environment variable is not defined");
        return;
    }
    // get All SRIM table for the target
    Info("Init", "Initializing SRIM table...");
    srim = new TSrim("srim", 16,
                     Form("%s/%s/range_fit_pol16_%s.txt", tsrim_path, fTargetName.Data(), fTargetName.Data()));
    Info("Init", "\t\"%s\" list loaded.", fTargetName.Data());

    // cross section input file
    InitGeneratingFunc();

    gRandom->SetSeed(time(nullptr));
}

void TNBodyReactionProcessor::Process() {
    fOutData->Clear("C");
    fOutReacData->Clear("C");

    if ((*fInData)->GetEntriesFast() != 1) {
        SetStateError("input branch entry is not 1");
        return;
    }

    const TDataObject *const inData = static_cast<TDataObject *>((*fInData)->At(0));
    const TParticleInfo *const Data = dynamic_cast<const TParticleInfo *>(inData);

    Double_t target_mass = amdc::Mass(fTargetAtmNum, fTargetMassNum) * amdc::amu; // MeV
    TLorentzVector target_vec(0., 0., 0., target_mass);
    TLorentzVector beam_vec = Data->GetLorentzVector();
    Double_t beam_mass = amdc::Mass(fBeamNucleus[0], fBeamNucleus[1]) * amdc::amu; // MeV
    Double_t beam_energy = Data->GetEnergy();

    // calculate reaction position
    // target should be set at z=0 (entrance of gas target)
    Double_t range;
    if (fTargetIsGas) {
        range = srim->Range(fBeamNucleus[0], fBeamNucleus[1], beam_energy,
                            std::string(fTargetName.Data()), fTargetPressure, 300.0);
    } else {
        range = srim->Range(fBeamNucleus[0], fBeamNucleus[1], beam_energy,
                            std::string(fTargetName.Data()));
    }

    // determine using random number
    Double_t reac_distance = GetRandomReactionDistance(range);
    Double_t beam_energy_new = 0.0;
    if (fTargetIsGas) {
        beam_energy_new = srim->EnergyNew(fBeamNucleus[0], fBeamNucleus[1], beam_energy,
                                          std::string(fTargetName.Data()), reac_distance, fTargetPressure, 300.0);
    } else {
        beam_energy_new = srim->EnergyNew(fBeamNucleus[0], fBeamNucleus[1], beam_energy,
                                          std::string(fTargetName.Data()), reac_distance);
    }

    Double_t reac_posz = 0.0;
    if (fTargetIsGas) {
        reac_posz = reac_distance * TMath::Cos(Data->GetTrack().GetA()) * TMath::Cos(Data->GetTrack().GetB());
    }
    Double_t reac_posx = Data->GetTrack().GetX(reac_posz);
    Double_t reac_posy = Data->GetTrack().GetY(reac_posz);

    beam_vec = GetLossEnergyVector(beam_vec, beam_energy - beam_energy_new);

    // calculate tof in the target (mean value)
    Double_t duration_beam = 0.0;
    if (fTargetIsGas) {
        duration_beam += reac_distance / (TMath::Sqrt(1.0 - TMath::Power(beam_mass / beam_vec.E(), 2.0)) * c);
    } // if solid target, tof is almost zero

    DoubleVec_t reac_masses;
    for (Int_t iPart = 0; iPart < fDecayNum; iPart++) {
        Double_t mass = amdc::Mass(fReacAtmNum[iPart], fReacMassNum[iPart]) * amdc::amu; // MeV
        mass += fExciteLevel[iPart];
        reac_masses.emplace_back(mass);
    }

    TLorentzVector compound_vec = beam_vec + target_vec;

    // to CM system (only beam_vec and target_vec)
    TVector3 beta_vec = compound_vec.BoostVector();
    beam_vec.Boost(-beta_vec);
    target_vec.Boost(-beta_vec);
    Double_t energy_cm = (beam_vec.E() - beam_vec.M()) + (target_vec.E() - target_mass);

    // need to change MeV to GeV
    compound_vec *= 0.001;
    DoubleVec_t GeVMass;
    for (decltype(reac_masses.size()) i = 0; i < reac_masses.size(); i++) {
        GeVMass.emplace_back(reac_masses[i] * 0.001);
    }

    Bool_t isOkay = event.SetDecay(compound_vec, fDecayNum, GeVMass.data());
    if (!isOkay) {
        std::cerr << "forbidden kinematics" << std::endl;
    }
    event.Generate();

    Double_t theta_cm = 0.0;
    for (Int_t iPart = 0; iPart < fDecayNum; ++iPart) {
        TLorentzVector reac_vec = *event.GetDecay(iPart);
        // need to change GeV to MeV
        reac_vec *= 1000.;

        TParticleInfo *outData = static_cast<TParticleInfo *>(fOutData->ConstructedAt(iPart));
        outData->SetID(iPart);
        outData->SetMassNumber(fReacMassNum[iPart]);
        outData->SetAtomicNumber(fReacAtmNum[iPart]);
        outData->SetCharge(fReacAtmNum[iPart]); // no need?
        outData->SetTrack(reac_posx, reac_posy, reac_posz, TMath::ATan(reac_vec.Px() / reac_vec.Pz()),
                          TMath::ATan(reac_vec.Py() / reac_vec.Pz()));
        if (fTargetIsGas) {
            outData->SetLorentzVector(reac_vec);
            outData->SetEnergy(reac_vec.E() - reac_masses[iPart]);
            outData->SetCurrentZ(reac_posz);
            outData->SetZeroTime(); // initialize
            outData->AddTime(duration_beam);
        } else {
            // for the solid target, ignore the angle
            Double_t first_energy = reac_vec.E() - reac_masses[iPart];
            if (first_energy > 0.01) {
                Double_t out_thickness = fTargetThickness - reac_distance;
                if (out_thickness < 0.0) {
                    out_thickness = 0.0;
                }
                TLorentzVector out_vec =
                    GetLossEnergyVector(reac_vec,
                                        first_energy - srim->EnergyNew(fReacAtmNum[iPart], fReacMassNum[iPart], first_energy, std::string(fTargetName.Data()), out_thickness));

                outData->SetLorentzVector(out_vec);
                outData->SetEnergy(out_vec.E() - reac_masses[iPart]);
                outData->SetCurrentZ(fTargetThickness);
                outData->SetZeroTime();
            } else {
                // outData->SetLorentzVector(reac_vec);
                outData->SetLorentzVector(0.0, 0.0, 0.0, reac_masses[iPart]);
                outData->SetEnergy(0.0);
                outData->SetCurrentZ(reac_posz);
                outData->SetZeroTime();
            }
        }

        // compound_vec and reac_vec is LAB system, convert to CM system
        reac_vec.Boost(-beta_vec);
        outData->SetThetaCM(reac_vec.Theta() / deg2rad);
        outData->SetPhiCM(reac_vec.Phi() / deg2rad);
        if (iPart == 0) {
            theta_cm = (Data->GetLorentzVector()).Angle(reac_vec.Vect()) / deg2rad;
        }
    }

    TReactionInfo *outReacData = static_cast<TReactionInfo *>(fOutReacData->ConstructedAt(0));
    outReacData->SetID(0);
    outReacData->SetExEnergy(fExciteLevel[0]);
    outReacData->SetEnergy(energy_cm);
    outReacData->SetTheta(theta_cm);
    outReacData->SetXYZ(reac_posx, reac_posy, reac_posz);
}

void TNBodyReactionProcessor::InitGeneratingFunc() {
    gr_generating_func = new TGraph();
    gr_generating_func_inv = new TGraph();

    // simplize range
    auto get_range = [&](Double_t e) {
        if (fTargetIsGas) {
            return srim->Range(fBeamNucleus[0], fBeamNucleus[1], e, std::string(fTargetName.Data()), fTargetPressure, 300.0);
        } else {
            return srim->Range(fBeamNucleus[0], fBeamNucleus[1], e, std::string(fTargetName.Data()));
        }
    };

    // get dE/dx (x : range)
    auto dedx = [&](Double_t e) {
        const Double_t h = 0.1;
        Double_t dxde = (get_range(e + h) - get_range(e)) / h;
        return 1.0 / dxde;
    };

    auto gr_density_func = new TGraph();
    Int_t index = 0; // index for TGraph
    Double_t xmin = 0.0, xmax = 0.0;
    Bool_t is_exist = std::filesystem::exists(fCSDataPath.Data());
    if (!is_exist) {
        Info("Init", "no input cross section file, use uniform energy distribution");
        for (auto e = 0.0; e < fBeamEnergy * 1.5; e += 0.5) {
            Double_t range = get_range(e);
            gr_density_func->SetPoint(index, range, dedx(e));
            index++;
            xmax = range;
            if (index == 1) {
                xmin = range;
            }
        }
    } else {
        std::ifstream fin(fCSDataPath.Data());
        Info("Init", "get cross section file: %s", fCSDataPath.Data());
        Double_t ene_factor = 0.0;
        if (fCSType == 0) {
            ene_factor = amdc::Mass(fBeamNucleus[0], fBeamNucleus[1]) / amdc::Mass(fTargetAtmNum, fTargetMassNum);
        } else if (fCSType == 1) {
            ene_factor = 1.0;
        } else if (fCSType == 2) {
            ene_factor = (amdc::Mass(fBeamNucleus[0], fBeamNucleus[1]) + amdc::Mass(fTargetAtmNum, fTargetMassNum)) /
                         amdc::Mass(fTargetAtmNum, fTargetMassNum);
        } else {
            SetStateError("CrossSectionType should be 0, 1 or 2");
            return;
        }

        std::string line;
        std::regex commentRegex("#.*");
        while (std::getline(fin, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }
            line = std::regex_replace(line, commentRegex, "");

            std::vector<TString> values;
            TString line_copy = line;
            while (line_copy.Contains(' ')) {
                Int_t index = line_copy.First(' ');
                TString line_val, line_remain;
                line_val = line_copy(0, index);
                line_remain = line_copy(index + 1, line_copy.Length());
                line_copy = line_remain;
                if (line_val == "") {
                    continue;
                }
                values.emplace_back(line_val);
            }
            Double_t e = ene_factor * values[0].Atof();
            Double_t cs = values[1].Atof() / ene_factor;
            Double_t range = get_range(e);
            gr_density_func->SetPoint(index, range, cs * dedx(e));
            index++;
            xmax = range;
            if (index == 1) {
                xmin = range;
            }
        }
    }

    auto f = new TF1("f", [&](double *x, double *p) { return p[0] * gr_density_func->Eval(x[0], nullptr, "S"); }, xmin, xmax, 1);
    f->SetParameter(0, 1.0);

    Info("Init", "suppressing the error message in this initialization process");
    Info("Init", "please wait for a moment...");

    // suppress error message when calculating the integral value
    __attribute__((unused)) auto *_dev_null = std::freopen("/dev/null", "w", stderr);
    index = 0;
    Double_t x_step = (xmax - xmin) / 1000.0; // 1000 steps
    for (auto x = xmin; x < xmax; x += x_step) {
        gr_generating_func->SetPoint(index, x, f->Integral(xmin, x));
        gr_generating_func_inv->SetPoint(index, f->Integral(xmin, x), x);
        index++;
    }
    __attribute__((unused)) auto *_dev_tty = std::freopen("/dev/tty", "w", stderr);

    Info("Init", "total cross section (arbitrary unit or mb)");
    Info("Init", "\t%lf", gr_generating_func->Eval(get_range(fBeamEnergy), nullptr, "S"));
}

Double_t TNBodyReactionProcessor::GetRandomReactionDistance(Double_t range) {
    Double_t random_x = 0.0;
    Double_t max_x = gr_generating_func->Eval(range, nullptr, "S");
    if (range < fTargetThickness) {
        random_x = max_x * gRandom->Uniform();
    } else {
        Double_t limit_range = range - fTargetThickness;
        Double_t min_x = gr_generating_func->Eval(limit_range, nullptr, "S");
        random_x = gRandom->Uniform(min_x, max_x);
    }
    Double_t distance = gr_generating_func_inv->Eval(random_x, nullptr, "S");

    return range - distance;
}

TLorentzVector TNBodyReactionProcessor::GetLossEnergyVector(TLorentzVector vec, Double_t eloss) {
    Double_t factor =
        ((vec.E() - eloss) * (vec.E() - eloss) - vec.M() * vec.M()) / (vec.E() * vec.E() - vec.M() * vec.M());
    factor = TMath::Sqrt(factor);
    TLorentzVector new_vec(factor * vec.Px(), factor * vec.Py(), factor * vec.Pz(), vec.E() - eloss);
    return new_vec;
}

// ===========================================
// kinematics
// lorentz invariant
// m^2 = (E - Eloss)^2 - (factor*p)^2
// factor*2 = ( (E - Eloss)^2 - m^2 )/p^2
// factor = sqrt( ((E - Eloss)^2 - m2)/(E^2 - m^2) )
//
// tof
// beta = v/c
// E = m/sqrt(1-beta^2)
// v = sqrt(1-(M/E)^2) c
