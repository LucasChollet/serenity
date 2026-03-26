/*
 * Copyright (c) 2026, Lucas Chollet <lucas.chollet@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/String.h>
#include <LibCore/ArgsParser.h>
#include <LibCore/EventLoop.h>
#include <LibCore/Process.h>
#include <LibCore/System.h>
#include <LibCore/TCPServer.h>
#include <LibMain/Main.h>
#include <SSHServer/ServerConfiguration.h>
#include <SSHServer/TCPClient.h>

static constexpr auto DEFAULT_LISTEN_ADDRESS = "0.0.0.0"sv;
static constexpr auto DEFAULT_PORT = 22;

static ErrorOr<void> accept_connection(Core::TCPServer& server, int redirect_stderr, bool unsafe_stub_private_key)
{
    auto client_socket = TRY(server.accept());
    TRY(client_socket->set_close_on_exec(false));

    Vector<ByteString> arguments = { "--socket-fd", ByteString::number(client_socket->fd()) };
    if (unsafe_stub_private_key)
        arguments.append("--unsafe-stub-private-key");

    TRY(Core::Process::spawn(Core::ProcessSpawnOptions {
        .name = "SSHServer - Connection"sv,
        // FIXME: Find a reliable way to find this executable.
        .executable = "./SSHConnection"sv,
        .search_for_executable_in_path = true,
        .arguments = arguments,
        .file_actions = {
            Core::FileAction::CloseFile { .fd = STDERR_FILENO },
            Core::FileAction::DuplicateFile { .old_fd = redirect_stderr, .new_fd = STDERR_FILENO },
        } }));

    return {};
}

ErrorOr<int> serenity_main(Main::Arguments args)
{
    TRY(Core::System::pledge("stdio accept inet unix rpath proc exec"));
    TRY(Core::System::unveil("/bin/SSHConnection", "rx"));
    TRY(Core::System::unveil(nullptr, nullptr));

    Optional<u32> port {};
    bool unsafe_stub_private_key { false };

    Core::ArgsParser parser;
    parser.add_option(port, "Port to listen on", "port", 'p', "port");
    parser.add_option(unsafe_stub_private_key, "Stub the server's private key - UNSAFE", "unsafe-stub-private-key");

    parser.parse(args);

    if (port.has_value() && *port != static_cast<u16>(*port))
        return Error::from_string_literal("Invalid port number");

    Core::EventLoop loop;

    auto tcp_server = TRY(Core::TCPServer::try_create());

    auto redirect_stderr = dup(STDERR_FILENO);

    tcp_server->on_ready_to_accept = [&] {
        auto maybe_error = accept_connection(*tcp_server, redirect_stderr, unsafe_stub_private_key);
        if (maybe_error.is_error())
            warnln("Can't accept connection: {}", maybe_error.error());
    };

    TRY(tcp_server->listen(*IPv4Address::from_string(DEFAULT_LISTEN_ADDRESS), port.value_or(DEFAULT_PORT)));

    outln("Listening on {}:{}", tcp_server->local_address().value(), tcp_server->local_port());

    TRY(Core::System::pledge("stdio accept rpath proc exec"));
    return loop.exec();
}
