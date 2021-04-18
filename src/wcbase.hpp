#pragma once

#include "nlohmann/json.hpp"
#include "websocketpp/client.hpp"
#include "websocketpp/config/asio_no_tls_client.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
    #include <Windows.h>
#endif
