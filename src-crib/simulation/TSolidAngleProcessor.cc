/**
 * @file    TSolidAngleProcessor.cc
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-18 22:37:37
 * @note    last modified: 2024-08-23 21:20:12
 * @details
 */

////////////////////////////////////////////////////////////////////////////////
/// ## Usage
/// Using Monte Carlo Method, it calculate Solid angle.
/// The related file is,
/// - [prm/geo/hoge.yaml](https://github.com/okawak/artemis_crib/tree/main/prm/geo)
/// : define the detector geometry
/// - [steering/simSolidAngle.yaml](https://github.com/okawak/artemis_crib/blob/main/steering/simSolidAngle.yaml)
/// : steering file to calculate Solid Angle
///
/// ### Solid Target
///
/// We can assume the reaction position is one point (like (0, 0, 0)).
/// The art::crib::TNBodyReactionProcessor can produce events from this
/// point uniformly in CM system (angle), so if we calculate the ratio
/// of generated event number and detected event number.
///
/// The solid angle is defined like
/// - Omega(theta, delta_theta): Solid angle at theta angle with delta_theta width
///
/// ### Gas Target
///
/// If we fix the Ecm of the reaction, the reaction point is also fixed.
/// So with given Ecm, we can calculate the Solid angle by Monte Carlo method.
///

#include "TSolidAngleProcessor.h"

#include "../reconst/TReactionInfo.h"
#include "../telescope/TTelescopeData.h"
#include <TArtemisUtil.h>
#include <TFile.h>

using art::crib::TSolidAngleProcessor;

ClassImp(TSolidAngleProcessor);

////////////////////////////////////////////////////////////////////////////////
/// steering file definition:
/// - InputCollection: detected particle branch (size = 1 of art::TTelescopeData)
/// - InputReacCollection: simulated reaction branch
/// - Nbin_angle: A1
/// - range_angle: [A2, A3]
///   - histogram binning like  tree->Draw(A1, A2, A3)
/// - Nbin_energy: E1
/// - range_energy: [E2, E3]
///   - histogram binning like  tree->Draw(E1, E2, E3)
/// - HistFile: saved histogram ROOT file name
///

TSolidAngleProcessor::TSolidAngleProcessor() : fInData(nullptr), fInReacData(nullptr) {
    RegisterInputCollection("InputCollection", "input branch name", fInputColName, TString("data"));
    RegisterInputCollection("InputReacCollection", "input reaction branch name", fInputReacColName, TString("reaction"));

    DoubleVec_t init_d_vec;
    RegisterProcessorParameter("Nbin_angle", "Nbin of the angle histogram", fNbin_angle, 0);
    RegisterProcessorParameter("range_angle", "range of the angle histogram", fRange_angle, init_d_vec);
    RegisterProcessorParameter("Nbin_energy", "Nbin of the energy histogram", fNbin_energy, 0);
    RegisterProcessorParameter("range_energy", "range of the energy histogram", fRange_energy, init_d_vec);

    RegisterProcessorParameter("HistFile", "name of output histogram rootfile", fFileName, TString(""));
}

////////////////////////////////////////////////////////////////////////////////
/// free the memory.

