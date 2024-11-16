#ifndef LINKDEF_CRIB_H
#define LINKDEF_CRIB_H

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

// segment and category
// main
#pragma link C++ class art::crib::TF1PPACProcessor;
#pragma link C++ class art::crib::TTimingChargeAllMappingProcessor;
#pragma link C++ class art::crib::TTimingDataMappingProcessor;
#pragma link C++ class art::crib::TBranchCopyProcessor;
#pragma link C++ class art::crib::TEvtNumProcessor;
#pragma link C++ class art::crib::TModuleData + ;
#pragma link C++ class art::crib::TSegmentOutputProcessor;
#pragma link C++ class art::crib::TScalerMonitorProcessor;
// MUX
#pragma link C++ class art::crib::TMUXData + ;
#pragma link C++ class art::crib::TMUXDataMappingProcessor;
#pragma link C++ class art::crib::TMUXPositionConverter;
#pragma link C++ class art::crib::TMUXPositionValidator;
#pragma link C++ class art::crib::TMUXCalibrationProcessor;
// telescope
#pragma link C++ class art::crib::TTelescopeData + ;
#pragma link C++ class art::crib::TTelescopeProcessor;
// reconst
#pragma link C++ class art::crib::TReconstProcessor;
#pragma link C++ class art::crib::TTGTIKProcessor;
#pragma link C++ class art::crib::TReactionInfo + ;
//  commands
#pragma link C++ class art::crib::TCatCmdLoopStart;
#pragma link C++ class art::crib::TCatCmdLoopStop;
#pragma link C++ class art::crib::TCmdXfitg;
#pragma link C++ class art::crib::TCmdXstatus;
#pragma link C++ class art::crib::TCmdXYblow;
#pragma link C++ class art::crib::TCmdXblow;
#pragma link C++ class art::crib::TCatCmdTCutG;
#pragma link C++ class art::crib::TCmdErase;
#pragma link C++ class art::crib::TCmdDraw;
// geo
#pragma link C++ class art::crib::TUserGeoInitializer;
#pragma link C++ class art::crib::TDetectorParameter + ;
#pragma link C++ class art::crib::TTargetParameter + ;
// simulation
#pragma link C++ class art::crib::TTreePeriodicEventStore;
#pragma link C++ class art::crib::TDetectParticleProcessor;
#pragma link C++ class art::crib::TNBodyReactionProcessor;
#pragma link C++ class art::crib::TParticleInfo + ;
#pragma link C++ class art::crib::TRandomBeamGenerator;
#pragma link C++ class art::crib::TTreeBeamGenerator;
#pragma link C++ class art::crib::TSolidAngleProcessor;
// #pragma link C++ class art::crib::TRutherfordScattering;
// timestamp
#pragma link C++ class art::crib::TTSData + ;
#pragma link C++ class art::crib::TTSMappingProcessor;
#pragma link C++ class art::crib::TModuleDecoderTimestamp;
#endif // __CINT__

#endif // LINKDEF_CRIB_H
