/**
 * @file protocol.c
 * @brief Implementation of binary protocol serialization and deserialization.
 */
#include "protocol/protocol.h"
#include <arpa/inet.h>
#include <string.h>

void serialize_header(const ProtocolHeader* header, uint8_t* buffer) {
    uint32_t net_length = htonl(header->payload_length);
    uint16_t net_type = htons(header->message_type);
    uint16_t net_version = htons(header->version);

    memcpy(buffer, &net_length, sizeof(uint32_t));
    memcpy(buffer + 4, &net_type, sizeof(uint16_t));
    memcpy(buffer + 6, &net_version, sizeof(uint16_t));
}

void deserialize_header(const uint8_t* buffer, ProtocolHeader* header) {
    uint32_t net_length;
    uint16_t net_type;
    uint16_t net_version;

    memcpy(&net_length, buffer, sizeof(uint32_t));
    memcpy(&net_type, buffer + 4, sizeof(uint16_t));
    memcpy(&net_version, buffer + 6, sizeof(uint16_t));

    header->payload_length = ntohl(net_length);
    header->message_type = ntohs(net_type);
    header->version = ntohs(net_version);
}