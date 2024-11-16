/**
 * @file    TReconstProcessor.h
 * @brief   for solid target reconstruction
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-09-03 14:33:21
 * @note    last modified: 2024-09-03 16:29:44
 * @details
 */

#ifndef _CRIB_TRECONSTPROCESSOR_H_
#define _CRIB_TRECONSTPROCESSOR_H_

#include "../telescope/TTelescopeData.h"
#include <TProcessor.h>
#include <TTrack.h>

namespace art::crib {
class TReconstProcessor;
} // namespace art::crib

class TClonesArray;

class art::crib::TReconstProcessor : public TProcessor {
  public:
    /// @brief Default constructor.
    TReconstProcessor();
    /// @brief Default destructor.
    ~TReconstProcessor();

    /// @brief Initialization
    void Init(TEventCollection *col) override;
    /// @brief Main process
    void Process() override;

  protected:
    /// @brief input telescope collection name (art::TTelescopeData)
    TString fInputColName;
    /// @brief input tracking collection name (art::TTrack)
    TString fInputTrackColName;
    /// @brief output collection name (art::TReactionInfo)
    TString fOutputColName;
    /// @brief detector parameter name (art::TDetectorParameter)
    TString fDetectorParameterName;
    /// @brief target parameter name (art::TTargetParameter)
    TString fTargetParameterName;

    /// @brief telescope input object (TClonesArray(art::TTelescopeData))
    TClonesArray **fInData; //!
    /// @brief tracking input object (TClonesArray(art::TTrack))
    TClonesArray **fInTrackData; //!
    /// @brief output object (TClonesArray(art::TReactionInfo))
    TClonesArray *fOutData;

    /// @brief detector parameter object (TClonesArray(art::TDetectorParameter))
    TClonesArray **fDetectorPrm; //!
    /// @brief target parameter obejct (TClonesArray(art::TTargetParameter))
    TClonesArray **fTargetPrm; //!

    /// @brief reaction particles Atomic num array
    IntVec_t fParticleZArray;
    /// @brief reaction particles Mass num array
    IntVec_t fParticleAArray;
    /// @brief Excited Energy
    Double_t fExcitedEnergy;
    /// @brief Flag of custom processor
    Bool_t fDoCenterPos;

    Double_t M1;
    Double_t M2;
    Double_t M3_default;
    Double_t M3;
    Double_t M4;

  private:
    /// @brief Get LAB energy and angle from detected particle information
    /// @param z (mm)
    /// @param track (art::TTrack)
    /// @param data (art::TTelescopeData)
    /// @return Ecm (MeV)
    std::pair<Double_t, Double_t> GetELabALabPair(Double_t z, const TTrack *track, const TTelescopeData *data);

    /// @brief Get Ecm from detected particle information
    /// @param z (mm)
    /// @param track (art::TTrack)
    /// @param data (art::TTelescopeData)
    /// @return Ecm (MeV)
    Double_t GetEcmFromDetectParticle(Double_t z, const TTrack *track, const TTelescopeData *data);

    /// @brief Get Ecm from detected particle information (relativity kinematics)
    /// @param energy (MeV)
    /// @param theta (radian)
    /// @param low_e (minimum energy)
    /// @param high_e (maxmum energy)
    /// @return Ecm (MeV)
    Double_t GetEcm_kinematics(Double_t energy, Double_t theta, Double_t low_e, Double_t high_e);

    /// @brief Get Ecm from detected particle information (classic kinematics)
    /// @param energy (MeV)
    /// @param theta (radian)
    /// @return Ecm (MeV)
    Double_t GetEcm_classic_kinematics(Double_t energy, Double_t theta);

    /// @brief Get Lab Angle after reconstruction
    /// @param ELab (MeV)
    /// @param Ecm (MeV)
    /// @param ALab (radian)
    /// @return angle (radian)
    Double_t GetCMAngle(Double_t ELab, Double_t Ecm, Double_t ALab);

    // Copy constructor (prohibited)
    TReconstProcessor(const TReconstProcessor &rhs) = delete;
    // Assignment operator (prohibited)
    TReconstProcessor &operator=(const TReconstProcessor &rhs) = delete;

    ClassDefOverride(TReconstProcessor, 1)
};

#endif // end of #ifndef _TRECONSTPROCESSOR_H_
