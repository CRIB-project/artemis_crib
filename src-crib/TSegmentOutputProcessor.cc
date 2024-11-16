/**
 * @file    TSegmentOutputProcessor.cc
 * @brief   from seg conf, output raw data TTree object
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-08-21 17:36:19
 * @note    last modified: 2024-08-23 20:56:40
 * @details
 */

#include "TSegmentOutputProcessor.h"

#include <TArtemisUtil.h>
#include <TModuleData.h>
#include <TModuleType.h>
#include <TROOT.h>
#include <TRawDataObject.h>
#include <TSegmentInfo.h>
#include <TSegmentedData.h>
#include <TTree.h>
#include <constant.h>

using art::crib::TSegmentOutputProcessor;

ClassImp(TSegmentOutputProcessor);

TSegmentOutputProcessor::TSegmentOutputProcessor()
    : fFile(nullptr), fTree(nullptr), fSegmentList(nullptr), fModuleList(nullptr),
      fSegmentedData(nullptr) {
    RegisterProcessorParameter("FileName", "The name of output file", fFileName, TString("tmp.root"));
    RegisterProcessorParameter("TreeName", "The name of output tree", fTreeName, TString("tree"));

    StringVec_t defaultignore;
    defaultignore.push_back("");
    RegisterInputCollection("SegmentedDataName", "name of the segmented data",
                            fSegmentedDataName, TString("segdata"),
                            &fSegmentedData, "art::TSegmentedData");

    RegisterInputInfo("SegmentList", "name of the segment list",
                      fSegmentListName, TString("seglist"), &fSegmentList, "TClonesArray", "art::TSegmentInfo");
    RegisterInputInfo("ModuleList", "name of the module list",
                      fModuleListName, TString("modlist"), &fModuleList, "TClonesArray", "art::TModuleType");
    RegisterProcessorParameter("Ignore", "ignore segment list", fIgnore, defaultignore);
}

TSegmentOutputProcessor::~TSegmentOutputProcessor() {
    if (gROOT->GetListOfFiles()->FindObject(fFile)) {
        if (fTree)
            fTree->GetUserInfo()->Clear();
        fFile->Close();
    }

    // input processors are deleted elsewhere
    fSegmentedData = nullptr;
    fSegmentList = nullptr;
    fModuleList = nullptr;
}

TSegmentOutputProcessor::TSegmentOutputProcessor(const TSegmentOutputProcessor &) {}

TSegmentOutputProcessor &TSegmentOutputProcessor::operator=(const TSegmentOutputProcessor &rhs) {
    if (this != &rhs) {
    }
    return *this;
}

void TSegmentOutputProcessor::Init(TEventCollection *) {
    Util::PrepareDirectoryFor(fFileName);
    fFile = TFile::Open(fFileName, "RECREATE");
    if (!fFile) {
        SetStateError(TString::Format("Cannot create file: %s", fFileName.Data()));
        return;
    }
    fTree = new TTree(fTreeName, fTreeName);
    Info("init", "Created %s", fFileName.Data());

    Int_t nSeg = (*fSegmentList)->GetEntriesFast();
    for (Int_t iSeg = 0; iSeg != nSeg; iSeg++) {
        TSegmentInfo *seg = (TSegmentInfo *)(*fSegmentList)->At(iSeg);
        Int_t nMod = seg->GetNumModules();

        // skip if no module registered
        if (!nMod)
            continue;
        if (fIgnore.end() != std::find(fIgnore.begin(), fIgnore.end(), seg->GetName()))
            continue;

        std::pair<int, std::vector<TModuleData *>> segment;
        segment.first = seg->GetSegID();
        std::vector<TModuleData *> &modules = fSegments.insert(segment).first->second;

        for (Int_t iMod = 0; iMod != nMod; iMod++) {
            TModuleInfo *mod_info = seg->GetModule(iMod);
            TModuleData *mod = new TModuleData(*mod_info);
            Int_t id = mod->GetID(); // geo ID
            if (modules.size() < (unsigned int)id + 1)
                modules.resize(id + 1);
            modules.at(id) = mod;
            TModuleType *type = (TModuleType *)(*fModuleList)->FindObject(seg->GetModuleType());
            Int_t nCh = type->GetNch();
            mod->SetCh(nCh);

            Int_t mod_id = type->GetDecoderID();
            mod->SetMod(mod_id);

            // prepare branch (id == 24 or 25 -> multihit TDC)
            if (mod_id == 24 || mod_id == 25) {
                Info("Init", "Set %s_%d branch", seg->GetName(), id);
                fTree->Branch(Form("%s_%d", seg->GetName(), id), &(mod->fData2D));
            } else {
                Info("Init", "Set %s_%d branch", seg->GetName(), id);
                fTree->Branch(Form("%s_%d", seg->GetName(), id), &(mod->fData1D));
            }
        }
    }

    for (std::vector<TString>::iterator it = fIgnore.begin(); it != fIgnore.end(); it++) {
        if ((*it) == "") {
            continue;
        }
        printf("%s will be ignored\n", (*it).Data());
    }
}

