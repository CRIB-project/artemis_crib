/**
 * @file    TF1PPACProcessor.cc
 * @brief   Created from TPPACPrcessor
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-08-08 17:25:02
 * @note    last modified: 2024-08-23 20:50:50
 * @details
 */

#include "TF1PPACProcessor.h"

#include "TConverterBase.h"
#include "TConverterUtil.h"
#include "TPPACData.h"
#include "TPPACParameter.h"

#include <TCategorizedData.h>
#include <TRawDataObject.h>

using art::crib::TF1PPACProcessor;

ClassImp(TF1PPACProcessor);

// Default constructor
TF1PPACProcessor::TF1PPACProcessor()
    : fPPACOut(nullptr),
      fTimingConverterArray(nullptr), fChargeConverterArray(nullptr),
      fXConverterArray(nullptr), fYConverterArray(nullptr) {
    StringVec_t defInput(1, "catdata");
    RegisterInputCollection("InputCollection", "rawdata object returned by TRIDFEventStore",
                            fInputColName, defInput);
    RegisterOutputCollection("OutputCollection", "output collection name",
                             fOutputColName, TString("ppac"));
    RegisterProcessorParameter("CatID", "Category ID of PPAC", fCatID, 1);
    RegisterProcessorParameter("TimingConverterArray",
                               "name of time converter array (ch2ns)",
                               fTimingConverterArrayName, TString("ppac_ch2ns"));
    RegisterProcessorParameter("ChargeConverterArray", "name of charge converter array (qped)",
                               fChargeConverterArrayName, TString("ppac_qped"));
    RegisterProcessorParameter("XConverterArray", "name of x position converter array (ns2mm)",
                               fXConverterArrayName, TString("ppac_xconv"));
    RegisterProcessorParameter("YConverterArray", "name of y position converter array (ns2mm)",
                               fYConverterArrayName, TString("ppac_yconv"));
    RegisterOptionalInputInfo("PPACParameter", "name of ppac parameter", fParameterName,
                              TString("prm:ppac"), &fParameterSet, "TClonesArray", "art::TPPACParameter");
    defInput.clear();
    RegisterOptionalParameter("ListOfParameterNames", "list of names of parameter",
                              fListOfParameterNames, defInput);
    RegisterOptionalParameter("ListOfPPACNames",
                              "list of names of ppac: the size should be same as the maximum detector id.\n"
                              "Index corresponds to the id of PPAC.\n"
                              "If name at certain index is empty the output of correnponding ppac will be ommitted.",
                              fListOfPPACNames, defInput);
    RegisterOptionalParameter("DoSeparatePPACs", "separate output for each ppacs",
                              fDoSeparatePPACs, kFALSE);
    RegisterOptionalParameter("HasEachChConverter", "converter should be prepared for each channel",
                              fHasEachChConverter, kFALSE);
    RegisterOptionalParameter("doInsideOffset", "calibrate delayline offset if non 0", fDoInsideOffset, kTRUE);
    RegisterOptionalParameter("doOutsideOffset", "calibrate line offset if non 0", fDoOutsideOffset, kTRUE);
    RegisterOptionalParameter("doGeometryOffset", "calibrate geometry offset if non 0", fDoGeometryOffset, kTRUE);
}

TF1PPACProcessor::~TF1PPACProcessor() {
    delete fPPACOut;
    delete fTimingConverterArray;
    delete fChargeConverterArray;
    delete fXConverterArray;
    delete fYConverterArray;
}

void TF1PPACProcessor::Init(TEventCollection *col) {

    fCategorizedData = reinterpret_cast<TCategorizedData **>(col->GetObjectRef(fInputColName[0].Data()));
    if (!fCategorizedData)
        return;

    fPPACOut = new TClonesArray(TPPACData::Class());
    fPPACOut->SetName(fOutputColName);
    col->Add(fOutputColName, fPPACOut, fOutputIsTransparent);

    Info("Init", "CatID: %d  => %s\n", fCatID, fOutputColName.Data());

    TConverterUtil::SetConverterArray(&fTimingConverterArray, fTimingConverterArrayName, col);
    TConverterUtil::SetConverterArray(&fChargeConverterArray, fChargeConverterArrayName, col);
    TConverterUtil::SetConverterArray(&fXConverterArray, fXConverterArrayName, col);
    TConverterUtil::SetConverterArray(&fYConverterArray, fYConverterArrayName, col);

    if (fDoSeparatePPACs) {
        const Int_t &nPPACs = fListOfPPACNames.size();
        const Int_t &nParams = fListOfParameterNames.size();
        if (!nPPACs) {
            Warning("Init", "Cannot separate PPACs since no name is set while DoSeparatePPACs is true");
            return;
        }
        if (!nParams) {
            std::copy(fListOfPPACNames.begin(), fListOfPPACNames.end(), std::back_inserter(fListOfParameterNames));
        } else if (nParams != nPPACs) {
            SetStateError("The numbers of parameters and PPACs are different, while trying to separate PPAC output");
            return;
        }
        fPPACArray.resize(nPPACs, nullptr);
        fPPACParameter.resize(nPPACs, nullptr);
        for (Int_t i = 0; i != nPPACs; i++) {
            if (fListOfPPACNames[i].IsNull())
                continue;
            fPPACArray[i] = new TClonesArray(TPPACData::Class());
            col->Add(fListOfPPACNames[i], fPPACArray[i], fOutputIsTransparent);
            TPPACParameter *prm = dynamic_cast<TPPACParameter *>((*fParameterSet)->FindObject(fListOfParameterNames[i]));
            if (!prm) {
                SetStateError(TString::Format("No such parameter '%s' is found,while trying to separate PPAC output",
                                              fListOfParameterNames[i].Data()));
                return;
            }
            fPPACParameter[i] = prm;
        }
    }
}

