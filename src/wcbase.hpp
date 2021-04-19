#pragma once

#include "lib/json_helpers.hpp"
#include "lib/socket_helpers.hpp"

#include <iostream>

#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
    #include <Windows.h>
#endif
