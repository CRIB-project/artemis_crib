/**
 * @file    TMUXData.h
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2022-07-30 09:48:52
 * @note    last modified: 2024-12-31 10:39:28
 * @details
 */

#ifndef _CRIB_TMUXDATA_H
#define _CRIB_TMUXDATA_H

#include "TDataObject.h"

namespace art::crib {
/**
 * @class TMUXData
 * @brief Class to store MUX data, E1, E2, P1, P2, T
 *
 * This class is used to represent and manipulate data taken
 * from MUX module. It is derived from TDataObject
 * and can be stored in ROOT files as part of a TTree.
 */
class TMUXData : public TDataObject {
  public:
    /**
     * @brief Constructor.
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

    /**
     * @brief Get the first energy value.
     * @return The first energy value.
     */
    Double_t GetE1() const { return fE1; }
    /**
     * @brief Get the first energy value.
     * @return The first energy value.
     */
    Double_t E1() const { return fE1; }
    /**
     * @brief Get the first energy value.
     * @return The first energy value.
     */
    Double_t e1() const { return fE1; }

    /**
     * @brief Set the first energy value.
     * @param value The energy value to set.
     */
    void SetE1(Double_t arg) { fE1 = arg; }

    /**
     * @brief Get the second energy value.
     * @return The second energy value.
     */
    Double_t GetE2() const { return fE2; }
    /**
     * @brief Get the second energy value.
     * @return The second energy value.
     */
    Double_t E2() const { return fE2; }

    /**
     * @brief Set the second energy value.
     * @param value The energy value to set.
     */
    Double_t e2() const { return fE2; }

    /**
     * @brief Get the second energy value.
     * @return The second energy value.
     */
    void SetE2(Double_t arg) { fE2 = arg; }

    /**
     * @brief Get the first position value.
     * @return The first position value.
     */
    Double_t GetP1() const { return fP1; }
    /**
     * @brief Get the first position value.
     * @return The first position value.
     */
    Double_t P1() const { return fP1; }
    /**
     * @brief Get the first position value.
     * @return The first position value.
     */
    Double_t p1() const { return fP1; }

    /**
     * @brief Set the first position value.
     * @param value The position value to set.
     */
    void SetP1(Double_t arg) { fP1 = arg; }

    /**
     * @brief Get the second position value.
     * @return The second position value.
     */
    Double_t GetP2() const { return fP2; }
    /**
     * @brief Get the second position value.
     * @return The second position value.
     */
    Double_t P2() const { return fP2; }
    /**
     * @brief Get the second position value.
     * @return The second position value.
     */
    Double_t p2() const { return fP2; }

    /**
     * @brief Set the second position value.
     * @param value The position value to set.
     */
    void SetP2(Double_t arg) { fP2 = arg; }

    /**
     * @brief Get the trigger timing value.
     * @return The trigger timing value.
     */
    Double_t GetTrig() const { return fTiming; }
    /**
     * @brief Set the trigger timing value.
     * @param value The trigger timing value to set.
     */
    void SetTrig(Double_t arg) { fTiming = arg; }

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

    /**
     * @brief Number of raw data elements.
     *
     * This constant represents the number of raw data elements
     * (E1, E2, P1, P2, Trigger).
     */
    static const Int_t kNRAW = 5;

  private:
    Double_t fE1;     ///< First energy value.
    Double_t fE2;     ///< Second energy value.
    Double_t fP1;     ///< First position value.
    Double_t fP2;     ///< Second position value.
    Double_t fTiming; ///< Timing value for the trigger.

    ClassDefOverride(TMUXData, 1);
};
} // namespace art::crib

#endif // _CRIB_TMUXDATA_H
