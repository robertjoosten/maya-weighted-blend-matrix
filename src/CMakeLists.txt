set(SOURCE_FILES
    "main.cpp"
    "weightedBlendMatrixNode.h"
    "weightedBlendMatrixNode.cpp"
)

add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:DEBUG>>:/bigobj>")

find_package(Maya REQUIRED)
find_package(Eigen3 REQUIRED)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE Maya::Maya Eigen3::Eigen)
target_include_directories(${PROJECT_NAME} 
    PRIVATE Maya::Maya Eigen3::Eigen
    PUBLIC "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
)
MAYA_PLUGIN(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} ${MAYA_TARGET_TYPE} DESTINATION plug-ins/${MAYA_VERSION})