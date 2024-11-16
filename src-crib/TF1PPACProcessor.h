/**
 * @file    TF1PPACProcessor.h
 * @brief   Created from TPPACProcessor
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-08-08 17:27:59
 * @note    last modified: 2024-08-23 20:51:28
 * @details
 */

#ifndef _CRIB_TF1PPACPROCESSOR_H
#define _CRIB_TF1PPACPROCESSOR_H

#include <TProcessor.h>

namespace art {
class TCategorizedData;
class TConverterBase;
class TPPACParameter;
} // namespace art

namespace art::crib {
class TF1PPACProcessor;
} // namespace art::crib

class TClonesArray;

class art::crib::TF1PPACProcessor : public TProcessor {

  public:
    // Default constructor
    TF1PPACProcessor();
    ~TF1PPACProcessor();

    void Init(TEventCollection *col) override;
    void Process() override;

  protected:
    StringVec_t fInputColName;
    TString fOutputColName;
    TCategorizedData **fCategorizedData; //!
    TClonesArray *fPPACOut;

    Int_t fCatID; // default: 1 (ppac in anapaw)

    static const Int_t DETID_ORIGIN = 0;

    TString fTimingConverterArrayName;
    TString fChargeConverterArrayName;
    TString fXConverterArrayName;
    TString fYConverterArrayName;

    std::vector<TConverterBase *> *fTimingConverterArray; //!
    std::vector<TConverterBase *> *fChargeConverterArray; //!
    std::vector<TConverterBase *> *fXConverterArray;      //!
    std::vector<TConverterBase *> *fYConverterArray;      //!

    // using ppac parameter set
    TString fParameterName;                       // name of parameter set
    TClonesArray **fParameterSet;                 //! parameter set
    StringVec_t fListOfParameterNames;            // list of parameter name for each ppac
    std::vector<TPPACParameter *> fPPACParameter; //! list of parmaeter for each ppac
    // output separately
    StringVec_t fListOfPPACNames;           // list of ppac name for each ppac
    std::vector<TClonesArray *> fPPACArray; //! list of ppacs
    Bool_t fDoSeparatePPACs;                // flag if separte the output of PPACs
    Bool_t fHasEachChConverter;             // flag if the converter for each channel is prepared
    Bool_t fDoInsideOffset;                 // flag to calibrate inside offset
    Bool_t fDoOutsideOffset;                // flag to calibrate outside offset
    Bool_t fDoGeometryOffset;               // flag to calibrate geometry offset

  private:
    // Copy constructor (prohibited)
    TF1PPACProcessor(const TF1PPACProcessor &rhs) = delete;
    // Assignment operator (prohibited)
    TF1PPACProcessor &operator=(const TF1PPACProcessor &rhs) = delete;

    ClassDefOverride(TF1PPACProcessor, 0) // processor for PPAC analysis
};

#endif // TF1PPACPROCESSOR_H
