# General Processors

To process data in `artemis`, you need to define a class that inherits from `art::TProcessor` and implement specific data processing logic.
Since `artemis` integrates with ROOT, the class must also be registered in **the ROOT dictionary**, requiring some adjustments beyond standard C++ conventions.

This section explains the common settings and methods that should be implemented when creating any processor.

## Header File

We will use an example of a simple processor named `art::crib::THogeProcessor`.
The prefix `T` follows ROOT naming conventions.
The header file is named `THogeProcessor.h` as this extension is conventionally used in artemis.

### 1. Include Guard

Use an include guard to prevent multiple inclusions of the header file.
Although `#pragma once` is an alternative, a token-based include guard is recommended for broader compatibility.

```cpp
#ifndef _CRIB_THOGEPROCESSOR_H_
#define _CRIB_THOGEPROCESSOR_H_

#endif
```

### 2. Include `TProcessor.h`

Include the base class `art::TProcessor` to inherit its functionality.
Forward declarations are not sufficient for inheritance.

```cpp
#include <TProcessor.h>
```

### 3. Forward Declarations

Use forward declarations for classes defined elsewhere to minimize dependencies.
This approach reduces compilation overhead.

```cpp
class TClonesArray;

namespace art {
class TSegmentedData;
class TCategorizedData;
} // namespace art
```

### 4. Class Definition

Classes developed for CRIB should use the `art::crib` namespace and inherit from `art::TProcessor`.
Although inline namespace declarations like `class art::crib::THogeProcessor` are possible, using a namespace block improves readability.

```cpp
namespace art::crib {
class THogeProcessor : public TProcessor {
};
} // namespace art::crib
```

> Note: The inline namespace syntax (`art::crib`) is standardized in C++17.

### 5. Class Methods

Define essential methods for a processor, such as constructors, destructors, and overridden methods from `art::TProcessor`.

```cpp
class THogeProcessor : public TProcessor {
  public:
    THogeProcessor();
    ~THogeProcessor();

    void Init(TEventCollection *col) override;
    void Process() override;
    void EndOfRun() override;
};
```

- Override additional virtual methods based on your analysis requirements.
  Refer to the `art::TProcessor` implementation for a full list of available methods.
- Use the `override` keyword to ensure clarity and correctness.

### 6. Copy Constructor and Assignment Operator

Disable copy constructors and assignment operators as they are not used in artemis.

```cpp
class THogeProcessor : public TProcessor {
  private:
    THogeProcessor(const THogeProcessor &) = delete;
    THogeProcessor &operator=(const THogeProcessor &) = delete;
};
```

### 7. Member Variables

Define member variables as `private` or `protected` unless they need external access.
**Use pointers for forward-declared classes.**

```cpp
class THogeProcessor : public TProcessor {
  private:
    TClonesArray **fInData;
    TSegmentedData **fSegmentedData;
    TCategorizedData **fCategorizedData;

    Int_t fCounter;
    Int_t fIntParameter;
};
```

### 8. ROOT-Specific Features

To integrate the class with ROOT, follow these conventions:

- Add `//!` to pointer-to-pointer members to exclude them from ROOT's streaming mechanism (`TStreamer`).
- Use `ClassDefOverride` if the class has `override` methods; otherwise, use `ClassDef`.

<div class="warning">

Double pointers can be complex and often reduce code readability.
While they are commonly seen in existing code, it is recommended to avoid using double pointers unless absolutely necessary.

</div>

The macro’s second argument indicates the class version.
Increment the version number when making changes to maintain compatibility.

```cpp
class THogeProcessor : public TProcessor {
  private:
    ClassDefOverride(THogeProcessor, 0);
};
```

> **Note**: Adding `;` after ROOT macros, though not required, improves readability.

### Complete Header File Example

```cpp
#ifndef _CRIB_THOGEPROCESSOR_H_
#define _CRIB_THOGEPROCESSOR_H_

#include "TProcessor.h"

class TClonesArray;

namespace art {
class TSegmentedData;
class TCategorizedData;
} // namespace art

namespace art::crib {
class THogeProcessor : public TProcessor {
  public:
    THogeProcessor();
    ~THogeProcessor();

    void Init(TEventCollection *col) override;
    void Process() override;
    void EndOfRun() override;

  private:
    TClonesArray **fInData; //!
    TSegmentedData **fSegmentedData; //!
    TCategorizedData **fCategorizedData; //!

    Int_t fCounter;
    Int_t fIntParameter;

    THogeProcessor(const THogeProcessor &) = delete;
    THogeProcessor &operator=(const THogeProcessor &) = delete;

    ClassDefOverride(THogeProcessor, 0);
};
} // namespace art::crib

#endif // end of #ifndef _CRIB_THOGEPROCESSOR_H_
```

