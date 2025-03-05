/**
 * @file    TBranchCopyProcessor.h
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-06-13 16:06:58
 * @note    last modified: 2025-03-05 18:28:23
 * @details
 */

#ifndef CRIB_TBRANCHCOPYPROCESSOR_H_
#define CRIB_TBRANCHCOPYPROCESSOR_H_

#include <TProcessor.h>

class TClonesArray;

namespace art::crib {

/**
 * @class TBranchCopyProcessor
 * @brief Processor for copying data from one TClonesArray to another.
 *
 * ### Example Steering File
 *
 * ```yaml
 * Processor:
 *   - name: MyTBranchCopyProcessor
 *     type: art::crib::TBranchCopyProcessor
 *     parameter:
 *       InputCollection: origin  # [TString] name of the origin branch
 *       OutputCollection: copy   # [TString] output collection name
 *       OutputTransparency: 0    # [Bool_t] Output is persistent if false (default)
 *       Verbose: 1               # [Int_t] verbose level (default 1 : non quiet)
 * ```
 */
class TBranchCopyProcessor : public TProcessor {
  public:
    /**
     * @brief Constructor.
     */
    TBranchCopyProcessor();

    /**
     * @brief Destructor.
     */
    ~TBranchCopyProcessor() override;

    /**
     * @brief Initializes the processor with the given event collection.
     *
     * @param col Pointer to the event collection.
     */
    void Init(TEventCollection *col) override;

    /**
     * @brief Processes the data by copying from input to output collection.
     */
    void Process() override;

  protected:
    TString fInputColName{"origin"}; ///< Name of the input collection.
    TString fOutputColName{"copy"};  ///< Name of the output collection.
    TClonesArray **fInData{nullptr}; ///<! Pointer to the input data
    TClonesArray *fOutData{nullptr}; ///<! Pointer to the output data.

  private:
    TBranchCopyProcessor(const TBranchCopyProcessor &) = delete;
    TBranchCopyProcessor &operator=(const TBranchCopyProcessor &) = delete;

    ClassDefOverride(TBranchCopyProcessor, 2); ///< ROOT class definition macro.
};

} // namespace art::crib

#endif // end of #ifndef CRIB_TBRANCHCOPYPROCESSOR_H_
