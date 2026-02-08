# CMakeList.txt : 2BinProcessSpeedComp の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#

# ソースをこのプロジェクトの実行可能ファイルに追加します。
add_executable(2BinProcessSpeedComp
	src/2BinProcessSpeedComp.cpp
	src/2BinProcess.cpp
	src/threshold.cpp
	include/2BinProcessSpeedComp.h
	include/main.h
	include/simd.h
)


set_property(TARGET 2BinProcessSpeedComp PROPERTY CXX_STANDARD 20)
set_property(TARGET 2BinProcessSpeedComp PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(2BinProcessSpeedComp PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET 2BinProcessSpeedComp PROPERTY CXX_STANDARD 20)
endif()

# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。

# C++20
set_property(TARGET 2BinProcessSpeedComp PROPERTY CXX_STANDARD 20)
set_property(TARGET 2BinProcessSpeedComp PROPERTY CXX_STANDARD_REQUIRED ON)

# 自分のヘッダーを #include "2BinProcessSpeedComp.h" のように書けるようにする
target_include_directories(2BinProcessSpeedComp PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# OpenCV
find_package(OpenCV CONFIG REQUIRED)

target_link_libraries(2BinProcessSpeedComp PRIVATE ${OpenCV_LIBS})
target_include_directories(2BinProcessSpeedComp PRIVATE ${OpenCV_INCLUDE_DIRS})

# OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  target_link_libraries(2BinProcessSpeedComp PRIVATE OpenMP::OpenMP_CXX)
else()
  message(WARNING "OpenMP not found. Building without OpenMP.")
endif()

# AVX2
if (MSVC)
  target_compile_options(2BinProcessSpeedComp PRIVATE /arch:AVX2)
else()
  target_compile_options(2BinProcessSpeedComp PRIVATE -mavx2)
endif()