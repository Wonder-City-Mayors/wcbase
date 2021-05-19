#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "boost/asio.hpp"
#include "httplib.h"
#include "nlohmann/json.hpp"

#include <cstdlib>
#include <ctime>
#include <exception>
#include <fstream>
#include <memory>
#include <string>

namespace bootstrap {
    class serial : public boost::asio::serial_port {
       private:
        boost::asio::io_context io;

       public:
        serial(std::string const& name) : boost::asio::serial_port(io) {
            open(name);
            set_option(boost::asio::serial_port_base::baud_rate(9600));
        }

        char read_char() {
            char c;

            boost::asio::read(*this, boost::asio::buffer(&c, 1));

            return c;
        }

        void write(std::string const& value) {
            write_some(boost::asio::buffer(value.c_str(), value.size()));
        }
    };

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
        auto api_uri = env["apiUri"];

        if (api_uri.is_null()) {
            throw std::runtime_error("No 'apiUri' field in .env.json.");
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

    auto make_serial(nlohmann::json const& env) {
        std::unique_ptr<serial> new_serial(nullptr);

        auto it = env.find("serialPortName");

        if (it != env.end()) {
            new_serial = std::make_unique<serial>(it->get<std::string>());
        }

        return new_serial;
    }

    auto setup_random() {
        srand(time(nullptr));
    }
}    // namespace bootstrap