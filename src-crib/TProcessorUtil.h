/**
 * @file    TProcessorUtil.h
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2025-01-02 14:48:14
 * @note    last modified: 2025-01-03 17:02:45
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

template <typename T>
std::enable_if_t<
    std::is_base_of<TObject, T>::value || std::is_same<T, TClonesArray>::value,
    std::variant<T *, TString>>
GetInputObject(
    TEventCollection *col,
    const TString &name,
    const TString &expectedTypeName,
    const TString &elementTypeName = "TObject") {
    // Input collection should be accessed by `GetObjectRef`
    auto *objRef = col->GetObjectRef(name);
    if (!objRef) {
        return Form("No input collection '%s'", name.Data());
    }

    auto *obj = static_cast<TObject *>(*objRef);
    if (!obj->InheritsFrom(expectedTypeName)) {
        return Form("Invalid input collection '%s': not %s",
                    name.Data(), expectedTypeName.Data());
    }

    auto *cast_obj = static_cast<T *>(obj);

    if constexpr (std::is_same<T, TClonesArray>::value) {
        const auto *cl = cast_obj->GetClass();
        if (!cl || !cl->InheritsFrom(elementTypeName)) {
            return Form("Invalid input collection '%s': not %s elements",
                        name.Data(), elementTypeName.Data());
        }
    }
    return cast_obj;
}

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
