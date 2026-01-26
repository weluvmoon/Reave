#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <map>
#include <unistd.h>
#include <vector>

#define SERVER_PORT 12345
#define MAX_CLIENTS 8

// Types of packets
enum PacketType { PKT_UPDATE, PKT_JOIN, PKT_QUIT };

struct NetworkPacket {
    PacketType type;
    uint32_t networkID; // Unique ID for each player
    float x, y;         // Position
};

// Helper to set non-blocking mode on Linux
void SetNonBlocking(int fd);

