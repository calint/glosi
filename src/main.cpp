// reviewed: 2023-12-22
// reviewed: 2024-01-04
// reviewed: 2024-01-16
// reviewed: 2024-07-15

#include "application/application.hpp"
#include "engine/net_server.hpp"

auto main(int argc, char const* argv[]) -> int {

    if (argc > 1 && *argv[1] == 's') {
        // instance is server
        glos::net_server.init();
        glos::net_server.run();
        // note: 'run' does not return
    }

    // instance is client
    application_print_hello();

    if (argc > 1 and *argv[1] == 'c') {
        // multiplayer client, enable 'net'
        glos::net.enabled = true;
        if (argc > 2) {
            // server ip
            glos::net.host = argv[2];
        }
    }

    glos::engine.init();
    glos::engine.run();
    glos::engine.free();
}
