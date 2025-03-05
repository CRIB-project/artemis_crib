/**
 * @file    TMUXPositionValidator.cc
 * @brief   Implementation of the TMUXPositionValidator class for validating positions in MUX data.
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-30 10:02:46
 * @note    last modified: 2025-03-05 18:31:54
 * @details
 */

#include "TMUXPositionValidator.h"
#include "../TProcessorUtil.h"

#include "TMUXData.h"

/// ROOT macro for class implementation
ClassImp(art::crib::TMUXPositionValidator);

namespace art::crib {

/**
 * @details
 * The constructor initializes member variables and registers input/output collections
 * as well as the `ValidPositionRange` parameter.
 * The default position range is set to `[0.0, 0.0]`, which effectively disables validation
 * unless explicitly configured.
 */
TMUXPositionValidator::TMUXPositionValidator()
    : fInData(nullptr), fOutData(nullptr) {
    RegisterInputCollection("InputCollection", "Input collection of TMUXData objects",
                            fInputColName, TString("input"));
    RegisterOutputCollection("OutputCollection", "Validated output collection",
                             fOutputColName, TString("validated"));

    const DoubleVec_t range(2, 0.0);
    RegisterProcessorParameter("ValidPositionRange",
                               "[min, max] -- Ignored if min == max",
                               fValidPositionRange, range);
}

/**
 * @details
 * The destructor releases any allocated resources, specifically deallocating the `fOutData`
 * collection to prevent memory leaks.
 */
TMUXPositionValidator::~TMUXPositionValidator() {
    delete fOutData;
    fOutData = nullptr;
}

/**
 * @details
 * This method initializes the processor by:
 * - Retrieving the input data collection from the event collection (`col`).
 * - Validating the configuration of the position range parameter (`fValidPositionRange`).
 * - Creating the output collection for validated data.
 *
 * If the position range parameter is invalid (e.g., incorrect size or `min > max`),
 * the processor enters an error state and halts further processing.
 */
void TMUXPositionValidator::Init(TEventCollection *col) {
    // Retrieve the input data collection
    auto result = util::GetInputObject<TClonesArray>(
        col, fInputColName, "TClonesArray", "art::crib::TMUXData");

    if (std::holds_alternative<TString>(result)) {
        SetStateError(std::get<TString>(result));
        return;
    }
    fInData = std::get<TClonesArray **>(result);
    Info("Init", "%s => %s", fInputColName.Data(), fOutputColName.Data());

    // Validate the position range parameter
    if (fValidPositionRange.size() != 2) {
        SetStateError(Form("Position range size should be 2, but %zu",
                           fValidPositionRange.size()));
        return;
    }

    if (fValidPositionRange[0] > fValidPositionRange[1]) {
        SetStateError("Position range : min > max");
        return;
    }

    // Set up the output data collection
    fOutData = new TClonesArray("art::crib::TMUXData");
    fOutData->SetName(fOutputColName);
    col->Add(fOutputColName, fOutData, fOutputIsTransparent);
}

/**
 * @details
 * The `Process` method validates each entry in the input collection against the
 * configured position range. Entries with positions (`P1`) within the range are copied
 * to the output collection, while others are ignored.
 *
 * Processing steps:
 * 1. Clear the output collection to prepare for new entries.
 * 2. Iterate over all entries in the input collection.
 * 3. Validate the position (`P1`) of each entry against the range.
 * 4. Copy valid entries to the output collection.
 *
 * Any invalid or missing data is skipped without affecting the remaining entries.
 */
void TMUXPositionValidator::Process() {
    fOutData->Clear("C");

    // Process each entry in the input collection
    const int nData = (*fInData)->GetEntriesFast();
    for (int iData = 0; iData < nData; ++iData) {
        const auto *data = dynamic_cast<const TMUXData *>((*fInData)->At(iData));
        if (!data) {
            // Warning("Process", "Invalid TMUXData object at index %d", iData);
            continue;
        }

        // Validate position
        const double pos1_raw = data->GetP1();
        if (pos1_raw < fValidPositionRange[0] || pos1_raw > fValidPositionRange[1]) {
            continue; // Skip entries outside the valid range
        }

        // Copy valid data to the output collection
        auto *outData = fOutData->ConstructedAt(fOutData->GetEntriesFast());
        data->Copy(*outData);
    }
}

} // namespace art::crib