TSolidAngleProcessor::~TSolidAngleProcessor() {
    delete h1_e;
    delete h1_e_all;
    delete h1_a;
    delete h1_a_all;
    delete h2;
    h1_e = nullptr;
    h1_e_all = nullptr;
    h1_a = nullptr;
    h1_a_all = nullptr;
    h2 = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// Initialization process

void TSolidAngleProcessor::Init(TEventCollection *col) {
    Info("Init", "make solid angle histogram Ecm and Thetacm");
    if (fRange_angle.size() != 2 || fRange_energy.size() != 2) {
        SetStateError("input steering yaml format is wrong, range size should be 2");
        return;
    }
    Info("Init", "\tAcm(%d, %.1lf, %.1lf)", fNbin_angle, fRange_angle[0], fRange_angle[1]);
    Info("Init", "\tEcm(%d, %.1lf, %.1lf)", fNbin_energy, fRange_energy[0], fRange_energy[1]);
    Info("Init", "\t2D(%d, %.1lf, %.1lf, %d, %.1lf, %.1lf)",
         fNbin_energy, fRange_energy[0], fRange_energy[1],
         fNbin_angle, fRange_angle[0], fRange_angle[1]);
    Info("Init", "saved %s", fFileName.Data());

    fInData = reinterpret_cast<TClonesArray **>(col->GetObjectRef(fInputColName.Data()));
    if (!fInData) {
        SetStateError(Form("input not found: %s", fInputColName.Data()));
        return;
    }
    const TClass *const cl = (*fInData)->GetClass();
    if (!cl->InheritsFrom(art::crib::TTelescopeData::Class())) {
        SetStateError("contents of input array must inherit from art::crib::TTelescopeData");
    }

    fInReacData = reinterpret_cast<TClonesArray **>(col->GetObjectRef(fInputReacColName.Data()));
    if (!fInReacData) {
        SetStateError(Form("input not found: %s", fInputReacColName.Data()));
        return;
    }
    const TClass *const cl_reac = (*fInReacData)->GetClass();
    if (!cl_reac->InheritsFrom(art::crib::TReactionInfo::Class())) {
        SetStateError("contents of input array must inherit from art::crib::TReactionInfo");
    }

    h1_a_all = new TH1D("norm_Acm", "norm_Acm", fNbin_angle, fRange_angle[0], fRange_angle[1]);
    h1_e_all = new TH1D("norm_Ecm", "norm_Ecm", fNbin_energy, fRange_energy[0], fRange_energy[1]);
    h2_all = new TH2D("norm_2D", "norm_2D", fNbin_energy, fRange_energy[0], fRange_energy[1],
                      fNbin_angle, fRange_angle[0], fRange_angle[1]);

    h1_a = new TH1D("Acm", "Acm", fNbin_angle, fRange_angle[0], fRange_angle[1]);
    h1_e = new TH1D("Ecm", "Ecm", fNbin_energy, fRange_energy[0], fRange_energy[1]);
    h2 = new TH2D("2D", "2D", fNbin_energy, fRange_energy[0], fRange_energy[1],
                  fNbin_angle, fRange_angle[0], fRange_angle[1]);
}

////////////////////////////////////////////////////////////////////////////////
/// Judge the event is detected at the detector and fill
/// to the histograms

void TSolidAngleProcessor::Process() {
    if ((*fInReacData)->GetEntriesFast() != 1) {
        SetStateError("input reaction data branch entry is not 1");
        return;
    }

    if ((*fInData)->GetEntriesFast() != 1) {
        SetStateError("input telescope data branch entry is not 1");
        return;
    }

    const TDataObject *const inData = static_cast<TDataObject *>((*fInReacData)->At(0));
    const TReactionInfo *const Data = dynamic_cast<const TReactionInfo *>(inData);
    h1_a_all->Fill(Data->GetTheta());
    h1_e_all->Fill(Data->GetEnergy());
    h2_all->Fill(Data->GetEnergy(), Data->GetTheta());

    // detected of not
    const TDataObject *const inDetData = static_cast<TDataObject *>((*fInData)->At(0));
    const TTelescopeData *const DetData = dynamic_cast<const TTelescopeData *>(inDetData);
    if (DetData->GetTelID() > 0) {
        h1_a->Fill(Data->GetTheta());
        h1_e->Fill(Data->GetEnergy());
        h2->Fill(Data->GetEnergy(), Data->GetTheta());
    }
}

////////////////////////////////////////////////////////////////////////////////
/// After all the events are processed, scale the histograms and
/// save them

void TSolidAngleProcessor::EndOfRun() {
    Util::PrepareDirectoryFor(fFileName);
    TFile *file = TFile::Open(fFileName, "recreate");
    if (!file || file->IsZombie()) {
        std::cerr << "ERROR : cannot open " << fFileName << std::endl;
        delete file;
        return;
    }

    h1_a->Divide(h1_a_all);
    h1_a->Scale(4.0 * TMath::Pi());
    h1_a->SetTitle("Solid Angle;Angle CM (deg);");
    h1_a->Write();

    h1_e->Divide(h1_e_all);
    h1_e->Scale(4.0 * TMath::Pi());
    h1_e->SetTitle("Solid Angle;Energy CM (MeV);");
    h1_e->Write();

    h2->Divide(h2_all);
    h2->Scale(4.0 * TMath::Pi());
    h2->SetTitle("Solid Angle;Energy CM (MeV);Angle CM (deg)");
    h2->Write();

    file->Close();
    delete file;
}
