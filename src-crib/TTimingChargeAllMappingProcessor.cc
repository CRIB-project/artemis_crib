/**
 * @file    TTimingChargeAllMappingProcessor.cc
 * @brief   from TTimingChargeMappingProcessor, both E and T
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022?
 * @note    last modified: 2024-08-23 20:58:00
 * @details
 */

#include "TTimingChargeAllMappingProcessor.h"

#include "TTimingChargeData.h"
#include "constant.h"
#include <TCategorizedData.h>
#include <TRawDataTimingCharge.h>
#include <TRawTimingWithEdge.h>

using art::crib::TTimingChargeAllMappingProcessor;

ClassImp(TTimingChargeAllMappingProcessor);

// Default constructor
TTimingChargeAllMappingProcessor::TTimingChargeAllMappingProcessor()
    : fPlastic(nullptr) {
    RegisterInputCollection("InputCollection", "rawdata object returned by TRIDFEventStore",
                            fInputColName, TString("catdata"),
                            &fCategorizedData, "art::TCategorizedData");
    RegisterOutputCollection("OutputCollection", "output collection name",
                             fOutputColName, TString("plastic_raw"),
                             &fPlastic, "TClonesArray", "art::TTimingChargeData");
    RegisterProcessorParameter("CatID", "Category ID of plastic", fCatID, 21);
    RegisterProcessorParameter("ChargeType", "0: V1190 width (default), 1: QDC (V792 etc.), 2: paired time and charge (hinp)",
                               fChargeType, 0);
    RegisterProcessorParameter("ChargeTypeID", "data type id for charge information (valid when ChargeType != (0 or 2) , default: 2)",
                               fChargeTypeID, 2);
    RegisterProcessorParameter("TimingTypeID", "data type id for timing information (default: 1)",
                               fTimingTypeID, 1);
    RegisterProcessorParameter("TrailingComesFirst", "0: Leading->Trailing (default), 1: Trailing->Leading (for QTC etc.)",
                               fTrailingComesFirst, kFALSE);
    RegisterProcessorParameter("Sparse", "hit will be inserted at the index corresponding to its id if sparse is 0, otherwize hit will be added ordinaly (default: 1)",
                               fIsSparse, kTRUE);
}

TTimingChargeAllMappingProcessor::~TTimingChargeAllMappingProcessor() {
    delete fPlastic;
}

void TTimingChargeAllMappingProcessor::Init(TEventCollection *) {
    const TString &chargeTypeStr =
        TString::Format("ChargeType: %d", fChargeType) +
        (fChargeType ? TString::Format(", ChargeTypeID: %d", fChargeTypeID) : "");

    Info("Init", "CatID: %d, TimingTypeID: %d => %s",
         fCatID, fTimingTypeID, fOutputColName.Data());
    Info("Init", "%s", chargeTypeStr.Data());
    Info("Init", "TrailingComesFirst: %d", fTrailingComesFirst);

    if (fIsSparse) {
        Info("Init", "This processor treat only not sparse data");
        fIsSparse = 0;
    };
}

void TTimingChargeAllMappingProcessor::Process() {
    fPlastic->Clear("C");

    const TObjArray *const cat = (*fCategorizedData)->FindCategory(fCatID);
    if (!cat)
        return;

    for (Int_t i = 0, n = cat->GetEntriesFast(); i != n; ++i) {
        const TObjArray *const det = static_cast<TObjArray *>(cat->At(i));
        const TObjArray *const tArray = static_cast<TObjArray *>(det->At(fTimingTypeID));
        // if (!tArray || tArray->IsEmpty()) continue;
        if (fChargeType == kWIDTH) {
            MapEdgedTime(tArray);
        } else if (fChargeType == kHINP) {
            MapPairedTimeCharge(tArray);
        } else /* if (fChargeType == kQDC) */ {
            const TObjArray *const qArray = static_cast<TObjArray *>(det->At(fChargeTypeID));
            MapTimeCharge(tArray, qArray);
        }
    }

    if (fIsSparse) {
        // sort data in the same event in ascending order of timing
        TTimingChargeData::SetSortType(TTimingChargeData::kTiming);
        TTimingChargeData::SetSortOrder(TTimingChargeData::kASC);
        fPlastic->Sort();
        fPlastic->Compress();
    } else {
        for (Int_t i = 0, n = fPlastic->GetEntriesFast(); i != n; ++i) {
            fPlastic->ConstructedAt(i);
        }
    }
}

