/**
 * @file    TSimpleVectorData.h
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-12-29 12:12:02
 * @note    last modified: 2024-12-29 17:22:43
 * @details
 */

#ifndef _CRIB_TSIMPLEVECTORDATA_H
#define _CRIB_TSIMPLEVECTORDATA_H

#include <TDataObject.h>
#include <constant.h>
#include <vector>

namespace art::crib {
template <typename T>
class TSimpleVectorDataT : public TDataObject {
  public:
    TSimpleVectorDataT();
    virtual ~TSimpleVectorDataT();

    TSimpleVectorDataT(const TSimpleVectorDataT &rhs);
    TSimpleVectorDataT &operator=(const TSimpleVectorDataT &rhs);

    virtual void Copy(TObject &dest) const override;
    virtual void Clear(Option_t *opt = "") override;

    T Get(int index) const {
        if (IsValidIndex(index))
            return fVec[index];
        else
            return kInvalidD;
    }

    void Push(T val) { fVec.emplace_back(val); }

  protected:
    std::vector<T> fVec;

    bool IsValidIndex(int index) {
        if (index >= 0 && index < fVec.size())
            return true;
        else
            return false;
    }
};

template <typename T>
TSimpleVectorDataT<T>::TSimpleVectorDataT()
    : TDataObject() {
}

template <typename T>
TSimpleVectorDataT<T>::~TSimpleVectorDataT() {
}

template <typename T>
TSimpleVectorDataT<T>::TSimpleVectorDataT(const TSimpleVectorDataT<T> &rhs)
    : TDataObject(rhs),
      fVec(rhs.fVec) {
}

template <typename T>
TSimpleVectorDataT<T> &TSimpleVectorDataT<T>::operator=(const TSimpleVectorDataT<T> &rhs) {
    if (this != &rhs) {
        TDataObject::operator=(rhs);
        fVec = rhs.fVec;
    }
    return *this;
}

template <typename T>
void TSimpleVectorDataT<T>::Copy(TObject &dest) const {
    TDataObject::Copy(dest);
    auto &obj = static_cast<TSimpleVectorDataT<T> &>(dest);
    obj.fVec = fVec;
}

template <typename T>
void TSimpleVectorDataT<T>::Clear(Option_t *opt) {
    TDataObject::Clear(opt);
    SetID(kInvalidI);
    fVec.clear();
}

class TSimpleVectorData : public TSimpleVectorDataT<Double_t> {
    ClassDef(TSimpleVectorData, 1);
};
class TSimpleVectorDataInt : public TSimpleVectorDataT<Int_t> {
    ClassDef(TSimpleVectorDataInt, 1);
};
class TSimpleVectorDataUInt : public TSimpleVectorDataT<UInt_t> {
    ClassDef(TSimpleVectorDataUInt, 1);
};
class TSimpleVectorDataShort : public TSimpleVectorDataT<Short_t> {
    ClassDef(TSimpleVectorDataShort, 1);
};
class TSimpleVectorDataUShort : public TSimpleVectorDataT<UShort_t> {
    ClassDef(TSimpleVectorDataUShort, 1);
};
class TSimpleVectorDataLong : public TSimpleVectorDataT<Long_t> {
    ClassDef(TSimpleVectorDataLong, 1);
};
class TSimpleVectorDataDouble : public TSimpleVectorDataT<Double_t> {
    ClassDef(TSimpleVectorDataDouble, 1);
};

} // namespace art::crib

#endif // _CRIB_TSIMPLEVECTORDATA_H
