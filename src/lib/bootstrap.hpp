#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "httplib.h"
#include "nlohmann/json.hpp"

#include <cstdlib>
#include <ctime>
#include <exception>
#include <fstream>
#include <memory>
#include <string>

namespace bootstrap {
    auto check_dotenv() {
        std::ifstream dotenv(".env.json");

        if (dotenv.fail()) {
            throw std::runtime_error(".env.json file was not found.");
        }

        auto env = std::make_unique<nlohmann::json>();

        dotenv >> *env;
        dotenv.close();

        return env;
    }

    auto get_jwt(nlohmann::json const& env) {
        auto api_uri = env["api_uri"];

        if (api_uri.is_null()) {
            throw std::runtime_error("No 'api_uri' field in .env.json.");
        }

        nlohmann::json body;
        body["id"]       = env["id"].get<int>();
        body["password"] = env["password"];

        httplib::Client client(static_cast<std::string>(api_uri).c_str());

        auto res =
            client.Post("/auth/signStation", body.dump(), "application/json");
        body = nlohmann::json::parse(res->body);

        if (res->status != 200) {
            throw std::runtime_error(body["message"].dump());
        }

        return body["jwt"].get<std::string>();
    }

    auto setup_random() {
        srand(time(NULL));
    }
}    // namespace bootstrap