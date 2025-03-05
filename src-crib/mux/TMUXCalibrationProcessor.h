/**
 * @file    TMUXCalibrationProcessor.h
 * @brief   Processor for calibrating timing, charge, and position data in the MUX system.
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 11:08:53
 * @note    last modified: 2025-03-05 18:32:59
 * @details
 */

#ifndef CRIB_TMUXCALIBRATIONPROCESSOR_H
#define CRIB_TMUXCALIBRATIONPROCESSOR_H

#include <TProcessor.h>

class TClonesArray;

namespace art::crib {

/**
 * @class TMUXCalibrationProcessor
 * @brief Handles the calibration of timing, charge, and position data in the MUX system.
 *
 * This class reads input data from a TClonesArray, applies calibration
 * transformations using affine converters for timing and charge,
 * and custom MUX converters for position. The calibrated data is
 * then stored in another TClonesArray.
 *
 * ### Example Steering File
 *
 * ```yaml
 * Processor:
 *   - name: MyTMUXCalibrationProcessor
 *     type: art::crib::TMUXCalibrationProcessor
 *     parameter:
 *       ChargeConverterArray: no_conversion    # [TString] Energy parameter object of TAffineConverter
 *       HasReflection: 0                       # [Bool_t] Reverse strip order (0--7) if true
 *       InputCollection: mux_raw               # [TString] Array of TMUXData objects
 *       InputIsDigital: 1                      # [Bool_t] Add randomness if true
 *       OutputCollection: mux_cal              # [TString] Output array of TTimingChargeData objects
 *       OutputTransparency: 0                  # [Bool_t] Output is persistent if false (default)
 *       PositionConverterArray: no_conversion  # [TString] Position parameter object of TMUXPositionConverter
 *       TimingConverterArray: no_conversion    # [TString] Timing parameter object of TAffineConverter
 *       Verbose: 1                             # [Int_t] verbose level (default 1 : non quiet)
 * ```
 */
class TMUXCalibrationProcessor : public TProcessor {
  public:
    /**
     * @brief Constructor.
     */
    TMUXCalibrationProcessor();

    /**
     * @brief Destructor. Cleans up allocated resources.
     */
    ~TMUXCalibrationProcessor() override;

    /**
     * @brief Initializes the processor by setting up input and output collections.
     * @param col A pointer to the TEventCollection for managing data collections.
     */
    void Init(TEventCollection *col) override;

    /**
     * @brief Processes the input data, applies calibration, and stores the results.
     */
    void Process() override;

  private:
    TString fInputColName;  ///< Name of the input collection.
    TString fOutputColName; ///< Name of the output collection.
    TClonesArray **fInData; ///<! Pointer to the input data collection.
    TClonesArray *fOutData; ///<! Pointer to the output data collection.

    TString fTimingConverterArrayName;   ///< Name of the timing converter array parameter.
    TString fChargeConverterArrayName;   ///< Name of the charge converter array parameter.
    TString fPositionConverterArrayName; ///< Name of the position converter array parameter.

    TClonesArray *fTimingConverterArray;   ///<! Pointer to the timing converter array.
    TClonesArray *fChargeConverterArray;   ///<! Pointer to the charge converter array.
    TClonesArray *fPositionConverterArray; ///<! Pointer to the position converter array.

    Bool_t fHasReflection;  ///< Indicates whether to apply reflection to the detector ID.
    Bool_t fInputIsDigital; ///< Indicates whether the input data is digital.

    /**
     * @brief Converts a raw position value to a detector ID using the position converter array.
     * @param pos The raw position value to convert.
     * @param id The index of the converter to use.
     * @return The converted detector ID, or a default invalid value if conversion fails.
     */
    double ConvertPosition(double pos, int id) const;

    /**
     * @brief Calibrates a raw value (e.g., timing or charge) using the specified converter array.
     * @param raw The raw value to calibrate.
     * @param id The index of the converter to use.
     * @param converterArray The array of converters to use for calibration.
     * @return The calibrated value, or the raw value if no converter is available.
     */
    double CalibrateValue(double raw, int id, const TClonesArray *converterArray) const;

    TMUXCalibrationProcessor(const TMUXCalibrationProcessor &rhs) = delete;
    TMUXCalibrationProcessor &operator=(const TMUXCalibrationProcessor &rhs) = delete;

    ClassDefOverride(TMUXCalibrationProcessor, 2); ///< ROOT class definition macro.
};
} // namespace art::crib

#endif // CRIB_TMUXCALIBRATIONPROCESSOR_H
