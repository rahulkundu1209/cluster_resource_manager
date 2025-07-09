#include "LoadBalancer.h"
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>

LoadBalancer::LoadBalancer(int listen_port, const std::vector<int>& ports)
    : listen_port(listen_port), backend_ports(ports), active_connections(ports.size(), 0) {}

int LoadBalancer::find_least_connection_index() {
    std::lock_guard<std::mutex> lock(connection_mutex);
    return std::min_element(active_connections.begin(), active_connections.end()) - active_connections.begin();
}

void LoadBalancer::handle_client(int client_socket) {
    int index = find_least_connection_index();
    int backend_port = backend_ports[index];

    {
        std::lock_guard<std::mutex> lock(connection_mutex);
        active_connections[index]++;
    }

    int backend_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (backend_socket < 0) {
        std::cerr << "Failed to create backend socket.\n";
        close(client_socket);
        return;
    }

    sockaddr_in backend_addr;
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(backend_port);
    inet_pton(AF_INET, "127.0.0.1", &backend_addr.sin_addr);

    if (connect(backend_socket, (struct sockaddr*)&backend_addr, sizeof(backend_addr)) < 0) {
        std::cerr << "Failed to connect to backend on port " << backend_port << ".\n";
        close(client_socket);
        close(backend_socket);
        return;
    }

    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(client_socket, buffer, sizeof(buffer))) > 0) {
        send(backend_socket, buffer, bytes_read, 0);
        bytes_read = read(backend_socket, buffer, sizeof(buffer));
        send(client_socket, buffer, bytes_read, 0);
    }

    close(client_socket);
    close(backend_socket);

    {
        std::lock_guard<std::mutex> lock(connection_mutex);
        active_connections[index]--;
    }

    std::cout << "Connection to backend port " << backend_port << " closed. Active connections: " << active_connections[index] << "\n";
}

void LoadBalancer::start() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Failed to create server socket.\n";
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(listen_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to bind server socket.\n";
        close(server_socket);
        return;
    }

    if (listen(server_socket, 10) < 0) {
        std::cerr << "Failed to listen on server socket.\n";
        close(server_socket);
        return;
    }

    std::cout << "Load balancer started. Listening on port " << listen_port << "...\n";

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            std::cerr << "Failed to accept client connection.\n";
            continue;
        }

        std::thread(&LoadBalancer::handle_client, this, client_socket).detach();
    }

    close(server_socket);
}