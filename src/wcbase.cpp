#include "wcbase.hpp"

int main(int argc, char* argv[]) {
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
    SetConsoleOutputCP(CP_UTF8);
#endif

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

    endpoint.set_on_connect_listener([&endpoint]() {
        if (endpoint.get_metadata()->get_status() !=
            connection_metadata::connection_status::open) {
            std::cerr << "Couldn't connect to specified 'uri'\n";
            endpoint.close();
            return;
        }

        std::cout << "Hello, World!\n";

        endpoint.close();
    });
    endpoint.connect(socket_uri, jwt);
    endpoint.join();

    return 0;
}
