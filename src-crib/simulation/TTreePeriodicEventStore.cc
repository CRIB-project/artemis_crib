/**
 * @file    TTreePeriodicEventStore.cc
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-08-01 22:36:36
 * @note    last modified: 2025-01-08 20:02:02
 * @details just modify the process() from TTreeEventStore to return 0
 */

#include "TTreePeriodicEventStore.h"

#include <TChain.h>
#include <TEventHeader.h>
#include <TLeaf.h>
#include <TObjString.h>
#include <TSystem.h>

/// ROOT macro for class implementation
ClassImp(art::crib::TTreePeriodicEventStore);

namespace art::crib {

TTreePeriodicEventStore::TTreePeriodicEventStore() {
    RegisterProcessorParameter("FileName", "The name of input file", fFileName, fFileName);
    RegisterProcessorParameter("TreeName", "The name of input tree", fTreeName, fTreeName);
    RegisterProcessorParameter("MaxEventNum", "The maximum event number to be analyzed.", fMaxEventNum, 0L);
}
TTreePeriodicEventStore::~TTreePeriodicEventStore() {
    fTree = nullptr;
}

/**
 * @details
 * Init method:
 *  1) Retrieve fCondition from TEventCollection
 *  2) Get file list from shell command
 *  3) Build TChain if necessary
 *  4) Set branch addresses (primitive or class objects)
 *  5) Load the first entry and get total entry counts
 */
void TTreePeriodicEventStore::Init(TEventCollection *col) {
    // 1) Get fCondition pointer from TEventCollection (for TLoop control)
    void **tmpCondRef = col->Get(TLoop::kConditionName)->GetObjectRef();
    if (!tmpCondRef) {
        Error("Init", "fCondition: tmpCondRef is null");
        return;
    }
    fCondition = reinterpret_cast<TConditionBit **>(tmpCondRef);
    if (!fCondition) {
        Error("Init", "fCondition is null after reinterpret_cast");
        return;
    }
    if (*fCondition) {
        TConditionBit *condPtr = dynamic_cast<TConditionBit *>(*fCondition);
        if (!condPtr) {
            Error("Init", "fCondition: *fCondition is not a TConditionBit!");
            return;
        }
    } else {
        Warning("Init", "fCondition: *fCondition is null. It might be set later?");
    }

    // 2) Get the list of files
    auto filelist = gSystem->GetFromPipe(Form("ls -tr %s", fFileName.Data()));
    auto *allfiles = filelist.Tokenize("\n");
    if (!allfiles) {
        SetStateError(Form("No files matched '%s'", fFileName.Data()));
        if (fCondition)
            (*fCondition)->Set(TLoop::kStopLoop);
        return;
    }

    // Check MaxEventNum
    if (fMaxEventNum == 0) {
        SetStateError("MaxEventNum == 0 => No event is processed.");
        if (fCondition)
            (*fCondition)->Set(TLoop::kStopLoop);
        return;
    }

    // 3) Build TChain if fTree is not created yet
    if (!fTree) {
        auto *chain = new TChain(fTreeName);
        for (int i = 0, n = allfiles->GetEntriesFast(); i < n; ++i) {
            auto *strObj = dynamic_cast<TObjString *>(allfiles->At(i));
            if (!strObj)
                continue;
            Info("Init", "Add '%s'", (strObj->GetString()).Data());
            chain->Add(strObj->GetString());
        }
        fTree = chain;
    }

    if (!fTree) {
        Error("Init", "Failed to create TChain for tree = '%s'", fTreeName.Data());
        if (fCondition)
            (*fCondition)->Set(TLoop::kStopLoop);
        return;
    }

    // Initialize event counters
    fEventNum = 0;
    fCurrentNum = 0;

    // 4) Explore each branch and set addresses
    std::vector<TBranch *> useBranch;
    TIter nextBr(fTree->GetListOfBranches());
    while (auto *br = dynamic_cast<TBranch *>(nextBr())) {
        TClass *cl = nullptr;
        EDataType dtype = kNoType_t;
        // If GetExpectedType fails, skip
        if (br->GetExpectedType(cl, dtype)) {
            Warning("Init", "Unresolved type for branch '%s'", br->GetName());
            continue;
        }

        // Case A: Primitive type
        if (!cl) {
            // We force arrSize to be at least 1
            int arrSize = 1;
            if (auto *leaf = br->GetLeaf(br->GetName())) {
                leaf = leaf->GetLeafCounter(arrSize);
                if (arrSize < 1) {
                    arrSize = 1; // enforce a minimum of 1
                }
            }
            void *arrPtr = nullptr;
            switch (dtype) {
            case kInt_t:
                arrPtr = new Int_t[arrSize];
                break;
            case kFloat_t:
                arrPtr = new Float_t[arrSize];
                break;
            case kDouble_t:
                arrPtr = new Double_t[arrSize];
                break;
            default:
                break;
            }
            if (!arrPtr) {
                continue;
            }

            // Register this array in TEventCollection
            col->Add(new TEventObject(
                br->GetName(),
                arrPtr,
                TString(TDataType::GetTypeName(dtype)),
                nullptr));
            auto **objRef = col->Get(br->GetName())->GetObjectRef();
            if (!objRef) {
                Error("Init", "Branch '%s': tmpRef is null (unexpected)", br->GetName());
                continue;
            }
            fTree->SetBranchAddress(br->GetName(), *objRef);
            useBranch.emplace_back(br);
            Info("Init", "branch : %s (type=%s, size=%d)",
                 br->GetName(), TDataType::GetTypeName(dtype), arrSize);
        }
        // Case B: Class type (including TClonesArray)
        else {
            // Check TClonesArray
            if (cl == TClonesArray::Class()) {
                TClonesArray *arr = nullptr;
                // Temporarily enable only this branch to check its class info
                fTree->SetBranchStatus("*", 0);
                fTree->SetBranchStatus(br->GetName(), 1);
                fTree->SetBranchAddress(br->GetName(), &arr);
                fTree->GetEntry(0);

                if (arr) {
                    TClass *realcls = arr->GetClass();
                    if (!realcls || !realcls->GetNew()) {
                        // If the class cannot be instantiated, skip
                        cl = nullptr;
                    }
                }
                br->ResetAddress();
            }

            // If still valid, create an instance and set address
            if (cl) {
                // Create a new object of this class
                auto *obj = static_cast<TObject *>(cl->New());
                col->Add(br->GetName(), obj, kTRUE);

                void **tmpRef = col->Get(br->GetName())->GetObjectRef();
                if (!tmpRef) {
                    Error("Init", "Branch '%s': tmpRef is null (unexpected)", br->GetName());
                    continue;
                }

                auto **objRef = reinterpret_cast<TObject **>(tmpRef);
                if (!objRef) {
                    Error("Init", "Branch '%s': objRef is null after cast", br->GetName());
                    continue;
                }
                if (*objRef) {
                    TObject *checkObj = dynamic_cast<TObject *>(*objRef);
                    if (!checkObj) {
                        Error("Init",
                              "Branch '%s': *objRef is not a TObject. Type mismatch?",
                              br->GetName());
                        continue;
                    }
                }

                fTree->SetBranchAddress(br->GetName(), objRef);

                useBranch.emplace_back(br);

                // If it's TEventHeader, store pointer in fEventHeader
                if (cl == TEventHeader::Class()) {
                    fEventHeader = static_cast<art::TEventHeader *>(obj);
                }
                Info("Init", "Branch: %s (class=%s)", br->GetName(), cl->GetName());
            }
        }
    }

    // Add user info objects to the collection
    TIter nextInfo(fTree->GetUserInfo());
    while (auto *infoObj = nextInfo()) {
        col->AddInfo(infoObj->GetName(), infoObj, kTRUE);
    }

    // Enable only branches we decided to use
    fTree->SetBranchStatus("*", 0);
    for (auto *br : useBranch) {
        fTree->SetBranchStatus(br->GetName(), 1);
    }

    // Load the first entry and get total entries
    fTree->LoadTree(0);
    fTree->GetEntry(0);
    fTreeEventNum = fTree->GetEntries();
}

/**
 * @details
 * Process method: called for each event
 *  - Load entry from fTree
 *  - Increment counters
 *  - Wrap around if needed
 *  - Stop if reaching fMaxEventNum
 */
void TTreePeriodicEventStore::Process() {
    fTree->GetEntry(fCurrentNum);
    ++fCurrentNum;
    ++fEventNum;

    // Wrap around if we reach the end of the tree
    if (fEventNum == fTreeEventNum) {
        fCurrentNum = 0;
    }
    // Stop if we reached the maximum events
    if (fMaxEventNum == fEventNum) {
        SetStopLoop();
        SetEndOfRun();
    }
}

Int_t TTreePeriodicEventStore::GetRunNumber() const {
    return (fEventHeader) ? fEventHeader->GetRunNumber() : 0;
}

const char *TTreePeriodicEventStore::GetRunName() const {
    return "";
}

std::string TTreePeriodicEventStore::GetStrRunName() const {
    return (fEventHeader) ? std::string(fEventHeader->GetRunName()) : "";
}

} // namespace art::crib
