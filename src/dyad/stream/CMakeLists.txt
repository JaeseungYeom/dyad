set(DYAD_FSTREAM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/dyad_stream_core.cpp)
set(DYAD_FSTREAM_PRIVATE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/dyad/common/dyad_rc.h
                                 ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/dyad/common/dyad_dtl.h
                                 ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/dyad/core/dyad_ctx.h
                                 ${CMAKE_CURRENT_SOURCE_DIR}/../client/dyad_client_int.h
                                 ${CMAKE_CURRENT_SOURCE_DIR}/../utils/utils.h)
set(DYAD_FSTREAM_PUBLIC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/dyad/stream/dyad_stream_api.hpp
                                ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/dyad/stream/dyad_params.hpp
                                ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/dyad/stream/dyad_stream_core.hpp)


add_library(${PROJECT_NAME}_fstream SHARED ${DYAD_FSTREAM_SRC}
            ${DYAD_FSTREAM_PRIVATE_HEADERS} ${DYAD_FSTREAM_PUBLIC_HEADERS})
target_compile_features(${PROJECT_NAME}_fstream PRIVATE)
target_link_libraries(${PROJECT_NAME}_fstream PRIVATE ${PROJECT_NAME}_client
                                                      ${PROJECT_NAME}_utils flux::core)

if (ENABLE_PERFFLOW)
    target_link_libraries(${PROJECT_NAME}_fstream PRIVATE perfflowaspect::perfflowaspect)
    target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${perfflowaspect_INCLUDE_DIRS})
    target_compile_definitions(${PROJECT_NAME}_fstream PRIVATE DYAD_PERFFLOW=1)
endif (ENABLE_PERFFLOW)

target_compile_definitions(${PROJECT_NAME}_fstream PUBLIC DYAD_HAS_CONFIG)
target_include_directories(${PROJECT_NAME}_fstream PUBLIC
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${DYAD_INSTALL_INCLUDEDIR}>)
target_include_directories(${PROJECT_NAME}_fstream SYSTEM PRIVATE ${FluxCore_INCLUDE_DIRS})

dyad_add_werror_if_needed(${PROJECT_NAME}_fstream)
if(DYAD_PROFILER STREQUAL "DFTRACER")
    target_link_libraries(${PROJECT_NAME}_fstream PRIVATE ${DFTRACER_LIBRARIES})
endif()

install(
        TARGETS ${PROJECT_NAME}_fstream
        EXPORT ${DYAD_EXPORTED_TARGETS}
        LIBRARY DESTINATION ${DYAD_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${DYAD_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${DYAD_INSTALL_BINDIR}
)
if(NOT "${DYAD_FSTREAM_PUBLIC_HEADERS}" STREQUAL "")
    dyad_install_headers("${DYAD_FSTREAM_PUBLIC_HEADERS}" ${CMAKE_CURRENT_SOURCE_DIR})
endif()
