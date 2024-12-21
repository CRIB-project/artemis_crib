# Creating New Processors

The source code in `artemis` is categorized into several types.
The names below are unofficial and were introduced by [okawak](https://github.com/okawak/):

- **Main**: The primary file for creating the executable binary for artemis.
- **Loop**: Manages the event loop.
- **Hist**: Handles histograms.
- **Command**: Defines commands available in the artemis console.
- **Decoder**: Manages decoders and their configurations.
- **EventStore**: Handles events used in the event loop.
- **Processor**: Processes data.
- **Data**: Defines data structures.
- **Parameter**: Manages parameter objects.
- **Others**: Miscellaneous files, such as those for artemis canvas or other utilities for analysis.

While users are free to customize these components, this chapter focuses on the `Processor` and `Data` categories, as these are often essential for specific analyses.

This chapter provides a step-by-step demonstration of creating a new processor and explains `art::TRIDFEventStore` and `art::TMappingProcessor` in detail, a crucial component for building processors.

## Contents

- [**EventStore**](./event_store.md)

  A brief introduction to `EventStore`.
  This section does not cover how to create or use a new `EventStore`.

- [**General Processors**](./general_processors.md)

  An overview of key concepts shared by all processors.

- [**Process Seg Data**](./seg_data.md)

  Explains `segdata` produced by `art::TRIDFEventStore` and demonstrates how to use it to build a new processor.

- [**Mapping Processors**](./mapping_processor.md)

  Describes **Mapping Processors**, which map data based on `catdata` output from `art::TMappingProcessor`.
  This section also provides a detailed explanation of `catdata`.

- [**Data Classes**](./data_class.md)

  Explains how to define new data classes.
  In artemis, specific classes are typically stored as elements in ROOT's `TClonesArray`.
  This section details how to define data structures for this purpose.

- [**Parameter Objects**](./parameter.md)

  Explains how to define parameters for data analysis, either as member variables within a processor or as standalone objects usable across multiple processors.
