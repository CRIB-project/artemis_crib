/**
 * @file    TSegmentOutputProcessor.h
 * @brief   from seg conf, output raw data TTree object
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-12-25 11:40:53
 * @note    last modified: 2024-08-23 20:57:10
 * @details
 */

#ifndef _CRIB_TSEGNEMTOUTPUTPROCESSOR_H_
#define _CRIB_TSEGNEMTOUTPUTPROCESSOR_H_

#include "TProcessor.h"
#include <TFile.h>

namespace art {
class TSegmentInfo;
class TSegmentedData;
class TModuleInfo;
} // namespace art

namespace art::crib {
class TSegmentOutputProcessor;
class TModuleData;
} // namespace art::crib

class TClonesArray;

class art::crib::TSegmentOutputProcessor : public TProcessor {
  public:
    TSegmentOutputProcessor();
    ~TSegmentOutputProcessor();

    TSegmentOutputProcessor(const TSegmentOutputProcessor &);
    TSegmentOutputProcessor &operator=(const TSegmentOutputProcessor &rhs);

    void Init(TEventCollection *) override;
    void Process() override;
    void PreLoop() override;
    void PostLoop() override;

  protected:
    TString fFileName;
    TString fTreeName;
    TFile *fFile; //! outputed file
    TTree *fTree; //! pure TTree object (not TArtTree)
    TString fSegmentListName;
    TString fModuleListName;
    TClonesArray **fSegmentList; //!
    TClonesArray **fModuleList;  //!

    TSegmentedData **fSegmentedData; //!
    TString fSegmentedDataName;
    StringVec_t fIgnore;                                   //! list of ignored segment
    std::map<Int_t, std::vector<TModuleData *>> fSegments; //!

  private:
    ClassDefOverride(TSegmentOutputProcessor, 1) // segment checking processor
};

#endif // end of #ifndef _TSEGMENTOUTPUTPROCESSOR_H_