void TSegmentOutputProcessor::Process() {
    std::map<Int_t, std::vector<TModuleData *>>::iterator it;

    for (it = fSegments.begin(); it != fSegments.end(); it++) {
        std::vector<TModuleData *> &modules = it->second;
        for (Size_t i = 0; i < modules.size(); i++) {
            Int_t mod = modules[i]->GetMod();
            if (mod < 0) {
                continue;
            } else if (mod == 24 || mod == 25) {
                std::vector<Int_t> v;
                v.emplace_back(kInvalidI);
                modules[i]->fData2D.assign(modules[i]->GetNCh(), v);
            } else {
                modules[i]->fData1D.assign(modules[i]->GetNCh(), kInvalidI);
            }
        }
    }

    for (it = fSegments.begin(); it != fSegments.end(); it++) {
        Int_t segid = it->first;
        TObjArray *arr = (*fSegmentedData)->FindSegmentByID(segid);
        if (!arr) {
            Warning("Process", "No segment having segid = %d", segid);
            Warning("Process", " Add this segid to Ignore if this semgment is not valid temporarily");
            SetStopLoop();
            return;
        }
        Int_t nHit = arr->GetEntriesFast();
        std::vector<TModuleData *> &modules = it->second;
        for (Int_t iHit = 0; iHit != nHit; iHit++) {
            TRawDataObject *data = (TRawDataObject *)arr->UncheckedAt(iHit);
            Int_t geo = data->GetGeo();
            Int_t ch = data->GetCh();

            // nVal is always 2 (see TRawTiming.h), so only take iVal=0
#if 0
            Int_t nVal = data->GetNumValues();
            Int_t tmp = -100;
            for (Int_t iVal = 0; iVal < nVal; iVal++) {
                if (modules.size() > geo && modules[geo] != nullptr) {
                    if (iVal == 0) {
                        tmp = data->GetValue(iVal);
                    } else {
                        if (tmp != data->GetValue(iVal)) {
                            std::cout << tmp << std::endl;
                            std::cout << data->GetValue(iVal) << std::endl;
                        }
                    }
                    // std::cout << data->GetValue(iVal) << std::endl;
                }
            }
#endif

            if ((Int_t)modules.size() > geo && modules[geo] != nullptr) {
                Int_t mod = modules[geo]->GetMod();
                if (mod == 24 || mod == 25) {
                    if (modules[geo]->fData2D[ch][0] == kInvalidI) {
                        modules[geo]->fData2D[ch][0] = data->GetValue(0);
                    } else {
                        modules[geo]->fData2D[ch].emplace_back(data->GetValue(0));
                    }
                } else {
                    modules[geo]->fData1D[ch] = data->GetValue(0);
                }
            }
        }
    }
    fTree->Fill();
}

void TSegmentOutputProcessor::PreLoop() {
}

void TSegmentOutputProcessor::PostLoop() {
    if (!gDirectory)
        gDirectory = gROOT;
    TDirectory *saved = gDirectory;
    fFile->cd();
    fTree->Write(0, TFile::kOverwrite);
    saved->cd();
}