void TTimingChargeAllMappingProcessor::MapEdgedTime(const TObjArray *tArray) {
    if (!tArray || tArray->IsEmpty())
        return;

    TTimingChargeData *data = nullptr;
    for (Int_t iHit = 0, nHit = tArray->GetEntriesFast(); iHit != nHit; ++iHit) {
        const TRawTimingWithEdge *const hit = static_cast<TRawTimingWithEdge *>(tArray->At(iHit));
        //      if (!hit) continue;

        if (hit->IsLeading() != fTrailingComesFirst) {
            // "Leading" edge
            const Int_t detID = hit->GetDetID();
            const Int_t idx = fIsSparse ? fPlastic->GetEntriesFast() : detID;
            data = static_cast<TTimingChargeData *>(fPlastic->ConstructedAt(idx));
            if (IsValid(data->GetDetID()))
                continue;

            data->SetDetID(detID);
            const Double_t tLeading = hit->GetTiming();
            data->SetTiming(tLeading);
        } else if (data) {
            // "Trailing" edge after "Leading" one
            const Double_t tLeading = data->GetTiming();
            const Double_t tTrailing = hit->GetTiming();
            const Double_t charge = tTrailing - tLeading;

            data->SetCharge(charge);
            data = nullptr;
        } else {
            // consecutive "Trailing" edge
            // thought to be abnormal
        }
    }
}

void TTimingChargeAllMappingProcessor::MapTimeCharge(const TObjArray *tArray, const TObjArray *qArray) {
    if ((!tArray || tArray->IsEmpty()) && (!qArray || qArray->IsEmpty()))
        return;

    TTimingChargeData *data;

    Bool_t data_flag = false;
    if (tArray && !(tArray->IsEmpty())) {
        const TRawDataObject *const tHit = static_cast<TRawDataObject *>(tArray->At(0));

        const Int_t detID = tHit->GetDetID();
        if (IsValid(detID)) {
            const Int_t idx = fIsSparse ? fPlastic->GetEntriesFast() : detID;
            data = static_cast<TTimingChargeData *>(fPlastic->ConstructedAt(idx));

            if (IsValid(data->GetDetID()))
                return; // take only the first hit
            data->SetDetID(detID);
            data->SetTiming(tHit->GetValue());

            data_flag = true;
        }
    }
    if (qArray && !(qArray->IsEmpty())) {
        const TRawTiming *const qHit = static_cast<TRawTiming *>(qArray->At(0));
        if (!data_flag) {
            const Int_t detID = qHit->GetDetID();
            const Int_t idx = fIsSparse ? fPlastic->GetEntriesFast() : detID;
            data = static_cast<TTimingChargeData *>(fPlastic->ConstructedAt(idx));

            if (IsValid(data->GetDetID()))
                return; // take only the first hit
            data->SetDetID(detID);
            data->SetCharge(qHit->GetValue());

            data_flag = true;
        } else {
            data->SetCharge(qHit->GetValue());
        }
    }
}

void TTimingChargeAllMappingProcessor::MapPairedTimeCharge(const TObjArray *tArray) {
    if (!tArray || tArray->IsEmpty())
        return;

    TTimingChargeData *data = nullptr;
    for (Int_t iHit = 0, nHit = tArray->GetEntriesFast(); iHit != nHit; ++iHit) {
        const TRawDataTimingCharge *const hit = static_cast<TRawDataTimingCharge *>(tArray->At(iHit));

        const Int_t detID = hit->GetDetID();
        const Int_t idx = fIsSparse ? fPlastic->GetEntriesFast() : detID;
        data = static_cast<TTimingChargeData *>(fPlastic->ConstructedAt(idx));
        if (IsValid(data->GetDetID()))
            continue;

        data->SetDetID(detID);
        const Double_t timing = hit->GetTiming();
        const Double_t charge = hit->GetCharge();
        data->SetTiming(timing);
        data->SetCharge(charge);
    }
}
