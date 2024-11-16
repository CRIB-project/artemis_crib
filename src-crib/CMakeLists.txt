set(CRIBLIB_NAME crib)

set(CRIBSOURCES
    TF1PPACProcessor.cc
    TTimingChargeAllMappingProcessor.cc
    TTimingDataMappingProcessor.cc
    TBranchCopyProcessor.cc
    TEvtNumProcessor.cc
    TModuleData.cc
    TSegmentOutputProcessor.cc
    TScalerMonitorProcessor.cc
    # mux
    mux/TMUXCalibrationProcessor.cc
    mux/TMUXData.cc
    mux/TMUXDataMappingProcessor.cc
    mux/TMUXPositionConverter.cc
    mux/TMUXPositionValidator.cc
    # telescope
    telescope/TTelescopeData.cc
    telescope/TTelescopeProcessor.cc
    # reconst
    reconst/TReactionInfo.cc
    # commands
    commands/TCatCmdLoopStart.cc
    commands/TCatCmdLoopStop.cc
    commands/TCmdXfitg.cc
    commands/TCmdXstatus.cc
    commands/TCmdXYblow.cc
    commands/TCmdXblow.cc
    commands/TCatCmdTCutG.cc
    commands/TCmdErase.cc
    commands/TCmdDraw.cc
    # geo
    geo/TUserGeoInitializer.cc
    geo/TDetectorParameter.cc
    geo/TTargetParameter.cc
    # simulation
    simulation/TTreePeriodicEventStore.cc
    simulation/TParticleInfo.cc
    simulation/TSolidAngleProcessor.cc
    # timestamp
    timestamp/TTSData.cc
    timestamp/TTSMappingProcessor.cc
    timestamp/TModuleDecoderTimestamp.cc)

set(CRIBHEADERS
    TF1PPACProcessor.h
    TTimingChargeAllMappingProcessor.h
    TTimingDataMappingProcessor.h
    TBranchCopyProcessor.h
    TEvtNumProcessor.h
    TModuleData.h
    TSegmentOutputProcessor.h
    TScalerMonitorProcessor.h
    # mux
    mux/TMUXCalibrationProcessor.h
    mux/TMUXData.h
    mux/TMUXDataMappingProcessor.h
    mux/TMUXPositionConverter.h
    mux/TMUXPositionValidator.h
    # telescope
    telescope/TTelescopeData.h
    telescope/TTelescopeProcessor.h
    # reconst
    reconst/TReactionInfo.h
    # commands
    commands/TCatCmdLoopStart.h
    commands/TCatCmdLoopStop.h
    commands/TCmdXfitg.h
    commands/TCmdXstatus.h
    commands/TCmdXYblow.h
    commands/TCmdXblow.h
    commands/TCatCmdTCutG.h
    commands/TCmdErase.h
    commands/TCmdDraw.h
    # geo
    geo/TUserGeoInitializer.h
    geo/TDetectorParameter.h
    geo/TTargetParameter.h
    # simulation
    simulation/TTreePeriodicEventStore.h
    simulation/TParticleInfo.h
    simulation/TSolidAngleProcessor.h
    # timestamp
    timestamp/TTSData.h
    timestamp/TTSMappingProcessor.h
    timestamp/TModuleDecoderTimestamp.h)

if(TSrim_FOUND)
  list(
    APPEND
    CRIBSOURCES
    reconst/TTGTIKProcessor.cc
    reconst/TReconstProcessor.cc
    simulation/TDetectParticleProcessor.cc
    simulation/TNBodyReactionProcessor.cc
    simulation/TTreeBeamGenerator.cc
    simulation/TRandomBeamGenerator.cc)
  list(
    APPEND
    CRIBHEADERS
    reconst/TTGTIKProcessor.h
    reconst/TReconstProcessor.h
    simulation/TDetectParticleProcessor.h
    simulation/TNBodyReactionProcessor.h
    simulation/TTreeBeamGenerator.h
    simulation/TRandomBeamGenerator.h)
endif()

add_library(${CRIBLIB_NAME} SHARED ${CRIBSOURCES})
target_compile_features(${CRIBLIB_NAME} PRIVATE cxx_std_17)
target_compile_options(${CRIBLIB_NAME} PRIVATE -Wall -Wextra -O2)

target_include_directories(
  ${CRIBLIB_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                         $<INSTALL_INTERFACE:include>)

target_link_libraries(
  ${CRIBLIB_NAME}
  PRIVATE ${ROOT_LIBRARIES}
          ${YAML_CPP_LIBRARIES}
          artemis::catcore
          artemis::catloop
          artemis::artcont
          artemis::catcmd
          artemis::catmc
          artemis::artdecoder
          artemis::artridf
          artemis::artrdf
          artemis::arthist
          artemis::artparam
          artemis::artshare
          artemis::CAT)

if(TSrim_FOUND)
  target_link_libraries(${CRIBLIB_NAME} PRIVATE TSrim::Srim)
endif()

root_generate_dictionary(G__${CRIBLIB_NAME} ${CRIBHEADERS} LINKDEF
                         "artcrib_linkdef.h" MODULE ${CRIBLIB_NAME})

set_target_properties(
  ${CRIBLIB_NAME}
  PROPERTIES VERSION ${PROJECT_VERSION}
             SOVERSION ${PROJECT_VERSION_MAJOR}
             PUBLIC_HEADER "${CRIBHEADERS}")

install(
  TARGETS ${CRIBLIB_NAME}
  EXPORT cribTargets
  INCLUDES
  DESTINATION include
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/src-crib
  LIBRARY DESTINATION lib)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${CRIBLIB_NAME}_rdict.pcm
        DESTINATION lib)
