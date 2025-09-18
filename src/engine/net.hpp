#pragma once
// reviewed: 2023-12-23
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-07-08

#include "../application/configuration.hpp"
#include "exception.hpp"
#include "metrics.hpp"
#include <SDL2/SDL_timer.h>
#include <arpa/inet.h>
#include <array>
#include <cstdint>
#include <cstring>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

namespace glos {

// client state
class net_state final {
  public:
    uint64_t keys = 0;
    float look_angle_y = 0;
    float look_angle_x = 0;
};

// initial packet sent by server
class net_init_packet final {
  public:
    uint32_t player_ix = 0;
    uint64_t ms = 0;
};

class net final {
  public:
    bool enabled = false;
    net_state next_state{};
    std::array<net_state, net_players + 1> states{};
    // note: +1 because index 0 is used for server message to clients

    uint32_t player_ix = 1;
    float dt = 0;
    uint64_t ms = 0;
    char const* host = "127.0.0.1";
    uint16_t port = 8085;

    inline auto init() -> void {
        if (net_players < 1) {
            throw exception{"configuration 'net_players' must be at least 1"};
        }

        if (!enabled) {
            return;
        }

        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1) {
            throw exception{strerror(errno)};
        }

        int flag = 1;
        if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag))) {
            throw exception{"cannot set TCP_NODELAY"};
        }

        struct sockaddr_in server{};
        server.sin_addr.s_addr = inet_addr(host);
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        printf("[ net ] connecting to '%s' on port %u\n", host, port);
        if (connect(fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
            throw exception{strerror(errno)};
        }

        printf("[ net ] connected. waiting for go ahead\n");
        net_init_packet nip{};
        ssize_t const n = recv(fd, &nip, sizeof(nip), 0);
        if (n == -1) {
            throw exception{strerror(errno)};
        }
        if (n == 0) {
            throw exception{"server disconnected"};
        }
        if (size_t(n) != sizeof(nip)) {
            throw exception{"incomplete receive"};
        }

        // get server assigned player index and time in milliseconds
        player_ix = nip.player_ix;
        ms = nip.ms;

        printf("[ net ] playing player %u\n", player_ix);
        printf("[ net ] server time: %lu ms\n\n", ms);
    }

    inline auto free() -> void {
        if (!enabled) {
            return;
        }

        if (fd) {
            close(fd);
            fd = 0;
        }
    }

    inline auto begin() -> void { send_state(); }

    inline auto receive_and_send() -> void {
        // calculate net lag
        uint64_t const t0 = SDL_GetPerformanceCounter();

        // receive signals from previous frame
        ssize_t const n = recv(fd, states.data(), sizeof(states), 0);
        if (n == -1) {
            throw exception{strerror(errno)};
        }
        if (n == 0) {
            throw exception{"server disconnected"};
        }
        if (size_t(n) != sizeof(states)) {
            throw exception{"incomplete read of states"};
        }

        // send current frame signals
        send_state();

        // get server 'dt' and time 'ms' from server state
        dt = states[0].look_angle_x;
        ms = states[0].keys;

        // calculate network lag
        uint64_t const t1 = SDL_GetPerformanceCounter();
        metrics.net_ms =
            1000.0f * float(t1 - t0) / float(SDL_GetPerformanceFrequency());
    }

  private:
    int fd = 0;

    inline auto send_state() const -> void {
        ssize_t const n = send(fd, &next_state, sizeof(next_state), 0);
        if (n == -1) {
            throw exception{strerror(errno)};
        }
        if (size_t(n) != sizeof(next_state)) {
            throw exception{"incomplete send"};
        }
    }
} static net{};

} // namespace glos
