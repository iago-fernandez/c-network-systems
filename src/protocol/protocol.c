/**
 * @file protocol.c
 * @brief Implementation of binary protocol serialization and deserialization.
 */
#include "protocol/protocol.h"
#include <arpa/inet.h>
#include <string.h>

void serialize_header(const PacketHeader* header, uint8_t* buffer) {
    uint16_t net_version = htons(header->version);
    uint16_t net_type = htons(header->type);
    uint32_t net_seq = htonl(header->sequence_number);
    uint32_t net_length = htonl(header->payload_length);

    // Copy fields sequentially to avoid padding issues (12 bytes total)
    memcpy(buffer, &net_version, sizeof(uint16_t));      // Offset 0
    memcpy(buffer + 2, &net_type, sizeof(uint16_t));     // Offset 2
    memcpy(buffer + 4, &net_seq, sizeof(uint32_t));      // Offset 4
    memcpy(buffer + 8, &net_length, sizeof(uint32_t));   // Offset 8
}

void deserialize_header(const uint8_t* buffer, PacketHeader* header) {
    uint16_t net_version;
    uint16_t net_type;
    uint32_t net_seq;
    uint32_t net_length;

    memcpy(&net_version, buffer, sizeof(uint16_t));
    memcpy(&net_type, buffer + 2, sizeof(uint16_t));
    memcpy(&net_seq, buffer + 4, sizeof(uint32_t));
    memcpy(&net_length, buffer + 8, sizeof(uint32_t));

    header->version = ntohs(net_version);
    header->type = ntohs(net_type);
    header->sequence_number = ntohl(net_seq);
    header->payload_length = ntohl(net_length);
}