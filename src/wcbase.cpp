#include "wcbase.hpp"

void get_data(bootstrap::serial& serial, nlohmann::json& result) {
    serial.write(std::to_string(result["deviceId"].get<int>()));

    std::string reading;
    char c;

    while ((c = serial.read_char()) != END_CHAR) {
        reading.push_back(c);
    }

    if (reading.find("error") == -1) {
        result["record"] = std::stod(reading);
    } else {
        result["type"] = "error";
    }
}

void mock_data(nlohmann::json& result) {
    result["record"] = rand() % 100;

    std::this_thread::sleep_for(std::chrono::seconds(5));
}

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

    auto const socket_uri = (*env)["socketUri"];

    if (socket_uri.is_null()) {
        std::cerr << "No 'socketUri' provided in .env.json\n";
        return 0;
    }

    std::string jwt;

    try {
        jwt = bootstrap::get_jwt(*env);
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 0;
    }

    std::unique_ptr<bootstrap::serial> serial(nullptr);

    try {
        serial = bootstrap::make_serial(*env);
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }

    socket_endpoint endpoint;
    std::mutex request_stack_mutex;
    std::stack<nlohmann::json> request_stack;

    endpoint.set_on_connect_listener([&]() {
        if (endpoint.get_metadata()->get_status() !=
            connection_metadata::connection_status::open) {
            std::cerr << "Couldn't connect to specified 'uri'\n";
            endpoint.close();
            return;
        }

        endpoint.set_on_message_listener([&](nlohmann::json const& payload) {
            std::cout << "Message received!\n" << payload << '\n';

            request_stack_mutex.lock();

            request_stack.push(payload);

            request_stack_mutex.unlock();
        });

        std::cout << "Hello, World!\n";
    });
    endpoint.connect(socket_uri, jwt);

    while (true) {
        request_stack_mutex.lock();

        if (request_stack.empty()) {
            request_stack_mutex.unlock();
        } else {
            nlohmann::json const& payload = request_stack.top();
            nlohmann::json result{{"deviceId", payload["deviceId"]},
                                  {"type", payload["type"]}};

            request_stack.pop();
            request_stack_mutex.unlock();

            if (serial) {
                get_data(*serial, result);
            } else {
                mock_data(result);
            }

            std::cout << "Sending back.\n" << result << "\n";

            endpoint.send(result);
        }
    }

    endpoint.join();

    return 0;
}
