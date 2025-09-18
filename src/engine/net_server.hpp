#pragma once
// reviewed: 2023-12-23
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-07-08

#include "net.hpp"
#include <SDL2/SDL.h>

namespace glos {

class net_server final {
  public:
    uint16_t port = 8085;

    inline auto init() -> void {
        if (SDL_Init(SDL_INIT_TIMER)) {
            throw exception{
                std::format("cannot initiate sdl timer: {}", SDL_GetError())};
        }

        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            throw exception{"cannot create socket"};
        }

        {
            int flag = 1;
            if (setsockopt(server_fd, IPPROTO_TCP, TCP_NODELAY, &flag,
                           sizeof(flag))) {
                throw exception{"cannot set TCP_NODELAY"};
            }
        }

        struct sockaddr_in server{};
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr*)&server, sizeof(server))) {
            throw exception{"cannot bind socket"};
        }

        if (listen(server_fd, net_players)) {
            throw exception{strerror(errno)};
        }

        printf(" * waiting for %u players to connect on port %u\n", net_players,
               port);

        for (uint32_t i = 1; i < net_players + 1; ++i) {
            clients_fd[i] = accept(server_fd, nullptr, nullptr);
            if (clients_fd[i] == -1) {
                throw exception{strerror(errno)};
            }

            int flag = 1;
            if (setsockopt(clients_fd[i], IPPROTO_TCP, TCP_NODELAY, &flag,
                           sizeof(flag))) {
                throw exception{strerror(errno)};
            }

            printf(" * player %u of %u connected\n", i, net_players);
        }

        printf(" * sending start\n");

        net_init_packet nip{};
        nip.ms = SDL_GetTicks64();
        // send the assigned player index and time to clients
        for (uint32_t i = 1; i < net_players + 1; ++i) {
            // send initial packet to clients
            nip.player_ix = i;
            ssize_t const n = send(clients_fd[i], &nip, sizeof(nip), 0);
            if (n == -1) {
                throw exception{std::format(
                    "could not send initial packet to player {}: {}", i,
                    strerror(errno))};
            }
            if (size_t(n) != sizeof(nip)) {
                throw exception{std::format("incomplete send to player {}: {}",
                                            i, strerror(errno))};
            }
        }
    }

    [[noreturn]] inline auto run() -> void {
        printf(" * entering loop\n");
        uint64_t t0 = SDL_GetPerformanceCounter();
        while (true) {
            // note: state[0] contains server info sent to all clients
            //       state[1] etc are the client states
            for (uint32_t i = 1; i < net_players + 1; ++i) {
                ssize_t const n =
                    recv(clients_fd[i], &state[i], sizeof(state[i]), 0);
                if (n == -1) {
                    throw exception{
                        std::format("player {}: {}", i, strerror(errno))};
                }
                if (n == 0) {
                    throw exception{std::format("player {}: disconnected", i)};
                }
                if (size_t(n) != sizeof(state[i])) {
                    throw exception{
                        std::format("player {}: read was incomplete", i)};
                }
            }

            // calculate the delta time for this frame
            uint64_t const t1 = SDL_GetPerformanceCounter();
            float const dt =
                float(t1 - t0) / float(SDL_GetPerformanceFrequency());
            t0 = t1;

            // using state[0] to broadcast data from server to all players
            state[0].look_angle_x = dt;
            state[0].keys = SDL_GetTicks64();

            for (uint32_t i = 1; i < net_players + 1; ++i) {
                ssize_t const n =
                    send(clients_fd[i], state.data(), sizeof(state), 0);
                if (n == -1) {
                    throw exception{
                        std::format("player {}: {}", i, strerror(errno))};
                }
                if (size_t(n) != sizeof(state)) {
                    throw exception{
                        std::format("player {}: send was incomplete", i)};
                }
            }
        }
    }

    inline auto free() -> void {
        size_t const n = clients_fd.size();
        for (uint32_t i = 1; i < n; i++) {
            close(clients_fd[i]);
            clients_fd[i] = 0;
        }
        close(server_fd);
        server_fd = 0;
        SDL_Quit();
    }

  private:
    int server_fd = 0;
    std::array<int, net_players + 1> clients_fd{};
    std::array<net_state, net_players + 1> state{};
    // note: state[0] is used by server to broadcast to all clients
    //       delta time for frame (dt) and current server time in ms
} static net_server{};

} // namespace glos
