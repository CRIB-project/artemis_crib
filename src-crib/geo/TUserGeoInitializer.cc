/**
 * @file    TUserGeoInitializer.cc
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-17 21:27:49
 * @note    last modified: 2024-08-23 21:33:03
 * @details
 */

#include "TUserGeoInitializer.h"

#include "TDetectorParameter.h"
#include "TTargetParameter.h"
#include <TDirectory.h>
#include <TVector3.h>
#include <yaml-cpp/yaml.h>

using art::crib::TUserGeoInitializer;

ClassImp(TUserGeoInitializer);

// definition of constant strings for the key of each node
namespace {
const char *kNodeKeyMaterial = "material";
const char *kNodeKeyConposition = "conposition";
const char *kNodeKeyVolume = "volume";
const char *kNodeKeyName = "name";
const char *kNodeKeyAMass = "atomic_mass";
const char *kNodeKeyANum = "atomic_num";
const char *kNodeKeyDensity = "density";
const char *kNodeKeyThickness = "thickness";
const char *kNodeKeyCenterRot = "center_rotation";
const char *kNodeKeyDistance = "distance";
const char *kNodeKeyAngle = "angle";
const char *kNodeKeyTop = "top";
const char *kNodeKeyDetector = "detector";
const char *kNodeKeyTarget = "target";
const char *kNodeKeyType = "type";
const char *kNodeKeySize = "size";
const char *kNodeKeyOffset = "offset";
const char *kNodeKeyStrip = "strip";
const char *kNodeKeyIsGas = "is_gas";
const char *kNodeKeyZ = "z_position";
const char *kNodeKeyPedestal = "pedestal";
} // namespace

////////////////////////////////////////////////////////////////////////////////
/// In the steering file, we set two parameters,
///
/// - "FileName": geometry file, like prm/geo/expname.yaml
/// - "Visible": make detector geometry figure
///
/// in the source code, the geometry parameter objects
/// can be used by the name of "prm_detector" or "prm_target"
/// in the constructor of other processor, you can get this
/// object by
/// ```
/// RegisterOptionalInputInfo("DetectorParameter", "name of telescope parameter", fDetectorParameterName,
///                            TString("prm_detectors"), &fDetectorPrm, "TClonesArray", "art::crib::TDetectorParameter");
/// ```

TUserGeoInitializer::TUserGeoInitializer() : fGeom(nullptr) {
    RegisterProcessorParameter("DetName", "parameter name of detectors", fDetPrmName, TString("prm_detectors"));
    RegisterProcessorParameter("TargetName", "parameter name of targets", fTargetPrmName, TString("prm_targets"));
    RegisterProcessorParameter("FileName", "parameter file of detector geometry", fFileName, TString(""));

    RegisterProcessorParameter("Visible", "add artemis directory or not", fIsVisible, false);
}

TUserGeoInitializer::~TUserGeoInitializer() {
    delete fGeom;
    delete fDetParameterArray;
    delete fTargetParameterArray;
    fGeom = nullptr;
    fDetParameterArray = nullptr;
    fTargetParameterArray = nullptr;
}

void TUserGeoInitializer::Init(TEventCollection *col) {
    Info("Init", "geometry parameter is loaded from %s", fFileName.Data());
    if (gGeoManager) {
        delete gGeoManager;
    }
    fGeom = new TGeoManager("geometry", "Detector Geometry");

    Info("Init", "detector parameters are produced to %s", fDetPrmName.Data());
    fDetParameterArray = new TClonesArray("art::crib::TDetectorParameter");

    Info("Init", "target parameters are produced to %s", fTargetPrmName.Data());
    fTargetParameterArray = new TClonesArray("art::crib::TTargetParameter");

    GeometryFromYaml(fFileName);

    col->Add("geom", fGeom, fOutputIsTransparent);

    fDetParameterArray->SetName(fDetPrmName);
    col->AddInfo(fDetPrmName, fDetParameterArray, fOutputIsTransparent);

    fTargetParameterArray->SetName(fTargetPrmName);
    col->AddInfo(fTargetPrmName, fTargetParameterArray, fOutputIsTransparent);
}

