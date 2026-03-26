/*
 * Copyright (c) 2026, Lucas Chollet <lucas.chollet@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibCore/ArgsParser.h>
#include <LibCore/EventLoop.h>
#include <LibCore/Socket.h>
#include <LibMain/Main.h>
#include <SSHServer/ServerConfiguration.h>
#include <SSHServer/TCPClient.h>

ErrorOr<int> serenity_main(Main::Arguments args)
{
    int socket_fd {};
    bool unsafe_stub_private_key { false };

    Core::ArgsParser parser;
    parser.add_option(socket_fd, "Socket to read from", "socket-fd", 0, "fd");
    parser.add_option(unsafe_stub_private_key, "Stub the server's private key - UNSAFE", "unsafe-stub-private-key");
    parser.parse(args);

    if (unsafe_stub_private_key)
        SSH::Server::ServerConfiguration::the().use_unsafe_stubbed_private_key();

    OwnPtr<Core::TCPSocket> socket = TRY(Core::TCPSocket::adopt_fd(socket_fd));

    Core::EventLoop event_loop;
    auto on_quit = [&]() {
        socket = nullptr;
        Core::EventLoop::current().quit(0);
    };

    auto client = SSH::Server::TCPClient::create(socket.release_nonnull(), move(on_quit));

    return event_loop.exec();
}
