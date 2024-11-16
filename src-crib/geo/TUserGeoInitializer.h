/**
 * @file    TUserGeoInitializer.h
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-17 21:30:15
 * @note    last modified: 2024-08-23 21:33:54
 * @details
 */

#ifndef _CRIB_TUSERGEOINITIALIZER_H_
#define _CRIB_TUSERGEOINITIALIZER_H_

#include <TGeoManager.h>
#include <TMath.h>
#include <TProcessor.h>

namespace art::crib {
class TUserGeoInitializer;
}

class TClonesArray;

class art::crib::TUserGeoInitializer : public TProcessor {
  public:
    /// @brief constructor
    TUserGeoInitializer();
    /// @brief destructor
    ~TUserGeoInitializer();

    /// @brief init
    void Init(TEventCollection *col) override;
    /// @brief process
    void Process() override;

  protected:
    /// @brief It is used for TGeoManager process
    TGeoManager *fGeom;

    /// @brief Input geometry file name. You can define in steering file.
    TString fFileName;
    /// @brief It should be "prm_detector"
    TString fDetPrmName;
    /// @brief It should be "prm_target"
    TString fTargetPrmName;

    /// @brief Make figure of Detectors of not.
    Bool_t fIsVisible;

    /// @brief Detector parameter object (art::TDetectorParameter array)
    TClonesArray *fDetParameterArray;
    /// @brief Target parameter object (art::TTargetParameter array)
    TClonesArray *fTargetParameterArray;

    /// @brief angle converter, degree to radian
    Double_t deg2rad = TMath::DegToRad();

  private:
    void GeometryFromYaml(TString yamlfile);

    // Copy constructor (prohibited)
    TUserGeoInitializer(const TUserGeoInitializer &) = delete;
    // Assignment operator (prohibited)
    TUserGeoInitializer &operator=(const TUserGeoInitializer &) = delete;

    ClassDefOverride(TUserGeoInitializer, 1)
};

#endif // end of #ifndef _TUSERGEOINITIALIZER_H_
