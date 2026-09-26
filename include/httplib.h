#ifndef HTTPLIB_H
#define HTTPLIB_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_t;
    #define INVALID_SOCKET_VAL INVALID_SOCKET
    #define SOCKET_ERROR_VAL SOCKET_ERROR
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VAL (-1)
    #define SOCKET_ERROR_VAL (-1)
    #define CLOSE_SOCKET close
#endif

namespace httplib {

struct Request {
    std::string method;
    std::string path;
    std::string raw_path;
    std::string query_string;
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> headers;
    std::string body;

    bool has_param(const std::string& key) const {
        return params.find(key) != params.end();
    }

    std::string get_param_value(const std::string& key) const {
        auto it = params.find(key);
        if (it != params.end()) return it->second;
        return "";
    }
};

struct Response {
    int status = 200;
    std::string content_type = "text/html; charset=utf-8";
    std::string body;
    std::map<std::string, std::string> headers;

    void set_content(const std::string& s, const std::string& type) {
        body = s;
        content_type = type;
    }

    void set_redirect(const std::string& url, int code = 302) {
        status = code;
        headers["Location"] = url;
    }
};

using Handler = std::function<void(const Request&, Response&)>;

class Server {
private:
    std::map<std::string, Handler> get_routes;
    std::map<std::string, Handler> post_routes;
    std::vector<std::pair<std::string, std::string>> mount_points;
    bool is_running;
    socket_t server_socket;

    static std::string get_mime_type(const std::string& path) {
        size_t dot_pos = path.rfind('.');
        if (dot_pos == std::string::npos) return "text/plain";
        std::string ext = path.substr(dot_pos);
        for (auto& c : ext) c = tolower(c);

        if (ext == ".html" || ext == ".htm") return "text/html; charset=utf-8";
        if (ext == ".css") return "text/css; charset=utf-8";
        if (ext == ".js") return "application/javascript; charset=utf-8";
        if (ext == ".json") return "application/json; charset=utf-8";
        if (ext == ".png") return "image/png";
        if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
        if (ext == ".gif") return "image/gif";
        if (ext == ".svg") return "image/svg+xml";
        if (ext == ".ico") return "image/x-icon";
        if (ext == ".txt") return "text/plain; charset=utf-8";
        return "application/octet-stream";
    }

    static std::string url_decode(const std::string& in) {
        std::string out;
        for (size_t i = 0; i < in.length(); ++i) {
            if (in[i] == '%') {
                if (i + 2 < in.length()) {
                    int hexVal = 0;
                    std::istringstream hex_stream(in.substr(i + 1, 2));
                    if (hex_stream >> std::hex >> hexVal) {
                        out += static_cast<char>(hexVal);
                        i += 2;
                    } else {
                        out += in[i];
                    }
                }
            } else if (in[i] == '+') {
                out += ' ';
            } else {
                out += in[i];
            }
        }
        return out;
    }

    static void parse_query_params(const std::string& query, std::map<std::string, std::string>& params) {
        std::istringstream iss(query);
        std::string pair;
        while (std::getline(iss, pair, '&')) {
            if (pair.empty()) continue;
            size_t eq = pair.find('=');
            if (eq != std::string::npos) {
                std::string key = url_decode(pair.substr(0, eq));
                std::string val = url_decode(pair.substr(eq + 1));
                params[key] = val;
            } else {
                params[url_decode(pair)] = "";
            }
        }
    }

    bool serve_file(const std::string& file_path, Response& res) {
        std::ifstream file(file_path.c_str(), std::ios::binary);
        if (!file.is_open()) return false;

        std::ostringstream ss;
        ss << file.rdbuf();
        res.body = ss.str();
        res.content_type = get_mime_type(file_path);
        res.status = 200;
        return true;
    }

    void handle_client(socket_t client_socket) {
        char buffer[8192];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            CLOSE_SOCKET(client_socket);
            return;
        }
        buffer[bytes_received] = '\0';

        std::string raw_request(buffer, bytes_received);
        std::istringstream req_stream(raw_request);
        std::string req_line;
        if (!std::getline(req_stream, req_line)) {
            CLOSE_SOCKET(client_socket);
            return;
        }

        if (!req_line.empty() && req_line.back() == '\r') {
            req_line.pop_back();
        }

        std::istringstream line_stream(req_line);
        Request req;
        std::string protocol;
        line_stream >> req.method >> req.raw_path >> protocol;

        // Parse query string
        size_t query_pos = req.raw_path.find('?');
        if (query_pos != std::string::npos) {
            req.path = req.raw_path.substr(0, query_pos);
            req.query_string = req.raw_path.substr(query_pos + 1);
            parse_query_params(req.query_string, req.params);
        } else {
            req.path = req.raw_path;
        }

        // Parse headers
        std::string header_line;
        while (std::getline(req_stream, header_line) && header_line != "\r" && !header_line.empty()) {
            if (!header_line.empty() && header_line.back() == '\r') header_line.pop_back();
            size_t colon_pos = header_line.find(':');
            if (colon_pos != std::string::npos) {
                std::string h_key = header_line.substr(0, colon_pos);
                std::string h_val = header_line.substr(colon_pos + 1);
                while (!h_val.empty() && h_val.front() == ' ') h_val.erase(0, 1);
                req.headers[h_key] = h_val;
            }
        }

