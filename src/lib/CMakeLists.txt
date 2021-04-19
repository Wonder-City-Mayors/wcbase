cmake_minimum_required (VERSION 3.8)

add_library(
	socket_helpers
	socket_helpers.hpp
)

add_library(
	json_helpers
	json_helpers.hpp
)

set_target_properties(socket_helpers PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(json_helpers PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(
	socket_helpers
	CONAN_PKG::websocketpp
)

target_link_libraries(
	json_helpers
	CONAN_PKG::nlohmann_json
)