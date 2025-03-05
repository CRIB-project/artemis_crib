/**
 * @file    TProcessorUtil.h
 * @brief   Utility functions for handling input and parameter objects in TEventCollection.
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2025-01-02 14:48:14
 * @note    last modified: 2025-03-05 14:35:46
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
    std::is_base_of_v<TObject, T>,
    std::variant<T *, TString>>
GetInputObject(TEventCollection *col,
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

    if constexpr (std::is_same_v<T, TClonesArray>) {
        const auto *cl = cast_obj->GetClass();
        if (!cl || !cl->InheritsFrom(elementTypeName)) {
            return TString::Format("Invalid input collection '%s': not %s elements",
                                   name.Data(), elementTypeName.Data());
        }
    }
    return cast_obj;
}

/**
 * @brief Retrieves a parameter object from a TEventCollection.
 *
 * @tparam T The type of the parameter object to retrieve. Must be derived from TObject.
 *           When T is TClonesArray, the element type is additionally validated.
 * @param col Pointer to the TEventCollection from which the parameter object is obtained.
 * @param name Name of the parameter object.
 * @param expectedTypeName The expected type name for the parameter object.
 * @param elementTypeName (Optional) For TClonesArray types, the expected type name for its elements
 *                        (default is "art::TParameterObject").
 * @return A std::variant containing either:
 *         - A pointer to the parameter object of type T if successful, or
 *         - A TString error message indicating the failure reason.
 *
 * @details
 * The function retrieves the parameter object via col->GetInfo(name) and performs several checks:
 * - If no object is found, an error message is returned.
 * - The object is verified to inherit from the expected type.
 * - If T is TClonesArray, the object's element type is also checked using its GetClass() method.
 */
template <typename T>
std::enable_if_t<
    std::is_base_of_v<TObject, T>,
    std::variant<T *, TString>>
GetParameterObject(TEventCollection *col,
                   const TString &name,
                   const TString &expectedTypeName,
                   const TString &elementTypeName = "art::TParameterObject") {
    // Parameter objects can be get by `GetInfo`
    auto *objRef = col->GetInfo(name);
    if (!objRef) {
        return TString::Format("No parameter object '%s'", name.Data());
    }

    auto *obj = static_cast<TObject *>(objRef);
    if (!obj->InheritsFrom(expectedTypeName)) {
        return TString::Format("Invalid input parameter '%s': not '%s'",
                               name.Data(), expectedTypeName.Data());
    }

    auto *prm_obj = static_cast<T *>(obj);
    if constexpr (std::is_same_v<T, TClonesArray>) {
        const auto *cl = prm_obj->GetClass();
        if (!cl || !cl->InheritsFrom(elementTypeName)) {
            return TString::Format("Invalid parameter element '%s': not %s",
                                   name.Data(), elementTypeName.Data());
        }
    }
    return prm_obj;
}

} // namespace art::crib::util

#endif // CRIB_TPROCESSORUTIL_H
