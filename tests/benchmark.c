/**
 * @file benchmark.c
 * @brief Concurrent load testing tool to measure server throughput and temporal efficiency.
 */
#include "protocol/protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define CMD_ECHO 0x02
#define THREAD_COUNT 10
#define REQUESTS_PER_THREAD 10000

typedef struct {
    int port;
    int thread_id;
    uint32_t success_count;
} BenchmarkConfig;

static double get_time_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static void* benchmark_worker(void* arg) {
    BenchmarkConfig* config = (BenchmarkConfig*)arg;
    int sock_fd;
    struct sockaddr_in server_addr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        pthread_exit(NULL);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config->port);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock_fd);
        pthread_exit(NULL);
    }

    const char* message_data = "BENCHMARK_PAYLOAD";
    uint32_t data_len = (uint32_t)strlen(message_data);

    PacketHeader req_header;
    memset(&req_header, 0, sizeof(PacketHeader));
    req_header.type = CMD_ECHO;
    req_header.payload_length = data_len;

    uint8_t header_buf[sizeof(PacketHeader)];
    uint8_t resp_header_buf[sizeof(PacketHeader)];
    char resp_payload[64];

    for (int i = 0; i < REQUESTS_PER_THREAD; i++) {
        req_header.sequence_number = i;
        serialize_header(&req_header, header_buf);

        if (send(sock_fd, header_buf, sizeof(header_buf), 0) <= 0) break;
        if (send(sock_fd, message_data, data_len, 0) <= 0) break;

        if (recv(sock_fd, resp_header_buf, sizeof(resp_header_buf), 0) <= 0) break;

        PacketHeader resp_header;
        deserialize_header(resp_header_buf, &resp_header);

        if (recv(sock_fd, resp_payload, resp_header.payload_length, 0) <= 0) break;

        config->success_count++;
    }

    close(sock_fd);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    printf("[BENCHMARK] Starting load test on %s:%d\n", SERVER_IP, port);
    printf("[BENCHMARK] Threads: %d, Requests per thread: %d\n", THREAD_COUNT, REQUESTS_PER_THREAD);

    pthread_t threads[THREAD_COUNT];
    BenchmarkConfig configs[THREAD_COUNT];

    double start_time = get_time_seconds();

    for (int i = 0; i < THREAD_COUNT; i++) {
        configs[i].port = port;
        configs[i].thread_id = i;
        configs[i].success_count = 0;
        pthread_create(&threads[i], NULL, benchmark_worker, &configs[i]);
    }

    uint32_t total_success = 0;
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
        total_success += configs[i].success_count;
    }

    double end_time = get_time_seconds();
    double elapsed = end_time - start_time;
    double rps = total_success / elapsed;

    printf("[BENCHMARK] Completed in %.4f seconds.\n", elapsed);
    printf("[BENCHMARK] Total successful requests: %u\n", total_success);
    printf("[BENCHMARK] Throughput: %.2f requests/second\n", rps);

    return 0;
}