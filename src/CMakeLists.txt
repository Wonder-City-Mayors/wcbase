cmake_minimum_required (VERSION 3.8)

include("${PROJECT_SOURCE_DIR}/include/conan/conanbuildinfo.cmake")
conan_basic_setup(TARGETS)

add_subdirectory(lib)

add_executable (
	wcbase
	wcbase.cpp
	wcbase.hpp
)

set_property(TARGET wcbase PROPERTY CXX_STANDARD 20)

target_include_directories(
	wcbase
	PUBLIC ${PROJECT_BINARY_DIR}
)

target_link_libraries(
	wcbase
	CONAN_PKG::websocketpp
	CONAN_PKG::boost
	CONAN_PKG::nlohmann_json
	CONAN_PKG::serial
	socket_helpers
)
