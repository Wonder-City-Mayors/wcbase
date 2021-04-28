#include "wcbase.hpp"

int main(int argc, char* argv[]) {
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
    SetConsoleOutputCP(CP_UTF8);
#endif
    bootstrap::setup_random();

    std::unique_ptr<nlohmann::json> env;

    try {
        env = std::unique_ptr(bootstrap::check_dotenv());
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return 0;
    }

    auto const socket_uri = (*env)["socket_uri"];

    if (socket_uri.is_null()) {
        std::cerr << "No 'socket_uri' provided in .env.json\n";
        return 0;
    }

    std::string jwt;

    try {
        jwt = bootstrap::get_jwt(*env);
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 0;
    }

    socket_endpoint endpoint;
    std::thread* awaiting = nullptr;

    endpoint.set_on_connect_listener([&]() {
        if (endpoint.get_metadata()->get_status() !=
            connection_metadata::connection_status::open) {
            std::cerr << "Couldn't connect to specified 'uri'\n";
            endpoint.close();
            return;
        }

        endpoint.set_on_message_listener([&](nlohmann::json const& payload) {
            std::cout << "Message received!\n" << payload << '\n';

            if (awaiting != nullptr) {
                awaiting->detach();
            }

            awaiting = new std::thread([payload, &endpoint]() {
                auto type      = payload["type"].get<std::string>();
                auto device_id = payload["deviceId"].get<int>();

                nlohmann::json someval;

                someval["deviceId"] = device_id;
                someval["record"]   = rand() % 100;
                someval["type"]     = type;

                std::this_thread::sleep_for(std::chrono::seconds(5));

                std::cout << "Sending back.\n" << someval << "\n";

                endpoint.send(someval);
            });
        });

        std::cout << "Hello, World!\n";
    });
    endpoint.connect(socket_uri, jwt);
    endpoint.join();

    return 0;
}
