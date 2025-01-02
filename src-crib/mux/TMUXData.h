/**
 * @file    TMUXData.h
 * @brief   Header file for the TMUXData class.
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-07-30 09:48:52
 * @note    last modified: 2025-01-02 12:16:56
 * @details
 */

#ifndef CRIB_TMUXDATA_H
#define CRIB_TMUXDATA_H

#include "TDataObject.h"

namespace art::crib {
/**
 * @class TMUXData
 * @brief Represents data from the MUX module (E1, E2, P1, P2, T).
 *
 * The TMUXData class is used to store and manipulate energy, position,
 * and timing values from the MUX module. It is derived from TDataObject
 * and can be stored in ROOT files as part of a TTree.
 */

class TMUXData : public TDataObject {
  public:
    /**
     * @brief Constructor.
     *
     * Initializes all member variables to default values.
     */
    TMUXData();

    /**
     * @brief Destructor.
     */
    ~TMUXData();

    /**
     * @brief Copy constructor.
     * @param rhs The object to copy from.
     */
    TMUXData(const TMUXData &rhs);

    /**
     * @brief Copy assignment operator.
     * @param rhs The object to assign from.
     * @return A reference to the assigned object.
     */
    TMUXData &operator=(const TMUXData &rhs);

    /// @name Energy Accessors
    /// @{
    Double_t GetE1() const { return fE1; }      ///< Get the first energy value.
    void SetE1(Double_t value) { fE1 = value; } ///< Set the first energy value.

    Double_t GetE2() const { return fE2; }      ///< Get the second energy value.
    void SetE2(Double_t value) { fE2 = value; } ///< Set the second energy value.
    /// @}

    /// @name Position Accessors
    /// @{
    Double_t GetP1() const { return fP1; }      ///< Get the first position value.
    void SetP1(Double_t value) { fP1 = value; } ///< Set the first position value.

    Double_t GetP2() const { return fP2; }      ///< Get the second position value.
    void SetP2(Double_t value) { fP2 = value; } ///< Set the second position value.
    /// @}

    /// @name Timing Accessors
    /// @{
    Double_t GetTrig() const { return fTiming; }      ///< Get the timing value.
    void SetTrig(Double_t value) { fTiming = value; } ///< Set the timing value.

    /**
     * @brief Get the timing value at a specific index.
     * @param index The index of the timing value to retrieve.
     * @return The timing value if valid, otherwise `kInvalidD`.
     */
    Double_t GetT(Int_t index = 0) const;

    /**
     * @brief Add a new timing value.
     * @param value The timing value to add.
     */
    void PushTiming(Double_t value);
    /// @}

    /**
     * @brief Copy the contents of this object to another TObject.
     * @param dest The destination TObject.
     */
    void Copy(TObject &dest) const override;

    /**
     * @brief Clear the object's contents.
     * @param opt Optional arguments for the clear operation.
     */
    void Clear(Option_t *opt = "") override;

    /// Number of raw data elements (E1, E2, P1, P2, Trigger).
    static const Int_t kNRAW = 5;

  private:
    Double_t fE1;     ///< First energy value.
    Double_t fE2;     ///< Second energy value.
    Double_t fP1;     ///< First position value.
    Double_t fP2;     ///< Second position value.
    Double_t fTiming; ///< Timing value for the trigger.

    std::vector<Double_t> fTVec; ///< Vector of timing values from MHTDC.

    ClassDefOverride(TMUXData, 2);
};
} // namespace art::crib

#endif // CRIB_TMUXDATA_H
