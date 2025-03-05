/**
 * @file    TMUXDataMappingProcessor.h
 * @brief   Declaration of TMUXDataMappingProcessor class for mapping categorized data to TMUXData objects.
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 09:46:45
 * @note    last modified: 2025-03-05 18:32:13
 * @details
 */

#ifndef CRIB_TMUXDATAMAPPINGPROCESSOR_H_
#define CRIB_TMUXDATAMAPPINGPROCESSOR_H_

#include <TProcessor.h>

class TClonesArray;

namespace art {
class TCategorizedData;
} // namespace art

namespace art::crib {
class TMUXData;

/**
 * @class TMUXDataMappingProcessor
 * @brief A processor for mapping categorized data to TMUXData objects.
 *
 * This class processes categorized input data (`TCategorizedData`) and maps it
 * to output objects (`TMUXData`), which are stored in a `TClonesArray`. The
 * mapping is controlled by a specified category ID (`fCatID`) and involves
 * extracting and transforming raw detector values.
 *
 * ### Example Steering File
 *
 * ```yaml
 * Processor:
 *   - name: MyTMUXDataMappingProcessor
 *     type: art::crib::TMUXDataMappingProcessor
 *     parameter:
 *       CatID: -1                    # [Int_t] Category ID
 *       CategorizedDataName: catdata # [TString] Name of the segmented data
 *       OutputCollection: mux        # [TString] Name of the output branch
 *       OutputTransparency: 0        # [Bool_t] Persistent output (default: false)
 *       Verbose: 1                   # [Int_t] Verbosity level (default: 1)
 * ```
 */
class TMUXDataMappingProcessor : public TProcessor {
  public:
    /**
     * @brief Constructor.
     */
    TMUXDataMappingProcessor();

    /**
     * @brief Destructor.
     */
    ~TMUXDataMappingProcessor() override;

    /**
     * @brief Initializes the processor with the provided event collection.
     * @param col Pointer to the `TEventCollection` containing input and output collections.
     */
    void Init(TEventCollection *col) override;

    /**
     * @brief Processes the categorized data and maps it to TMUXData.
     */
    void Process() override;

    /**
     * @brief Processes data for a single detector.
     *
     * @param det_array Pointer to the array of raw data for the detector.
     * @param mux Pointer to the TMUXData object where mapped values will be stored.
     * @return The detector ID associated with the processed data.
     */
    int ProcessDetectorData(const TObjArray *det_array, TMUXData *mux);

  private:
    TString fCategorizedDataName; ///< Name of the input categorized data collection.
    TString fOutputColName;       ///< Name of the output TMUXData collection.

    TCategorizedData **fCategorizedData; ///<! Pointer to the categorized data collection.
    TClonesArray *fOutData;              ///<! Pointer to the output TMUXData array.

    Int_t fCatID; ///< Category ID used for filtering input data.

    TMUXDataMappingProcessor(const TMUXDataMappingProcessor &) = delete;
    TMUXDataMappingProcessor &operator=(const TMUXDataMappingProcessor &) = delete;

    ClassDefOverride(TMUXDataMappingProcessor, 2); ///< ROOT macro for class definition.
};
} // namespace art::crib

#endif // CRIB_TMUXDATAMAPPINGPROCESSOR_H_