void TUserGeoInitializer::Process() {}

void TUserGeoInitializer::GeometryFromYaml(TString yamlfile) {
    FileStat_t info;
    if (gSystem->GetPathInfo(yamlfile.Data(), info) != 0) {
        SetStateError(Form("File %s does not exist.", yamlfile.Data()));
        return;
    }

    YAML::Node yaml_all = YAML::LoadFile(yamlfile.Data());

#if 0
  YAML::Emitter emitter;
  emitter << yaml_all;
  std::cout << emitter.c_str() << std::endl;
  std::cout << yaml_all.size() << std::endl;
#endif

    // TGeo material setting
    YAML::Node yaml_mat = yaml_all[kNodeKeyMaterial].as<YAML::Node>();
    std::vector<TGeoMaterial *> mat_vec;
    std::vector<TGeoMedium *> med_vec;
    for (decltype(yaml_mat.size()) i = 0; i < yaml_mat.size(); i++) {
        TString name = yaml_mat[i][kNodeKeyName].as<std::string>();
        Double_t atm_mass = yaml_mat[i][kNodeKeyAMass].as<double>();
        Double_t atm_num = yaml_mat[i][kNodeKeyANum].as<double>();
        Double_t density = yaml_mat[i][kNodeKeyDensity].as<double>();
        mat_vec.emplace_back(new TGeoMaterial(name.Data(), atm_mass, atm_num, density));
        med_vec.emplace_back(new TGeoMedium(name.Data(), i + 1, mat_vec[mat_vec.size() - 1]));
    }

    // TGeo top setting
    YAML::Node yaml_top = yaml_all[kNodeKeyVolume][kNodeKeyTop].as<YAML::Node>();
    TString top_name = yaml_top[kNodeKeyName].as<std::string>();
    Int_t top_mat_id = yaml_top[kNodeKeyMaterial].as<int>();
    DoubleVec_t top_size = yaml_top[kNodeKeySize].as<std::vector<double>>();
    if (top_size.size() != 3) {
        SetStateError("input yaml error");
        return;
    }

    TGeoVolume *top =
        fGeom->MakeBox(top_name.Data(), med_vec[top_mat_id], top_size[0] / 2., top_size[1] / 2., top_size[2] / 2.);
    fGeom->SetTopVolume(top);

    // TGeo detector setting
    YAML::Node yaml_prm = yaml_all[kNodeKeyConposition][kNodeKeyDetector].as<YAML::Node>();
    YAML::Node yaml_det = yaml_all[kNodeKeyVolume][kNodeKeyDetector].as<YAML::Node>();
    if (yaml_prm.size() != yaml_det.size()) {
        SetStateError("in yaml, conposition number and detector number is different");
        return;
    }

    for (decltype(yaml_det.size()) i = 0; i < yaml_det.size(); i++) {
        if (yaml_prm[i][kNodeKeyName].as<std::string>() != yaml_det[i][kNodeKeyName].as<std::string>()) {
            SetStateError("in yaml, set same order for composition and detector");
            return;
        }
        TString det_name = yaml_det[i][kNodeKeyName].as<std::string>();
        Int_t det_mat_id = yaml_det[i][kNodeKeyMaterial].as<int>();
        DoubleVec_t det_size = yaml_det[i][kNodeKeySize].as<std::vector<double>>();
        if (det_size.size() != 3) {
            SetStateError("input yaml error, detector volume size must be 3D");
            return;
        }

        // currently only support box type detector
        if (yaml_det[i][kNodeKeyType].as<std::string>() != "box") {
            SetStateError("Currently only support only box type");
            return;
        }

        TGeoVolume *det =
            fGeom->MakeBox(det_name.Data(), med_vec[det_mat_id], det_size[0] / 2., det_size[1] / 2., det_size[2] / 2.);
        DoubleVec_t rot_point = yaml_prm[i][kNodeKeyCenterRot].as<std::vector<double>>();
        DoubleVec_t offset = yaml_prm[i][kNodeKeyOffset].as<std::vector<double>>();
        IntVec_t det_strip = yaml_prm[i][kNodeKeyStrip].as<std::vector<int>>();
        if (rot_point.size() != 3 || offset.size() != 3 || det_strip.size() != 2) {
            SetStateError("input yaml error, detector conposition setting is wrong");
            return;
        }
        Double_t distance = yaml_prm[i][kNodeKeyDistance].as<double>();
        Double_t angle = yaml_prm[i][kNodeKeyAngle].as<double>() * deg2rad;

        TVector3 det_pos(offset[0], offset[1], distance + offset[2]);
        det_pos.RotateY(angle);
        TVector3 rot_point_vec(rot_point[0], rot_point[1], rot_point[2]);
        det_pos += rot_point_vec;

        TGeoCombiTrans *det_trans =
            new TGeoCombiTrans(det_pos.X(), det_pos.Y(), det_pos.Z(), new TGeoRotation("rot", 90.0, angle / deg2rad, 0.0));
        top->AddNode(det, i, det_trans);

        // parameter input
        TDetectorParameter *prm = static_cast<TDetectorParameter *>(fDetParameterArray->ConstructedAt(i));
        DoubleVec_t thickness = yaml_prm[i][kNodeKeyThickness].as<std::vector<double>>();
        DoubleVec_t pedestal = yaml_prm[i][kNodeKeyPedestal].as<std::vector<double>>();
        if (thickness.size() != pedestal.size()) {
            SetStateError("input yaml error, thickness and pedestal array size are different");
            return;
        }
        std::vector<std::string> material = yaml_prm[i][kNodeKeyMaterial].as<std::vector<std::string>>();
        StringVec_t material_vec;
        for (decltype(thickness.size()) j = 0; j < thickness.size(); j++) {
            if (material.size() == 1) {
                material_vec.emplace_back(material[0]);
            } else if (material.size() == thickness.size()) {
                material_vec.emplace_back(material[j]);
            } else {
                SetStateError("input yaml error, composition thickness and material is wrong");
                return;
            }
        }

        if (thickness.size() != material_vec.size()) {
            SetStateError("something wrong happen...");
            return;
        }

        prm->SetDetName(det_name);
        prm->SetN(thickness.size());
        prm->SetMaxRadius(top_size[2]);
        prm->SetMaterial(material_vec);
        prm->SetThickness(thickness);
        prm->SetPedestal(pedestal);
        prm->SetCenterRotPos(rot_point);
        prm->SetOffset(offset);
        prm->SetDistance(distance);
        prm->SetAngle(angle); // radian
        prm->SetSize(det_size);
        prm->SetStripNum(det_strip);
    }

    // targets setting
    YAML::Node yaml_target = yaml_all[kNodeKeyConposition][kNodeKeyTarget].as<YAML::Node>();
    for (decltype(yaml_target.size()) i = 0; i < yaml_target.size(); i++) {
        TTargetParameter *prm = static_cast<TTargetParameter *>(fTargetParameterArray->ConstructedAt(i));
        TString name = yaml_target[i][kNodeKeyName].as<std::string>();
        Bool_t is_gas = yaml_target[i][kNodeKeyIsGas].as<bool>();
        Double_t z = yaml_target[i][kNodeKeyZ].as<double>();
        Double_t thickness = yaml_target[i][kNodeKeyThickness].as<double>();

        prm->SetTargetName(name);
        prm->SetIsGasState(is_gas);
        prm->SetZ(z);
        prm->SetThickness(thickness);
    }

    fGeom->CloseGeometry();
    fGeom->SetTopVisible();
    top->SetLineColor(kRed);

    if (fIsVisible) {
        gDirectory->Add(top);
    }
}
