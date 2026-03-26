serenity_component(
    SSHServer
    TARGETS SSHServer SSHConnection
)

set(SOURCES
    SSHConnection.cpp
    ServerConfiguration.cpp
    SSHClient.cpp
    TCPClient.cpp
)

serenity_bin(SSHConnection)
target_link_libraries(SSHConnection PRIVATE LibCore LibCrypto LibMain LibSSH)

set(SOURCES
        SSHServer.cpp
)

serenity_bin(SSHServer)
target_link_libraries(SSHServer PRIVATE LibCore LibMain)
add_dependencies(SSHServer SSHConnection)
