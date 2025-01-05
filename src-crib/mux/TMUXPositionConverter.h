/**
 * @file    TMUXPositionConverter.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-01-30 11:50:01
 * @note    last modified: 2025-01-05 17:37:40
 * @details
 */

#ifndef CRIB_TMUXPOSITIONCONVERTER_H
#define CRIB_TMUXPOSITIONCONVERTER_H

#include <TConverterBase.h>

namespace art::crib {

/**
 * @class TMUXPositionConverter
 * @brief A class for loading and converting MUX position parameters.
 *
 * The TMUXPositionConverter class provides functionality to load numeric parameters
 * from a file and convert MUX position signals into silicon-strip detector
 * strip numbers using binary search. It inherits from TConverterBase.
 *
 * ### Example Steering File
 *
 * ```yaml
 * Processor:
 *   - name: MyTParameterArrayLoader
 *     type: art::TParameterArrayLoader
 *     parameter:
 *       FileFormat: text        # [TString] file format : text (default), yaml
 *       FileName: path/to/file  # [TString] input filename
 *       Name: parameter         # [TString] name of parameter array output
 *       OutputTransparency: 0   # [Bool_t] Output is persistent if false (default)
 *       Type: TParameterObject  # [TString] type(class) of parameter
 *       Verbose: 1              # [Int_t] verbose level (default 1 : non quiet)
 * ```
 */
class TMUXPositionConverter : public TConverterBase {
  public:
    /**
     * @brief Default constructor.
     */
    TMUXPositionConverter();

    /**
     * @brief Default destructor.
     */
    ~TMUXPositionConverter();

    /**
     * @brief Converts a value based on the loaded parameters.
     * @param val The input value to be converted.
     * @return The index of the boundary, or kInvalidI if the value is out of range
     *         or if no parameters have been loaded.
     */
    Double_t Convert(Double_t val) const override;

    /**
     * @brief Loads numeric parameters from a string.
     * @param str A TString containing the parameters to load.
     * @return True if parameters were successfully loaded; otherwise, false.
     */
    Bool_t LoadString(const TString &str) override;

    /**
     * @brief Prints the loaded parameters.
     * @param opt Optional string parameter (currently unused).
     */
    void Print(Option_t *opt = "") const override;

  private:
    std::vector<Double_t> fParams; ///< A vector to store the loaded numeric parameters.

    ClassDefOverride(TMUXPositionConverter, 2); ///< ROOT macro for class definition.
};
} // namespace art::crib

#endif // CRIB_TMUXPOSITIONCONVERTER_H
