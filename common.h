#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define MAX_FILENAME 256

// Commands
#define CMD_SEND_FILE 1
#define CMD_LIST_FILES 2
#define CMD_DOWNLOAD 3
#define CMD_EXIT 4

// Protocol header for file transfer
typedef struct {
  uint32_t command;
  uint32_t file_size;
  uint32_t filename_len;
  char filename[MAX_FILENAME];
} file_header_t;

#endif // !COMMON_H
