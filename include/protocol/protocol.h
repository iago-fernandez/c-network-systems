/**
 * @file protocol.h
 * @brief Defines the packed structures and constants for the custom binary network protocol.
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define PROTOCOL_VERSION_1 0x0001
#define MAX_PAYLOAD_SIZE 1024

 /**
  * @enum PacketType
  * @brief Enumeration of available packet types for the network protocol.
  */
typedef enum {
    PACKET_TYPE_HEARTBEAT = 0x01,
    PACKET_TYPE_DATA = 0x02,
    PACKET_TYPE_ACK = 0x03,
    PACKET_TYPE_ERROR = 0xFF
} PacketType;

/**
 * @struct PacketHeader
 * @brief Header for all protocol packets. Packed to prevent compiler padding.
 */
typedef struct __attribute__((packed)) {
    uint16_t version;
    uint16_t type;
    uint32_t sequence_number;
    uint32_t payload_length;
} PacketHeader;

/**
 * @struct DataPacket
 * @brief Standard data packet containing a header and a fixed-size payload.
 */
typedef struct __attribute__((packed)) {
    PacketHeader header;
    uint8_t payload[MAX_PAYLOAD_SIZE];
} DataPacket;

#endif