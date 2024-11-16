/**
 * @file    TSolidAngleProcessor.h
 * @brief
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-08-01 11:08:17
 * @note    last modified: 2024-08-23 21:20:38
 * @details
 */

#ifndef _CRIB_TSOLIDANGLEPROCESSOR_H_
#define _CRIB_TSOLIDANGLEPROCESSOR_H_

#include <TProcessor.h>

#include <TH2.h>

namespace art::crib {
class TSolidAngleProcessor;
}

class TClonesArray;

class art::crib::TSolidAngleProcessor : public TProcessor {
  public:
    /// @brief default constructor
    TSolidAngleProcessor();
    /// @brief default destructor
    ~TSolidAngleProcessor();

    /// @brief Init (override)
    /// @param col
    void Init(TEventCollection *col) override;

    /// @brief Process (override)
    void Process() override;
    /// @brief EndOfRun process (override)
    void EndOfRun() override;

  protected:
    /// @brief Input detected particle object name
    TString fInputColName;
    /// @brief Input reaction object name
    TString fInputReacColName;
    /// @brief Input detected particle object
    TClonesArray **fInData; //!
    /// @brief Input reaction object
    TClonesArray **fInReacData; //!

    /// @brief Histogram bin number
    Int_t fNbin_angle;
    /// @brief Histogram range (min, max)
    DoubleVec_t fRange_angle;
    /// @brief Histogram bin number
    Int_t fNbin_energy;
    /// @brief Histogram range (min, max)
    DoubleVec_t fRange_energy;

    /// @brief Output histogram ROOT file name
    TString fFileName;

    /// @brief solid angle (x: angle_cm)
    TH1D *h1_a;
    /// @brief for normalize (x: angle_cm)
    TH1D *h1_a_all;
    /// @brief solid angle (x: Ecm)
    TH1D *h1_e;
    /// @brief for normalize (x: Ecm)
    TH1D *h1_e_all;
    /// @brief solid angle (x: Ecm, y: angle_cm)
    TH2D *h2;
    /// @brief for normalie (x: Ecm, y: angle_cm)
    TH2D *h2_all;

  private:
    TSolidAngleProcessor(const TSolidAngleProcessor &rhs) = delete;
    TSolidAngleProcessor &operator=(const TSolidAngleProcessor &rhs) = delete;

    ClassDefOverride(TSolidAngleProcessor, 1)
};

#endif // end of #ifndef _TSOLIDANGLEPROCESSOR_H_
