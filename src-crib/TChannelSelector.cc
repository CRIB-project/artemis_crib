/**
 * @file    TChannelSelector.cc
 * @brief   extract one channel data
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-12-18 15:41:32
 * @note    last modified: 2024-12-20 16:15:39
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
    fSegmentedData = nullptr;
    delete fOutData;
    fOutData = nullptr;
}

void TChannelSelector::Init(TEventCollection *col) {
    // segdata process
    auto seg_ref = col->GetObjectRef(fSegmentedDataName);
    if (!seg_ref) {
        SetStateError(Form("No such input collection '%s'\n", fSegmentedDataName.Data()));
        return;
    }

    auto *seg_obj = static_cast<TObject *>(*seg_ref);
    if (!seg_obj->InheritsFrom("art::TSegmentedData")) {
        SetStateError(Form("'%s' is not of type art::TSegmentedData\n", fSegmentedDataName.Data()));
        return;
    }

    fSegmentedData = static_cast<TSegmentedData *>(seg_obj);

    // segid process
    if (fSegID.size() != 5) {
        SetStateError("parameter: SegID size is not 5, input [dev, fp, mod, geo, ch]\n");
        return;
    }
    Info("Init", "Process [dev=%d, fp=%d, mod=%d, geo=%d, ch=%d]", fSegID[0], fSegID[1], fSegID[2], fSegID[3], fSegID[4]);

    fOutData = new TClonesArray("art::TSimpleData");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);
    Info("Init", "%s -> %s", fSegmentedDataName.Data(), fOutputColName.Data());
}

void TChannelSelector::Process() {
    fOutData->Clear("C");

    auto *seg_array = fSegmentedData->FindSegment(fSegID[0], fSegID[1], fSegID[2]);
    if (!seg_array) {
        Warning("Process", "No segment having segid = [dev=%d, fp=%d, mod=%d]", fSegID[0], fSegID[1], fSegID[2]);
        return;
    }

    auto nData = seg_array->GetEntriesFast();
    auto counter = 0;
    for (auto iData = 0; iData < nData; iData++) {
        auto *data = (TRawDataObject *)seg_array->UncheckedAt(iData);
        auto geo = data->GetGeo();
        auto ch = data->GetCh();
        if (geo == fSegID[3] && ch == fSegID[4]) {
            auto *outData = static_cast<art::TSimpleData *>(fOutData->ConstructedAt(counter));
            counter++;
            outData->SetValue(data->GetValue());
        }
    }
}

} // namespace art::crib
