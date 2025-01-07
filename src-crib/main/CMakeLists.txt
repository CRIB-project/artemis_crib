# custom hadd
set(CUSTOM_HADD_NAME chadd)
add_executable(${CUSTOM_HADD_NAME} custom_hadd.cpp)
target_compile_features(${CUSTOM_HADD_NAME} PRIVATE cxx_std_17)
target_compile_options(${CUSTOM_HADD_NAME} PRIVATE -Wall -Wextra -O2)
target_link_libraries(
  ${CUSTOM_HADD_NAME}
  PUBLIC ${ROOT_LIBRARIES}
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
         artemis::CAT
         ${CRIBLIB_NAME})

install(
  TARGETS ${CUSTOM_HADD_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
