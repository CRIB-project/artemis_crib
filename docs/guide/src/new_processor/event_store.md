# EventStore

In the previous CRIB analyses, the following three types of `EventStore` provided by artemis were sufficient:

- `art::TRIDFEventStore`: Reads RIDF files
- `art::TTreeEventStore`: Reads ROOT files
- `art::TRandomNumberEventStore`: Output random number

If you want to create a new `EventStore`, please refer to these files as examples.
