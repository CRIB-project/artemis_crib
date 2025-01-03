# Parameter Objects (Converters)

A **Parameter Object** is an object that stores strings or numeric values loaded from a parameter file.
This section focuses on **Converters**, which use these parameters to transform one value into another, such as for calibration purposes.

In the Artemis framework, these objects are managed using the `art::TParameterArrayLoader` processor.
While the `art::TParameterLoader` processor handles non-array parameters, `art::TParameterArrayLoader` can process single-element arrays, making it more versatile.

The `art::TParameterArrayLoader` packs a specific parameter type into a `TClonesArray`, enabling other processors to access it.
To use this feature, you need to define a custom parameter object type to be stored in the `TClonesArray`.

For instance, to transform a value `value` using the first element of the parameter object in the `TClonesArray`:

```cpp
auto converted_value = (prm->At(0))->Convert(value);
```

Here, the `Convert` method performs the transformation.

This guide demonstrates how to implement a Converter, focusing on the `art::crib::TMUXPositionConverter`, which converts MUX position output to strip numbers.
We’ll also explore its application in the `art::crib::TMUXCalibrationProcessor`.

- [TMUXPositionConverter Documentation](https://crib-project.github.io/artemis_crib/reference/html/classart_1_1crib_1_1TMUXPositionConverter.html)
- [TMUXCalibrationProcessor Documentation](https://crib-project.github.io/artemis_crib/reference/html/classart_1_1crib_1_1TMUXCalibrationProcessor.html)

## Understanding `TParameterArrayLoader`

Before creating a custom parameter class, it's important to understand how `art::TParameterArrayLoader` works.

```yaml
Processor:
  - name: MyTParameterArrayLoader
    type: art::TParameterArrayLoader
    parameter:
      FileFormat: text # [TString] file format : text (default), yaml
      FileName: path/to/file # [TString] input filename
      Name: parameter # [TString] name of parameter array output
      OutputTransparency: 0 # [Bool_t] Output is persistent if false (default)
      Type: art::TParameterObject # [TString] type(class) of parameter
      Verbose: 1 # [Int_t] verbose level (default 1 : non quiet)
```

While the `FileFormat` can be set to `yaml` for YAML files, this guide focuses on reading numeric values from text files.
For details on YAML processing, refer to the [art::TParameterArrayLoader](https://github.com/artemis-dev/artemis/blob/develop/sources/loop/param/TParameterArrayLoader.cc) implementation.

The class specified in the `Type` field of the steering file will be implemented in later sections.

### Reading from a Text File

The `FileName` field in the steering file specifies the text file to read.
The following is the relevant snippet from the loader’s implementation (error handling excluded):

```cpp
Bool_t TParameterArrayLoader::LoadText() {
   std::ifstream fin(fFileName.Data());
   TParameterObject *parameter = nullptr;
   TString buf;
   Int_t count = 0;
   while(buf.ReadLine(fin)) {
      if (!parameter) {
         parameter =
            static_cast<TParameterObject*>(fParameterArray->ConstructedAt(count));
      }

      if (parameter->LoadString(buf)) {
         parameter = nullptr;
         ++count;
      }
   }
   fin.close();
   if(!count)
      return kFALSE;
   fParameterArray->Expand(count);
   return kTRUE;
}
```

#### Key Points

- `buf.ReadLine(fin)`: Reads the file line by line.
  - For each line, a new parameter object element is created using `ConstructedAt`.
- `LoadString()`: Processes a single line of text.
  - If `LoadString()` returns `true`, the `count` variable increments, preparing for the next object.
  - This is a virtual method in `art::TParameterObject` that must be overridden in custom classes.

## Defining a Custom Parameter Class

All parameter objects must inherit from `art::TParameterObject`.
To use the `Convert` method, extend `art::TConverterBase`, which declares the `virtual Convert()` method.

```cpp
#include <TConverterBase.h>

namespace art::crib {
class TMUXPositionConverter : public TConverterBase {
  public:
    TMUXPositionConverter();
    ~TMUXPositionConverter();

    Double_t Convert(Double_t val) const override;
    Bool_t LoadString(const TString &str) override;
    void Print(Option_t *opt = "") const override;

  private:
    std::vector<Double_t> fParams;

    ClassDefOverride(TMUXPositionConverter, 0);
};
} // namespace art::crib
```

This class overrides the following methods:

- `Convert`: Performs value transformation.
- `LoadString`: Reads and processes a single line from the text file.

### Implementation in Source File

In the source file, provide specific implementations for the `Convert` and `LoadString` methods.
These methods handle the transformation logic and file parsing, respectively.

```cpp
namespace art::crib {
Double_t TMUXPositionConverter::Convert(const Double_t val) const {
    // Define the specific transformation logic
}

Bool_t TMUXPositionConverter::LoadString(const TString &str) {
    // Define the specific transformation logic
}
} // namespace art::crib
```

> **Note**: Design the structure of the parameter file before implementation to ensure compatibility.
> Reviewing examples in the following sections can help you visualize how the Converter will be used.

## Using Parameter Classes

This section demonstrates how to use parameter classes to create a processor that performs value transformations.
First, use `art::TParameterArrayLoader` to add parameter objects to the `TEventCollection` managed by Artemis.
This can be defined in the steering file as follows:

```yaml
# MUX position parameters
- name: proc_@NAME@_dE_position
  type: art::TParameterArrayLoader
  parameter:
    Name: prm_@NAME@_dEX_position
    Type: art::crib::TMUXPositionConverter
    FileName: prm/@NAME@/pos_dEX/current
    OutputTransparency: 1
```

Here, the `Type` field specifies `art::crib::TMUXPositionConverter`, the converter introduced earlier.

Subsequent processors can access this parameter object using the name defined in the `Name` field.
This workflow is illustrated using the `art::crib::TMUXCalibrationProcessor`.

### Accessing Parameter Objects

Previously, when retrieving data objects from `TEventCollection* col` in the `Init` method, the `GetObjectRef` method was used:

```cpp
auto *objRef = col->GetObjectRef(name);
```

For parameter objects stored in a separate location, use `GetInfo` instead:

```cpp
auto *obj = col->GetInfo(name);
```

The `GetInfo` method returns a `TObject *`, which directly references the parameter object.
When using `art::TParameterArrayLoader`, the object is stored in a `TClonesArray`.
Cast it appropriately to access the parameter values:

```cpp
auto *obj = col->GetInfo(name);
auto *prm_obj = static_cast<TClonesArray *>(obj);

double raw = 0.0; // before conversion
auto *converter = static_cast<TMUXPositionConverter *>(prm_obj->At(0));
double cal = converter ? converter->Convert(raw) : kInvalidD;
```

This code checks if the converter exists and applies the transformation using the `Convert` method.
If the converter is absent, it returns an invalid value (`kInvalidD`).

### Understanding Element Indexing

In the example above, note the use of `At(0)` to access the first element in the parameter array:

```cpp
auto *converter = static_cast<TMUXPositionConverter *>(prm_obj->At(0));
```

When parameters are loaded using `art::TParameterArrayLoader`, the number of rows in the input file corresponds to the indices in the `TClonesArray`.

For example:

- Each row in the file corresponds to one parameter object.
- For MUX calibration, as explained in the [MUX Calibration](../preparation/MUX_calibration.md) section, 17 values are required to separate the P1 output into strip numbers (assuming 16 strips).
  These values are stored in a single row of the parameter file.
- For energy calibration, each strip requires two values.
  The file contains 16 rows (one per strip), with each row providing the coefficients (a and b) for transformations like `x -> f(x) = a + b * x`.
  - This design ensures that parameter object indices correspond directly to detector strip numbers, enabling efficient access and mapping.

It’s important to design parameter files and objects according to the specific requirements of the data being processed.

## Summary

- **Parameter Objects**: Store values from parameter files and are used for tasks such as calibration.
- **art::TParameterArrayLoader**: Loads parameter objects into a `TClonesArray` for access by other processors.
  - Supports multiple file formats (e.g., text, YAML).
  - Processes each row of a text file as a separate parameter object.
- **Custom Parameter Classes**: Extend `art::TParameterObject` or `art::TConverterBase` to implement parameter-specific logic.
  - `LoadString`: Reads and processes individual rows from parameter files.
  - `Convert`: Transforms raw values based on the parameters.
- `Indexing`: File rows map directly to `TClonesArray` indices, aligning with detector strip numbers or other entities.

By tailoring parameter files and classes to match your application’s requirements, you can optimize data access and streamline processing workflows in Artemis.
