#pragma once

#include "websocketpp/client.hpp"
#include "websocketpp/config/asio_no_tls_client.hpp"

#include <memory>

typedef websocketpp::client<websocketpp::config::asio_client> socket_client;

class connection_metadata {
   public:
    enum connection_status { connecting, open, failed, closed };

   private:
    int id;
    websocketpp::connection_hdl handle;
    connection_status status;
    std::string const uri;
    std::string server;
    std::string error_reason;

   public:
    connection_metadata(int id,
                        websocketpp::connection_hdl handle,
                        std::string uri) :
        id(id),
        handle(handle), uri(uri), status(connecting), server("N/A") {}

    auto get_handle() {
        return handle;
    }

    auto get_status() {
        return status;
    }

    void on_open(socket_client* client,
                 websocketpp::connection_hdl arg_handle) {
        auto connection = client->get_con_from_hdl(arg_handle);
        status          = open;
        server          = connection->get_response_header("Server");
    }

    void on_fail(socket_client* client,
                 websocketpp::connection_hdl arg_handle) {
        auto connection = client->get_con_from_hdl(arg_handle);
        status          = failed;
        server          = connection->get_response_header("Server");
        error_reason    = connection->get_ec().message();
    }

    void on_close(socket_client* client,
                  websocketpp::connection_hdl arg_handle) {
        auto connection = client->get_con_from_hdl(arg_handle);
        status          = closed;
        server          = connection->get_response_header("Server");
        error_reason    = connection->get_ec().message();
    }
};

class socket_endpoint {
    socket_client endpoint;
    std::shared_ptr<std::thread> thread;
    std::shared_ptr<connection_metadata> metadata                  = nullptr;
    std::function<void()>* on_connect                              = nullptr;
    std::function<void(nlohmann::json const& payload)>* on_message = nullptr;

   public:
    static constexpr auto CONNECTION_ERROR = -1;

    socket_endpoint() {
        endpoint.clear_access_channels(websocketpp::log::alevel::all);
        endpoint.clear_error_channels(websocketpp::log::elevel::all);

        endpoint.init_asio();
        endpoint.start_perpetual();

        thread.reset(new std::thread(&socket_client::run, &endpoint));
    }

    ~socket_endpoint() {
        if (metadata != nullptr) {
            close();
        }

        delete on_connect;
    }

    auto connect(std::string const& uri, std::string const& jwt = "") {
        std::error_code err;

        socket_client::connection_ptr connection =
            endpoint.get_connection(uri, err);

        if (err) {
            return CONNECTION_ERROR;
        }

        metadata = std::shared_ptr<connection_metadata>(
            new connection_metadata(1, connection->get_handle(), uri));

        connection->set_open_handler(
            std::bind(&socket_endpoint::on_open,
                      this,
                      websocketpp::lib::placeholders::_1));

        connection->set_fail_handler(
            std::bind(&socket_endpoint::on_fail,
                      this,
                      websocketpp::lib::placeholders::_1));

        connection->set_message_handler(
            std::bind(&socket_endpoint::on_message_handler,
                      this,
                      websocketpp::lib::placeholders::_1,
                      websocketpp::lib::placeholders::_2));

        if (!jwt.empty()) {
            connection->append_header("Authorization",
                                      std::string("Bearer ") + jwt);
        }

        endpoint.connect(connection);

        return 1;
    }

    void close(uint16_t code = 0) {
        std::error_code error_code;

        endpoint.stop_perpetual();
        endpoint.close(metadata->get_handle(), code, "", error_code);

        metadata = nullptr;
    }

    auto get_metadata() {
        return metadata;
    }

    void set_on_connect_listener(std::function<void()> new_listener) {
        on_connect = new std::function(new_listener);
    }

    void set_on_message_listener(
        std::function<void(nlohmann::json const& payload)> new_listener) {
        on_message = new std::function(new_listener);
    }

    void send(nlohmann::json const& message) {
        std::error_code ec;

        endpoint.send(metadata->get_handle(),
                      message.dump(),
                      websocketpp::frame::opcode::text,
                      ec);

        if (ec) {
            throw std::runtime_error(ec.message());
        }
    }

    void join() {
        thread->join();
    }

   private:
    void on_open(websocketpp::connection_hdl arg_handle) {
        this->metadata->on_open(&this->endpoint, arg_handle);

        if (on_connect != nullptr) {
            (*on_connect)();
        }
    }

    void on_fail(websocketpp::connection_hdl arg_handle) {
        this->metadata->on_fail(&this->endpoint, arg_handle);

        if (on_connect != nullptr) {
            (*on_connect)();
        }
    }

    void on_message_handler(
        websocketpp::connection_hdl arg_handle,
        std::shared_ptr<websocketpp::config::core_client::message_type> mt) {
        if (on_message != nullptr) {
            (*on_message)(nlohmann::json::parse(mt->get_payload()));
        }
    }
};