## Source File

Save the implementation in a file named `THogeProcessor.cc`.

### 1. Include the Header File

Include the header file with double quotes.

```cpp
#include "THogeProcessor.h"
```

### 2. `ClassImp` Macro

Use the ClassImp macro to register the class with ROOT.
Add `;` for consistency.

```cpp
ClassImp(art::crib::THogeProcessor);
```

### 3. Use a Namespace Block

Wrap implementations within the `art::crib` namespace.

```cpp
namespace art::crib {
// Implementation goes here
}
```

### 4. Implement Class Methods

#### Constructor

The constructor is called when an instance of the class is created.
It initializes member variables and registers parameters using methods like `RegisterProcessorParameter()`.

```cpp
THogeProcessor::THogeProcessor() : fInData(nullptr), fSegmentedData(nullptr), fCategorizedData(nullptr), fCounter(0)
{
  RegisterProcessorParameter("IntParameter", "an example parameter read from steering file",
                              fIntParameter, 0);
}
```

`RegisterProcessorParameter()` links a parameter in the steering file to a member variable.
It accepts four arguments:

- **Parameter Name**: The name used in the steering file.
- **Description**: A brief description of the parameter.
- **Variable Reference**: The variable to assign the parameter value.
- **Default Value**: The default value used if the parameter is not specified in the steering file.

This method is implemented using templates, allowing it to accept variables of different types.
Other functions for registering parameters will be explained later as needed, on a case-by-case basis.

#### Destructor

The destructor is called when the instance is destroyed.
Release resources if needed in the destructor.

```cpp
THogeProcessor::~THogeProcessor() {
}
```

#### `Init()` Method

Called before the event loop begins, when you command `add steering/hoge.yaml`.

```cpp
void THogeProcessor::Init(TEventCollection *) {
  Info("Init", "Parameter: %d", fIntParameter);
}
```

- Use ROOT’s logging functions, such as `Info()`, `Warning()`, and `Error()`, inherited from `TObject`.

#### `Process()` Method

Handles event-by-event processing.

```cpp
void THogeProcessor::Process() {
   fCounter++;
   Info("Process", "Event Number: %d", fCounter);
}
```

#### `EndOfRun()` Method

Finalizes processing after the event loop ends.

```cpp
void THogeProcessor::EndOfRun() {
}
```

#### Complete Source File Example

```cpp
#include "THogeProcessor.h"

ClassImp(art::crib::THogeProcessor);

namespace art::crib {
THogeProcessor::THogeProcessor() : fInData(nullptr), fSegmentedData(nullptr), fCategorizedData(nullptr), fCounter(0)
{
  RegisterProcessorParameter("IntParameter", "an example parameter read from steering file",
                              fIntParameter, 0);
}

THogeProcessor::~THogeProcessor() {
}

void THogeProcessor::Init(TEventCollection *) {
  Info("Init", "Parameter: %d", fIntParameter);
}

void THogeProcessor::Process() {
   fCounter++;
   Info("Process", "Event Number: %d", fCounter);
}

void THogeProcessor::EndOfRun() {
}

} // namespace art::crib
```

## Registering the Class

Add the new class to the `src-crib/artcrib_linkdef.h` file for dictionary registration:

```cpp
// skip
#pragma link C++ class art::crib::THogeProcessor;
// skip
```

Various processors are already registered, so you can copy the line and replace it with the name of your own class.

## Build Configuration

Update `src-crib/CMakeLists.txt` to include the new files:

```plaintext
set(CRIBSOURCES
    # skip
    THogeProcessor.cc
    # skip
   )

set(CRIBHEADERS
    # skip
    THogeProcessor.h
    # skip
    )
```

Rebuild the project:

```shell
artlogin <username>
cd build
cmake ..
make -j4
make install
```

## Additional Comments

Existing processors may have less optimal implementations as they were created when the author ([okawak](https://github.com/okawak/)) was less experienced with C++.
Feel free to improve them.

If you create a new processor or modify an existing one for future CRIB use, document its usage here or include Doxygen comments in the code.
Clear documentation is vital for future team members to maintain and use the tool effectively.
