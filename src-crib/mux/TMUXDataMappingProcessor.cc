/**
 * @file    TMUXDataMappingProcessor.cc
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 09:47:17
 * @note    last modified: 2024-08-23 21:30:10
 * @details
 */

#include "TMUXDataMappingProcessor.h"

#include "TMUXData.h"
#include <TCategorizedData.h>
#include <TRawDataObject.h>

using art::crib::TMUXDataMappingProcessor;

ClassImp(TMUXDataMappingProcessor);

TMUXDataMappingProcessor::TMUXDataMappingProcessor()
    : fOutputArray(nullptr) {
    RegisterInputCollection("InputCollection", "rawdata object returned by TRIDFEventStore",
                            fInputColName, TString("catdata"),
                            reinterpret_cast<void ***>(&fCategorizedData),
                            "art::TCategorizedData");
    RegisterOutputCollection("OutputCollection", "output collection name",
                             fOutputColName, TString("simple"),
                             reinterpret_cast<void **>(&fOutputArray),
                             "TClonesArray", "art::crib::TMUXData");
    RegisterProcessorParameter("CatID", "Category ID", fCatID, 0);
}

TMUXDataMappingProcessor::~TMUXDataMappingProcessor() {
    delete fOutputArray;
    fOutputArray = nullptr;
}

void TMUXDataMappingProcessor::Init(TEventCollection *) {
    Info("Init", "CatID: %d => %s", fCatID, fOutputColName.Data());
}

void TMUXDataMappingProcessor::Process() {
    if (!fCategorizedData)
        return;

    fOutputArray->Clear("C");

    const TObjArray *const cat = (*fCategorizedData)->FindCategory(fCatID);
    if (!cat)
        return;

    Double_t m[TMUXData::kNRAW];
    const Int_t n = cat->GetEntriesFast(); // should be 1

    for (Int_t i = 0; i != n; ++i) {
        const TObjArray *const det = static_cast<TObjArray *>(cat->At(i));
        // std::cout << "debug: find " << det->GetEntriesFast() << " contents" << std::endl; //debug

        Int_t detID = kInvalidI;

        for (Int_t j = 0; j != TMUXData::kNRAW; ++j) {
            m[j] = kInvalidD;

            const TObjArray *const dataArray = static_cast<TObjArray *>(det->At(j));

            if (dataArray && dataArray->GetEntriesFast()) {
                const TRawDataObject *const hit = dynamic_cast<TRawDataObject *>(dataArray->At(0));
                if (!hit)
                    continue;

                m[j] = hit->GetValue();
                detID = hit->GetDetID();
            }
        }

        TMUXData *const data = static_cast<TMUXData *>(fOutputArray->ConstructedAt(fOutputArray->GetEntriesFast()));
        data->TDataObject::SetID(detID);

        data->SetE1(m[0]);
        data->SetE2(m[1]);
        data->SetP1(m[2]);
        data->SetP2(m[3]);
        data->SetTrig(m[4]);
    }
}