        // Body if any
        size_t body_start = raw_request.find("\r\n\r\n");
        if (body_start != std::string::npos) {
            req.body = raw_request.substr(body_start + 4);
        }

        Response res;
        bool handled = false;

        if (req.method == "GET") {
            auto it = get_routes.find(req.path);
            if (it != get_routes.end()) {
                it->second(req, res);
                handled = true;
            } else {
                // Check static mount points
                for (const auto& mp : mount_points) {
                    const std::string& mount_prefix = mp.first;
                    const std::string& base_dir = mp.second;

                    if (req.path.rfind(mount_prefix, 0) == 0) {
                        std::string rel_path = req.path.substr(mount_prefix.length());
                        if (rel_path.empty() || rel_path == "/") {
                            rel_path = "/index.html";
                        }
                        if (!rel_path.empty() && rel_path.front() == '/') {
                            rel_path.erase(0, 1);
                        }

                        std::string full_path = base_dir;
                        if (!full_path.empty() && full_path.back() != '/' && full_path.back() != '\\') {
                            full_path += "/";
                        }
                        full_path += rel_path;

                        if (serve_file(full_path, res)) {
                            handled = true;
                            break;
                        }
                    }
                }
            }
        } else if (req.method == "POST") {
            auto it = post_routes.find(req.path);
            if (it != post_routes.end()) {
                it->second(req, res);
                handled = true;
            }
        } else if (req.method == "OPTIONS") {
            res.status = 200;
            res.body = "";
            handled = true;
        }

        if (!handled) {
            res.status = 404;
            res.content_type = "text/plain";
            res.body = "404 Not Found";
        }

        // Send HTTP Response
        std::ostringstream response_stream;
        response_stream << "HTTP/1.1 " << res.status << " ";
        if (res.status == 200) response_stream << "OK\r\n";
        else if (res.status == 302) response_stream << "Found\r\n";
        else if (res.status == 404) response_stream << "Not Found\r\n";
        else response_stream << "Status\r\n";

        response_stream << "Content-Type: " << res.content_type << "\r\n";
        response_stream << "Content-Length: " << res.body.size() << "\r\n";
        response_stream << "Access-Control-Allow-Origin: *\r\n";
        response_stream << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        response_stream << "Access-Control-Allow-Headers: Content-Type\r\n";
        response_stream << "Connection: close\r\n";

        for (const auto& header : res.headers) {
            response_stream << header.first << ": " << header.second << "\r\n";
        }

        response_stream << "\r\n";
        response_stream << res.body;

        std::string response_str = response_stream.str();
        send(client_socket, response_str.c_str(), static_cast<int>(response_str.size()), 0);

        CLOSE_SOCKET(client_socket);
    }

public:
    Server() : is_running(false), server_socket(INVALID_SOCKET_VAL) {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }

    ~Server() {
        stop();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    bool set_mount_point(const std::string& mount_point, const std::string& dir) {
        mount_points.push_back({mount_point, dir});
        return true;
    }

    Server& Get(const std::string& pattern, Handler handler) {
        get_routes[pattern] = handler;
        return *this;
    }

    Server& Post(const std::string& pattern, Handler handler) {
        post_routes[pattern] = handler;
        return *this;
    }

    bool listen(const std::string& host, int port) {
        server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_socket == INVALID_SOCKET_VAL) {
            std::cerr << "Failed to create socket\n";
            return false;
        }

        int opt = 1;
#ifdef _WIN32
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

        sockaddr_in server_addr;
        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(static_cast<u_short>(port));

        if (host == "0.0.0.0" || host.empty()) {
            server_addr.sin_addr.s_addr = INADDR_ANY;
        } else {
            server_addr.sin_addr.s_addr = inet_addr(host.c_str());
        }

        if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR_VAL) {
            std::cerr << "Failed to bind to " << host << ":" << port << "\n";
            CLOSE_SOCKET(server_socket);
            server_socket = INVALID_SOCKET_VAL;
            return false;
        }

        if (::listen(server_socket, 10) == SOCKET_ERROR_VAL) {
            std::cerr << "Failed to listen on socket\n";
            CLOSE_SOCKET(server_socket);
            server_socket = INVALID_SOCKET_VAL;
            return false;
        }

        is_running = true;

        while (is_running) {
            sockaddr_in client_addr;
            int client_len = sizeof(client_addr);
            socket_t client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

            if (client_socket == INVALID_SOCKET_VAL) {
                if (!is_running) break;
                continue;
            }

            handle_client(client_socket);
        }

        return true;
    }

    void stop() {
        is_running = false;
        if (server_socket != INVALID_SOCKET_VAL) {
            CLOSE_SOCKET(server_socket);
            server_socket = INVALID_SOCKET_VAL;
        }
    }
};

} // namespace httplib

#endif // HTTPLIB_H