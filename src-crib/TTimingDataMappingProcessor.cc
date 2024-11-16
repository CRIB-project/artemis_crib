/**
 * @file    TTimingDataMappingProcessor.cc
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-07-19 18:36:37
 * @note    last modified: 2024-08-23 20:58:57
 * @details
 */

#include "TTimingDataMappingProcessor.h"

#include "constant.h"
#include <TCategorizedData.h>
#include <TRawDataObject.h>
#include <TTimingData.h>

using art::crib::TTimingDataMappingProcessor;

ClassImp(TTimingDataMappingProcessor);

TTimingDataMappingProcessor::TTimingDataMappingProcessor()
    : fOutputArray(nullptr) {
    RegisterInputCollection("InputCollection", "rawdata object returned by TRIDFEventStore",
                            fInputColName, TString("catdata"),
                            reinterpret_cast<void ***>(&fCategorizedData),
                            "art::TCategorizedData");
    RegisterOutputCollection("OutputCollection", "output collection name",
                             fOutputColName, TString("simple"),
                             reinterpret_cast<void **>(&fOutputArray),
                             "TClonesArray", "art::TTimingData");
    RegisterProcessorParameter("CatID", "Category ID", fCatID, 0);
    RegisterProcessorParameter("DataTypeID", "data type id for input",
                               fDataTypeID, 0);
    //  RegisterProcessorParameter("OutputType","not yet implemented (always output TTimingData)"
    //			     fOutputType,2);
    RegisterProcessorParameter("Sparse", "hit will be inserted at the index corresponding to its id if sparse is 0, otherwise added ordinaly", fIsSparse, kTRUE);
}

TTimingDataMappingProcessor::~TTimingDataMappingProcessor() {
    delete fOutputArray;
    fOutputArray = nullptr;
}

void TTimingDataMappingProcessor::Init(TEventCollection *) {
    Info("Init", "CatID: %d, DataTypeID: %d => %s",
         fCatID, fDataTypeID, fOutputColName.Data());
    if (!fIsSparse)
        Info("Init", "Not sparse.");
}

void TTimingDataMappingProcessor::Process() {
    if (!fCategorizedData)
        return;

    fOutputArray->Clear("C");

    const TObjArray *const cat = (*fCategorizedData)->FindCategory(fCatID);
    if (!cat)
        return;

    const Int_t n = cat->GetEntriesFast();

    for (Int_t i = 0; i != n; ++i) {
        const TObjArray *const det = static_cast<TObjArray *>(cat->At(i));
        const TObjArray *const dataArray = static_cast<TObjArray *>(det->At(fDataTypeID));
        if (!dataArray)
            continue;

        const TRawDataObject *const hit = dynamic_cast<TRawDataObject *>(dataArray->At(0));
        const Int_t detID = hit->GetDetID();
        const Int_t idx = fIsSparse ? fOutputArray->GetEntriesFast() : detID;
        TTimingData *const data = static_cast<TTimingData *>(fOutputArray->ConstructedAt(idx));
        if (IsValid(data->GetID()))
            return; // take only the first hit if not sparse

        data->SetID(detID);
        data->SetTiming(hit->GetValue());
    }

    if (fIsSparse) {
        TTimingData::SetSortType(TTimingData::kTiming);
        TTimingData::SetSortOrder(TTimingData::kASC);
        fOutputArray->UnSort();
        fOutputArray->Sort();
        fOutputArray->Compress();
    } else {
        const Int_t n = fOutputArray->GetEntriesFast();
        for (Int_t i = 0; i != n; ++i) {
            fOutputArray->ConstructedAt(i);
        }
    }
}
