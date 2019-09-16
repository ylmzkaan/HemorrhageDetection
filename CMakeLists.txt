cmake_minimum_required(VERSION 3.9.5)

project(BrainDamageAnalysis)

set(SimpleITK_DIR ~/Projects/SimpleITK-build/lib/cmake/SimpleITK-1.3)
set(ITK_DIR ~/Projects/SimpleITK-build/ITK-build)

find_package(SimpleITK REQUIRED)
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

set(INCLUDE_DIR includes/)
set(LIB_DIR lib/)
include_directories(${INCLUDE_DIR})
include_directories(${LIB_DIR})

file(GLOB LIB ${LIB_DIR}/*.cpp ${INCLUDE_DIR}/*.h ${INCLUDE_DIR}/Masking/*.h ${INCLUDE_DIR}/Masking/MaskingMethods/*.h)

add_executable(BrainDamageAnalysis src/BrainDamageAnalysis.cpp ${LIB})

target_link_libraries(BrainDamageAnalysis ${SimpleITK_LIBRARIES} ${ITK_LIBRARIES})
