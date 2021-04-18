#include "wcbase.hpp"

typedef websocketpp::client<websocketpp::config::asio_client> socket_client;

auto check_dotenv() {
    std::ifstream dotenv("./.env.json");

    if (dotenv.fail()) {
        throw std::runtime_error(".env.json file was not found.");
    }

    auto env = std::make_unique<nlohmann::json>();

    dotenv >> *env;

    return env;
}

int main(int argc, char* argv[]) {
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::unique_ptr<nlohmann::json> env;

    try {
        env = std::unique_ptr<nlohmann::json>(check_dotenv());
    } catch (std::exception& e) { std::cerr << e.what() << '\n'; }

    /*
     * А чё дальше-то?
     */

    return 0;
}