void TF1PPACProcessor::Process() {
    if (!fCategorizedData)
        return;

    fPPACOut->Clear("C");
    if (fDoSeparatePPACs) {
        for (std::vector<TClonesArray *>::iterator it = fPPACArray.begin(); it != fPPACArray.end(); it++) {
            (*it)->Clear("C");
        }
    }

    const TObjArray *const cat = (*fCategorizedData)->FindCategory(fCatID);
    if (!cat)
        return;
    Int_t t[TPPACData::kNRAW], q[TPPACData::kNRAW];

    const Int_t n = cat->GetEntriesFast();
    for (Int_t i = 0; i != n; ++i) {
        const TObjArray *const det = static_cast<TObjArray *>(cat->At(i));
        Bool_t idFlag = kFALSE;
        Int_t detID = kInvalidI;

        for (Int_t j = 0; j != TPPACData::kNRAW; ++j) {

            t[j] = q[j] = kInvalidI;

            const TObjArray *const tArray = static_cast<TObjArray *>(det->At(j));
            const TObjArray *const qArray = static_cast<TObjArray *>(det->At(j + TPPACData::kNRAW));

            if (tArray && tArray->GetEntriesFast()) {
                TRawDataObject *const tHit = static_cast<TRawDataObject *>(tArray->At(0));
                if (!tHit)
                    continue;
                t[j] = tHit->GetValue();
                if (!idFlag) {
                    detID = tHit->GetDetID();
                    idFlag = IsValid(detID);
                }
            }

            if (qArray && qArray->GetEntriesFast()) {
                TRawDataObject *const qHit = static_cast<TRawDataObject *>(qArray->At(0));
                if (qHit)
                    q[j] = qHit->GetValue();
            }
        }

        if (!idFlag)
            continue;

        TPPACData *const ppac = static_cast<TPPACData *>(fPPACOut->ConstructedAt(fPPACOut->GetEntriesFast()));
        ppac->SetDetID(detID);
        for (Int_t j = 0; j != TPPACData::kNRAW; ++j) {
            if (IsValid(t[j])) {
                if (fTimingConverterArray) {
                    TConverterBase *conv = nullptr;
                    if (fHasEachChConverter) {
                        conv = fTimingConverterArray->at(detID * TPPACData::kNRAW + j);
                    } else {
                        conv = fTimingConverterArray->at(detID);
                    }
                    if (conv) {
                        ppac->SetT(conv->Convert(t[j]), j);
                    } else {
                        ppac->SetT(t[j], j);
                    }
                } else {
                    ppac->SetT(t[j], j);
                }
            }

            if (IsValid(q[j])) {
                if (fChargeConverterArray) {
                    TConverterBase *conv = nullptr;
                    if (fHasEachChConverter) {
                        conv = fChargeConverterArray->at(detID * TPPACData::kNRAW + j);
                    } else {
                        conv = fChargeConverterArray->at(detID);
                    }
                    ppac->SetQ(conv->Convert(q[j]), j);
                } else {
                    ppac->SetQ(q[j], j);
                }
            }
        }
        if (fXConverterArray) {
            ppac->SetXConverter(fXConverterArray->at(detID - DETID_ORIGIN));
        }
        if (fYConverterArray) {
            ppac->SetYConverter(fYConverterArray->at(detID - DETID_ORIGIN));
        }
        ppac->Update();

        if (fDoSeparatePPACs && detID < (Int_t)fPPACArray.size()) {
            Double_t X, Y;
            TClonesArray *arr = fPPACArray[detID];
            TPPACData *ppacdata = (TPPACData *)arr->ConstructedAt(arr->GetEntriesFast());
            TPPACParameter *prm = (TPPACParameter *)fPPACParameter[detID];
            ppac->Copy(*ppacdata);
            Double_t dTX = (ppac->GetTX1() - ppac->GetTX2()) / (ppac->GetTX1() + ppac->GetTX2());
            Double_t dTY = (ppac->GetTY1() - ppac->GetTY2()) / (ppac->GetTY1() + ppac->GetTY2());

            // if (delaylinecalibrated)
            if (fDoInsideOffset) {
                dTX += prm->GetInsideOffset(0);
                dTY += prm->GetInsideOffset(1);
            }

            // if (line calib ofset)
            if (fDoOutsideOffset) {
                dTX -= prm->GetOutsideOffset(0);
                dTY -= prm->GetOutsideOffset(1);
            }

            ppacdata->SetTXDiff(dTX);
            ppacdata->SetTYDiff(dTY);
            // exchange and calibration
            X = dTX * prm->GetNs2mm(0) * 0.5;
            Y = dTY * prm->GetNs2mm(1) * 0.5;
            if (prm->GetExchangeXY()) {
                Double_t tmp = X;
                X = Y;
                Y = tmp;
            }
            if (prm->GetReflection()) {
                X *= -1;
            }

            // geometry offset
            if (fDoGeometryOffset) {
                X -= prm->GetGeometryOffset(0);
                Y -= prm->GetGeometryOffset(1);
            }
            ppacdata->ResetQualityBit(TDataObject::kInvalid);
            if (!prm->IsInsideTXSum(ppacdata->GetTXSum())) {
                ppacdata->SetQualityBit(TPPACData::kBadTXSum | TDataObject::kInvalid);
            }
            if (!prm->IsInsideTYSum(ppacdata->GetTYSum())) {
                ppacdata->SetQualityBit(TPPACData::kBadTYSum | TDataObject::kInvalid);
            }
            ppacdata->SetX(X);
            ppacdata->SetY(Y);
            ppacdata->SetZ(prm->GetGeometryOffset(2));
            ppacdata->Copy(*ppac);
        }
    }
}
