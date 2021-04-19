#include "wcbase.hpp"

int main(int argc, char* argv[]) {
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::unique_ptr<nlohmann::json> env;

    try {
        env = std::unique_ptr<nlohmann::json>(check_dotenv());
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return 0;
    }

    socket_endpoint endpoint;

    endpoint.connect((*env)["uri"]);

    endpoint.set_on_connect_listener([&endpoint]() {
        if (endpoint.get_metadata()->get_status() !=
            connection_metadata::connection_status::open) {
            std::cerr << "Couldn't connect to specified 'uri' from .env.json.\n"
                         "Or maybe you just forgot to set it? ;)\n";
            endpoint.close();
            return;
        }

        std::cout << "Hello, World!\n";

        endpoint.close();
    });

    endpoint.join();

    return 0;
}
