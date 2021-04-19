#pragma once

#include "nlohmann/json.hpp"

#include <exception>
#include <fstream>
#include <memory>

auto check_dotenv() {
    std::ifstream dotenv("./.env.json");

    if (dotenv.fail()) {
        throw std::runtime_error(".env.json file was not found.");
    }

    auto env = std::make_unique<nlohmann::json>();

    dotenv >> *env;
    dotenv.close();

    return env;
}