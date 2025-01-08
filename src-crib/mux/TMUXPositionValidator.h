/**
 * @file    TMUXPositionValidator.h
 * @brief   Header file for the TMUXPositionValidator class.
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-30 10:02:32
 * @note    last modified: 2025-01-08 10:32:04
 * @details
 */

#ifndef CRIB_TMUXPOSITIONVALIDATOR_H_
#define CRIB_TMUXPOSITIONVALIDATOR_H_

#include <TProcessor.h>

class TClonesArray;

namespace art::crib {

/**
 * @class TMUXPositionValidator
 * @brief Validates positions from the MUX module based on specified ranges.
 *
 * This class is designed to work with the TMUX module in a daisy-chain configuration,
 * separating data from different modules based on position information.
 *
 * ### Example Steering File
 *
 * ```yaml
 * Processor:
 *   - name: MyTMUXPositionValidator
 *     type: art::crib::TMUXPositionValidator
 *     parameter:
 *       InputCollection: input       # [TString] Input collection of TMUXData objects
 *       OutputCollection: validated  # [TString] Validated output collection
 *       OutputTransparency: 0        # [Bool_t] Output is persistent if false (default)
 *       ValidPositionRange: [0, 0]   # [vector<Double_t>] [min, max] -- Ignored if min == max
 *       Verbose: 1                   # [Int_t] verbose level (default 1 : non quiet)
 * ```
 */
class TMUXPositionValidator : public TProcessor {
  public:
    /**
     * @brief Constructor.
     */
    TMUXPositionValidator();

    /**
     * @brief Destructor.
     */
    ~TMUXPositionValidator() override;

    /**
     * @brief Initializes the validator by setting up input and output collections.
     * @param col A pointer to the TEventCollection used for data management.
     */
    void Init(TEventCollection *col) override;

    /**
     * @brief Processes input data, validates positions, and stores the results.
     */
    void Process() override;

  private:
    TString fInputColName;  ///< Name of the input collection.
    TString fOutputColName; ///< Name of the output collection.
    TClonesArray *fInData;  ///<! Pointer to the input data collection.
    TClonesArray *fOutData; ///<! Pointer to the output data collection.

    std::vector<Double_t> fValidPositionRange; ///< Range of valid positions for validation.

    TMUXPositionValidator(const TMUXPositionValidator &rhs) = delete;
    TMUXPositionValidator &operator=(const TMUXPositionValidator &rhs) = delete;

    ClassDefOverride(TMUXPositionValidator, 2) ///< ROOT class definition macro.
};

} // namespace art::crib

#endif // CRIB_TMUXPOSITIONVALIDATOR_H_
