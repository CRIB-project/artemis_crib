# Data Classes

In previous examples, we used `art::TSimpleData`, which stores a single `fValue` element, as an item in a `TClonesArray`.
When handling more complex data structures, you need to define custom data classes.

This page explains how to design a data class using `art::crib::TMUXData` as an example.

- [TMUXData Documentation](https://crib-project.github.io/artemis_crib/reference/html/classart_1_1crib_1_1TMUXData.html)

Finally, we demonstrate how to use `TMUXData` in a mapping processor to pack data from `catdata` into the `TMUXData` structure for MUX data.

- [TMUXDataMappingProcessor Documentation](https://crib-project.github.io/artemis_crib/reference/html/classart_1_1crib_1_1TMUXDataMappingProcessor.html)

## Designing `TMUXData`

### Step 1: Include Guards

Add a unique include guard to the header file to prevent multiple inclusions.

```cpp
#ifndef _CRIB_TMUXDATA_H
#define _CRIB_TMUXDATA_H

#endif // _CRIB_TMUXDATA_H
```

### Step 2: Namespace Block

Define the class within the `art::crib` namespace to ensure proper organization.

```cpp
namespace art::crib {
} // namespace art::crib
```

### Step 3: Class Definition

All Artemis data classes must inherit from `art::TDataObject`.
Include this header file and define the basic class structure:

```cpp
#include <TDataObject.h>

namespace art::crib {
class TMUXData : public TDataObject {
  public:
    TMUXData();
    ~TMUXData();
    TMUXData(const TMUXData &rhs);
    TMUXData &operator=(const TMUXData &rhs);

    void Copy(TObject &dest) const override;
    void Clear(Option_t *opt = "") override;
  private:
    ClassDefOverride(TMUXData, 1);
};
} // namespace art::crib
```

This class includes:

- Constructor and destructor
- Copy constructor and assignment operator
- `Copy` and `Clear` methods (required for all data classes)

The `ClassDef` macro enables ROOT to manage the class.

### Step 4: Data Structure Design

MUX modules output five types of data as a group: `[E1, E2, P1, P2, T]`.
Define a structure to store these values in one object.

```cpp
class TMUXData : public TDataObject {
  public:
    // Getter and Setter
    Double_t GetE1() const { return fE1; }
    void SetE1(Double_t value) { fE1 = value; }
    Double_t GetE2() const { return fE2; }
    void SetE2(Double_t value) { fE2 = value; }
    Double_t GetP1() const { return fP1; }
    void SetP1(Double_t value) { fP1 = value; }
    Double_t GetP2() const { return fP2; }
    void SetP2(Double_t value) { fP2 = value; }
    Double_t GetTrig() const { return fTiming; }
    void SetTrig(Double_t value) { fTiming = value; }

    static const Int_t kNRAW = 5;

  private:
    Double_t fE1;
    Double_t fE2;
    Double_t fP1;
    Double_t fP2;
    Double_t fTiming;
};
```

Store the data in private member variables and provide access through getters and setters.

### Step 5: Implement Methods

Implement the required methods in the source file.
These include:

- Initializing member variables in the constructor
- Implementing the destructor (if necessary)
- Copy constructor and assignment operator
- `Copy` and `Clear` methods

Additionally, handle the logic of the parent class `art::TDataObject`.

```cpp
#include "TMUXData.h"

#include <constant.h> // for kInvalidD and kInvalidI

ClassImp(art::crib::TMUXData);

namespace art::crib {
TMUXData::TMUXData()
    : fE1(kInvalidD), fE2(kInvalidD),
      fP1(kInvalidD), fP2(kInvalidD),
      fTiming(kInvalidD) {
    TDataObject::SetID(kInvalidI);
}

TMUXData::~TMUXData() = default;

TMUXData::TMUXData(const TMUXData &rhs)
    : TDataObject(rhs),
      fE1(rhs.fE1),
      fE2(rhs.fE2),
      fP1(rhs.fP1),
      fP2(rhs.fP2),
      fTiming(rhs.fTiming) {
}

TMUXData &TMUXData::operator=(const TMUXData &rhs) {
    if (this != &rhs) {
        TDataObject::operator=(rhs);
        fE1 = rhs.fE1;
        fE2 = rhs.fE2;
        fP1 = rhs.fP1;
        fP2 = rhs.fP2;
        fTiming = rhs.fTiming;
    }
    return *this;
}

void TMUXData::Copy(TObject &dest) const {
    TDataObject::Copy(dest);
    auto *cobj = dynamic_cast<TMUXData *>(&dest);
    cobj->fE1 = this->GetE1();
    cobj->fE2 = this->GetE2();
    cobj->fP1 = this->GetP1();
    cobj->fP2 = this->GetP2();
    cobj->fTiming = this->GetTrig();
}

void TMUXData::Clear(Option_t *opt) {
    TDataObject::Clear(opt);
    TDataObject::SetID(kInvalidI);
    fE1 = kInvalidD;
    fE2 = kInvalidD;
    fP1 = kInvalidD;
    fP2 = kInvalidD;
    fTiming = kInvalidD;
}
} // namespace art::crib
```

These methods ensure that the data class is properly initialized, copied, and cleared during its lifecycle.

## Designing `TMUXDataMappingProcessor`

With the `TMUXData` class created, we can now use it in an actual processor.
This section also explains the general structure of the `Process()` function in a Mapping Processor.
For detailed information, refer to [Mapping Processors](./mapping_processor.md).

### Selecting the Category ID

The Category ID (`catid`) groups detectors or data requiring similar processing.
Unlike the `TMapSelector` processor introduced earlier, all data within a single `catid` is generally processed together in a single processor.

```cpp
// fCatID: Int_t
const auto *cat_array = fCategorizedData->FindCategory(fCatID);
```

All data in this `cat_array` will be used within the processor.

### Iterating Through Detector IDs

To access all data within `cat_array`, iterate using a `for` loop:

```cpp
const int nDet = cat_array->GetEntriesFast();
    for (int iDet = 0; iDet < nDet; ++iDet) {
        const auto *det_array = static_cast<const TObjArray *>(cat_array->At(iDet));
    }
```

> **Note**: The `detid` specified in the map file does not directly match the array index.

You can retrieve the `detid` from `art::TRawDataObject` and store it in the `art::crib::TMUXData` object:

```cpp
// data : art::crib::TMUXData*
int detID = data->GetDetID();
muxData->SetID(detID);
```

`SetID` is defined in the parent class `art::TDataObject`.
When interacting with the object in Artemis, use `fID` for access:

- `detid` <-> `fID`

Example:

```shell
artemis [] tree->Draw("obj.fID")
```

### Accessing `TRawDataObject`

To retrieve a data object from `det_array`:

```cpp
const auto *data_array = static_cast<const TObjArray *>(det_array->At(iType));
const auto *data = dynamic_cast<const TRawDataObject *>(data_array->At(0));
```

For multi-hit TDCs, where multiple data points exist in a single segment, the size of `data_array` increases.
When receiving data from `catdata` (as a **Mapping Processor**), the data is handled as `art::TRawDataObject`.

### Storing Data in the Output Object

To store data in the output object defined in `Init()` or similar functions, allocate memory using `TClonesArray`'s `ConstructedAt(idx)`:

```cpp
auto *outData = static_cast<TMUXData *>(fOutData->ConstructedAt(idx));
outData->SetE1(raw_data[0]);
outData->SetE2(raw_data[1]);
outData->SetP1(raw_data[2]);
outData->SetP2(raw_data[3]);
outData->SetTrig(raw_data[4]);
```

Here, the output object is cast to `art::crib::TMUXData`, and the data is stored using the defined setters.

For the next event, clear the values by calling:

```cpp
fOutData->Clear("C");
```

This invokes the `Clear()` function defined in `art::crib::TMUXData`.
Ensure that all elements are correctly cleared; otherwise, data from previous events might persist.
Proper implementation of the `Clear()` function is essential.

## Summary

- **TMUXData**: A custom data class tailored for MUX data.
- **TMUXDataMappingProcessor**: Demonstrates how to process and store data using the custom class.
  - Group data by `catid` and access it using `detid`.
  - Process raw data (`TRawDataObject`) and store it in `TMUXData`.
- **Key Considerations**:
  - Use `SetID` to store `detid` and access it consistently with `fID`.
  - Implement the `Clear()` function correctly to avoid processing errors in subsequent events.

This guide completes the design and usage of `TMUXData` and its integration into a mapping processor.
The example provides a solid foundation for handling more complex data structures in similar workflows.
