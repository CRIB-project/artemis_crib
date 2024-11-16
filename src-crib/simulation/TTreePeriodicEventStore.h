/**
 * @file    TTreePeriodicEventStore.h
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-07-16 15:16:56
 * @note    last modified: 2024-08-23 21:23:56
 * @details
 */

#ifndef _CRIB_TTREEPERIODICEVENTSTORE_H_
#define _CRIB_TTREEPERIODICEVENTSTORE_H_

#include "IEventStore.h"
#include "TProcessor.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MPI_H
#include <mpi.h>
#endif

namespace art {
class TConditionBit;
class TEventHeader;
} // namespace art

namespace art::crib {
class TTreePeriodicEventStore;
} // namespace art::crib

class TString;
class TFile;
class TTree;
class TList;

class art::crib::TTreePeriodicEventStore : public TProcessor, public IEventStore {

  public:
    TTreePeriodicEventStore();
    ~TTreePeriodicEventStore();

    void Init(TEventCollection *col) override;
    void Process() override;

    /// @brief override from IEventStore
    /// @return Run number
    Int_t GetRunNumber() const override;

    /// @brief override from IEventStore but it is UNSAFE
    /// @return not return anything
    const char *GetRunName() const override;

    /// @brief safe function to get run name
    /// @return std::string runname
    std::string GetStrRunName() const;

  private:
    TString fFileName;
    TString fTreeName;
    TFile *fFile;    //!
    TTree *fTree;    //!
    TList *fObjects; //!
    Long_t fEventNum;
    Long_t fMaxEventNum;
    Long_t fTreeEventNum;
    Long_t fCurrentNum;

    TEventHeader **fEventHeader; //!

#ifdef USE_MPI
    Int_t fRankID; //!
    Int_t fNPE;    //! number of pe
#endif

    ClassDefOverride(TTreePeriodicEventStore, 1);
};
#endif // end of #ifdef _TTREEPERIODICEVENTSTORE_H_
