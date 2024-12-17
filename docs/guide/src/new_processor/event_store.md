# EventStore

For previous CRIB analyses, the following three types of `EventStore` provided by artemis have been sufficient:

- `art::TRIDFEventStore`: Reads RIDF files.
- `art::TTreeEventStore`: Reads ROOT files.
- `art::TRandomNumberEventStore`: Outputs random numbers.

If you need to create a new `EventStore`, these files can serve as useful examples.
