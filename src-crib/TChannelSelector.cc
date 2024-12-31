/**
 * @file    TChannelSelector.cc
 * @brief   extract one channel data
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-12-18 15:41:32
 * @note    last modified: 2024-12-31 17:28:59
 * @details
 */

#include "TChannelSelector.h"

#include <TRawDataObject.h>
#include <TSegmentedData.h>
#include <TSimpleData.h>

ClassImp(art::crib::TChannelSelector);

namespace art::crib {
TChannelSelector::TChannelSelector() : fSegmentedData(nullptr), fOutData(nullptr) {
    RegisterInputCollection("SegmentedDataName", "name of the segmented data",
                            fSegmentedDataName, TString("segdata"));
    RegisterOutputCollection("OutputCollection", "name of the output branch",
                             fOutputColName, TString("channel"));

    IntVec_t init_i_vec;
    RegisterProcessorParameter("SegID", "segment ID, [dev, fp, mod, geo, ch]",
                               fSegID, init_i_vec);
}

TChannelSelector::~TChannelSelector() {
    delete fOutData;
    fOutData = nullptr;
}

void TChannelSelector::Init(TEventCollection *col) {
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

    delete fOutData; // Release memory allocated for fOutData if it exists
    fOutData = new TClonesArray("art::TSimpleData");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);
    Info("Init", "%s -> %s, [dev=%d, fp=%d, mod=%d, geo=%d, ch=%d]",
         fSegmentedDataName.Data(), fOutputColName.Data(),
         fSegID[0], fSegID[1], fSegID[2], fSegID[3], fSegID[4]);
}

void TChannelSelector::Process() {
    fOutData->Clear("C");
    if (!fSegmentedData) {
        Warning("Process", "No SegmentedData object");
        return;
    }

    auto *seg_array = fSegmentedData->FindSegment(fSegID[0], fSegID[1], fSegID[2]);
    if (!seg_array) {
        Warning("Process", "No segment having segid = [dev=%d, fp=%d, mod=%d]",
                fSegID[0], fSegID[1], fSegID[2]);
        return;
    }

    const int nData = seg_array->GetEntriesFast();
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
