#pragma once

#include "lib/bootstrap.hpp"
#include "lib/socket_helpers.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
    #include <windows.h>
#endif
