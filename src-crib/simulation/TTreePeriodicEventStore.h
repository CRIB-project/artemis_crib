/**
 * @file    TTreePeriodicEventStore.h
 * @brief   Declaration of TTreePeriodicEventStore for periodic event handling in a TTree
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-07-16 15:16:56
 * @note    last modified: 2025-01-08 20:03:50
 * @details
 */

#ifndef CRIB_TTREEPERIODICEVENTSTORE_H_
#define CRIB_TTREEPERIODICEVENTSTORE_H_

#include "IEventStore.h"
#include "TProcessor.h"

namespace art {
class TEventHeader;
} // namespace art

class TTree;

namespace art::crib {
/**
 * @class TTreePeriodicEventStore
 * @brief A processor class that implements periodic event reading from a TTree.
 *
 * This class inherits from TProcessor and IEventStore to provide a mechanism
 * for initializing, reading, and processing events periodically from a ROOT
 * TTree. The user can configure the input file name, tree name, and number of
 * events to process through various parameters.
 *
 * ### Example Steering File
 *
 * ```yaml
 * Processor:
 *   - name: MyTTreePeriodicEventStore
 *     type: art::crib::TTreePeriodicEventStore
 *     parameter:
 *       FileName: temp.root  # [TString] The name of input file
 *       MaxEventNum: 0  # [Long_t] The maximum event number to be analyzed.
 *       OutputTransparency: 0  # [Bool_t] Output is persistent if false (default)
 *       TreeName: tree  # [TString] The name of input tree
 *       Verbose: 1  # [Int_t] verbose level (default 1 : non quiet)
 * ```
 */
class TTreePeriodicEventStore : public TProcessor, public IEventStore {
  public:
    /**
     * @brief Constructor.
     */
    TTreePeriodicEventStore();

    /**
     * @brief Destructor.
     */
    ~TTreePeriodicEventStore() override;

    /**
     * @brief Initializes the TTreePeriodicEventStore with the given event collection.
     *
     * @param col Pointer to a TEventCollection that holds shared objects
     *            and conditions among processors.
     *
     */
    void Init(TEventCollection *col) override;

    /**
     * @brief Processes one event from the TTree.
     */
    void Process() override;

    /**
     * @brief Returns the run number of the current event.
     * @return The run number if available, otherwise 0.
     */
    Int_t GetRunNumber() const override;

    /**
     * @brief Returns the run name as a C-style string.
     * @return A const char pointer to the run name, or an empty string if unavailable.
     */
    const char *GetRunName() const override;

    /**
     * @brief Returns the run name as a std::string.
     * @return The run name, or an empty string if unavailable.
     */
    std::string GetStrRunName() const;

  private:
    TString fFileName{"temp.root"}; ///< The name (or pattern) of the input file(s). Default is "temp.root".
    TString fTreeName{"tree"};      ///< The name of the TTree to be processed. Default is "tree".
    TTree *fTree{nullptr};          ///<! Pointer to the TTree object that holds event data.
    Long_t fEventNum{0};            ///< The current event number within the run.
    Long_t fMaxEventNum{0};         ///< The maximum number of events to process.
    Long_t fTreeEventNum{0};        ///< The total number of events in the TTree.
    Long_t fCurrentNum{0};          ///< The current entry index in the TTree.

    TEventHeader *fEventHeader{nullptr}; ///<! Pointer to the TEventHeader object read from the TTree.

    ClassDefOverride(TTreePeriodicEventStore, 3); ///< ROOT class definition macro.
};
} // namespace art::crib

#endif // end of #ifdef CRIB_TTREEPERIODICEVENTSTORE_H_
