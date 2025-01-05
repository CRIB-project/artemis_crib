/**
 * @file    TProcessorUtil.h
 * @brief   Utility functions for handling input and parameter objects in TEventCollection.
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2025-01-02 14:48:14
 * @note    last modified: 2025-01-05 18:52:26
 * @details
 */

#ifndef CRIB_TPROCESSORUTIL_H_
#define CRIB_TPROCESSORUTIL_H_

#include <type_traits>
#include <variant>

#include <TClonesArray.h>
#include <TEventCollection.h>
#include <TString.h>

namespace art::crib::util {

/**
 * @brief Retrieve an object from TEventCollection with type validation.
 *
 * @details
 * This function checks if the specified object exists in the TEventCollection and verifies that its type matches the expected ROOT class type.
 * If the object is of type TClonesArray, it further checks that the elements of the array match the specified element type.
 *
 * - `col->GetObjectRef(name)` is used to retrieve the object reference.
 * - If the object type does not match `expectedTypeName`, an error message is returned.
 * - For `TClonesArray`, the element type is verified using `GetClass`.
 *
 * @tparam T Expected type of the object, must derive from TObject.
 * @param col Pointer to the TEventCollection.
 * @param name Name of the object in the collection.
 * @param expectedTypeName Expected ROOT class name of the object.
 * @param elementTypeName (Optional) Expected type of elements if T is TClonesArray.
 * @return std::variant containing a pointer to the object if successful, or a TString with an error message.
 */
template <typename T>
std::enable_if_t<
    std::is_base_of<TObject, T>::value,
    std::variant<T *, TString>>
GetInputObject(
    TEventCollection *col,
    const TString &name,
    const TString &expectedTypeName,
    const TString &elementTypeName = "TObject") {
    auto *objRef = col->GetObjectRef(name);
    if (!objRef) {
        return TString::Format("No input collection '%s'", name.Data());
    }

    auto *obj = static_cast<TObject *>(*objRef);
    if (!obj->InheritsFrom(expectedTypeName)) {
        return TString::Format("Invalid input collection '%s': not %s",
                               name.Data(), expectedTypeName.Data());
    }

    auto *cast_obj = static_cast<T *>(obj);

    if constexpr (std::is_same<T, TClonesArray>::value) {
        const auto *cl = cast_obj->GetClass();
        if (!cl || !cl->InheritsFrom(elementTypeName)) {
            return TString::Format("Invalid input collection '%s': not %s elements",
                                   name.Data(), elementTypeName.Data());
        }
    }
    return cast_obj;
}

/**
 * @brief Retrieve a TClonesArray parameter object from TEventCollection.
 *
 * @details
 * This function retrieves a TClonesArray object from the TEventCollection by its name and checks if its elements match the specified type.
 *
 * - `col->GetInfo(name)` is used to obtain the object.
 * - The function returns `nullptr` if the object is not found or its type is invalid.
 * - The expected element type is verified using `GetClass`.
 *
 * @param col Pointer to the TEventCollection.
 * @param name Name of the parameter object.
 * @param elementTypeName (Optional) Expected type of elements in the TClonesArray.
 * @return Pointer to the TClonesArray if successful, or nullptr on failure.
 */
inline TClonesArray *GetParameterObject(
    TEventCollection *col,
    const TString &name,
    const TString &elementTypeName = "art::TParameterObject") {
    // Parameter objects can be get by `GetInfo`
    auto *obj = col->GetInfo(name);
    if (!obj || !obj->InheritsFrom("TClonesArray")) {
        return nullptr;
    }

    auto *prm_obj = static_cast<TClonesArray *>(obj);
    const auto *cl = prm_obj->GetClass();
    if (!cl || !cl->InheritsFrom(elementTypeName)) {
        return nullptr;
    }
    return prm_obj;
}

} // namespace art::crib::util

#endif // CRIB_TPROCESSORUTIL_H
