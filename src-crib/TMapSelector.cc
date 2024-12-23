/**
 * @file    TMapSelector.cc
 * @brief   extract one catid data
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-12-23 11:56:21
 * @note    last modified: 2024-12-23 11:58:48
 * @details
 */

#include "TMapSelector.h"

#include <TRawDataObject.h>
#include <TSegmentedData.h>
#include <TSimpleData.h>

ClassImp(art::crib::TMapSelector);

namespace art::crib {
TMapSelector::TMapSelector() : fSegmentedData(nullptr), fOutData(nullptr) {
    RegisterInputCollection("SegmentedDataName", "name of the segmented data",
                            fSegmentedDataName, TString("segdata"));
    RegisterOutputCollection("OutputCollection", "name of the output branch",
                             fOutputColName, TString("channel"));

    IntVec_t init_i_vec;
    RegisterProcessorParameter("SegID", "segment ID, [dev, fp, mod, geo, ch]",
                               fSegID, init_i_vec);
}

TMapSelector::~TMapSelector() {
    // release fOutData in TEventCollection
}

void TMapSelector::Init(TEventCollection *col) {
    // Segmented data initialization
    auto seg_ref = col->GetObjectRef(fSegmentedDataName);
    if (!seg_ref) {
        SetStateError(Form("No input collection '%s'", fSegmentedDataName.Data()));
        return;
    }

    auto seg_obj = static_cast<TObject *>(*seg_ref);
    if (!seg_obj->InheritsFrom("art::TSegmentedData")) {
        SetStateError(Form("Invalid input collection '%s': not TSegmentedData",
                           fSegmentedDataName.Data()));
        return;
    }
    fSegmentedData = static_cast<TSegmentedData *>(seg_obj);

    // SegID validation
    if (fSegID.size() != 5) {
        SetStateError("SegID must contain exactly 5 elements: [dev, fp, mod, geo, ch]");
        return;
    }

    fOutData = new TClonesArray("art::TSimpleData");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);
    Info("Init", "%s -> %s, [dev=%d, fp=%d, mod=%d, geo=%d, ch=%d]",
         fSegmentedDataName.Data(), fOutputColName.Data(),
         fSegID[0], fSegID[1], fSegID[2], fSegID[3], fSegID[4]);
}

void TMapSelector::Process() {
    fOutData->Clear("C");

    auto *seg_array = fSegmentedData->FindSegment(fSegID[0], fSegID[1], fSegID[2]);
    if (!seg_array) {
        Warning("Process", "No segment having segid = [dev=%d, fp=%d, mod=%d]",
                fSegID[0], fSegID[1], fSegID[2]);
        return;
    }

    const auto nData = seg_array->GetEntriesFast();
    int counter = 0;
    for (int iData = 0; iData < nData; ++iData) {
        auto *data = dynamic_cast<TRawDataObject *>(seg_array->At(iData));
        if (data && data->GetGeo() == fSegID[3] && data->GetCh() == fSegID[4]) {
            auto *outData = static_cast<art::TSimpleData *>(fOutData->ConstructedAt(counter));
            counter++;
            outData->SetValue(data->GetValue());
        }
    }
}

} // namespace art::crib
