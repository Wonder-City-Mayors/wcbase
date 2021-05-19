cmake_minimum_required (VERSION 3.15)

project ("wcbase" VERSION 0.1 LANGUAGES CXX)

if(MSVC)
	add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
endif()

add_subdirectory ("src")
