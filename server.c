#include "common.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

void handle_error(const char *msg);
void handle_client(int client_sock);
void send_file_list(int client_sock);
void receive_file(int client_sock, file_header_t *header);
void send_file(int client_sock, const char *filename);

int main() {
  int server_fd, client_sock;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_len = sizeof(client_addr);
  pid_t pid;

  // Create socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    handle_error("Socket creation failed");
  }

  // Set socket options to reuse address
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    handle_error("Setsockopt failed");
  }

  // Configure server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  // Bind socket
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    handle_error("Bind failed");
  }

  // Listen for connections
  if (listen(server_fd, MAX_CLIENTS) < 0) {
    handle_error("Listen failed");
  }

  printf("Server listening on port %d\n", PORT);

  // Main server loop
  while (1) {
    // Accept client connection
    client_sock =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_sock < 0) {
      perror("Accept failed");
      continue;
    }

    printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    // Fork to handle client
    pid = fork();
    if (pid < 0) {
      perror("Fork failed");
      close(client_sock);
    } else if (pid == 0) {
      // Child process
      close(server_fd); // Child doesn't need listener
      handle_client(client_sock);
      close(client_sock);
      exit(0);
    } else {
      // Parent process
      close(client_sock); // Parent doesn't need client socket
    }
  }

  return 0;
}

void handle_error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

void handle_client(int client_sock) {
  file_header_t header;
  ssize_t bytes_received;

  while (1) {
    // Receive command header
    bytes_received = recv(client_sock, &header, sizeof(header), 0);
    if (bytes_received <= 0) {
      if (bytes_received == 0) {
        printf("Client disconnected\n");
      } else {
        perror("Receive failed");
      }
      break;
    }

    switch (header.command) {
    case CMD_SEND_FILE:
      printf("Receiving file: %s\n", header.filename);
      receive_file(client_sock, &header);
      break;

    case CMD_LIST_FILES:
      printf("Sending file list\n");
      send_file_list(client_sock);
      break;

    case CMD_DOWNLOAD:
      printf("Sending file: %s\n", header.filename);
      send_file(client_sock, header.filename);
      break;

    case CMD_EXIT:
      printf("Client requested exit\n");
      return;

    default:
      printf("Unknown command: %d\n", header.command);
    }
  }
}

void receive_file(int client_sock, file_header_t *header) {
  char buffer[BUFFER_SIZE];
  FILE *file;
  ssize_t bytes_received;
  size_t total_received = 0;

  // Open file for writing
  file = fopen(header->filename, "wb");
  if (!file) {
    perror("Failed to open file");
    return;
  }

  // Receive file data
  while (total_received < header->file_size) {
    bytes_received = recv(client_sock, buffer,
                          (header->file_size - total_received < BUFFER_SIZE)
                              ? header->file_size - total_received
                              : BUFFER_SIZE,
                          0);

    if (bytes_received <= 0) {
      perror("Receive error");
      break;
    }

    fwrite(buffer, 1, bytes_received, file);
    total_received += bytes_received;

    // Progress indicator
    printf("\rReceiving: %.2f%%",
           (float)total_received / header->file_size * 100);
    fflush(stdout);
  }

  printf("\nFile received successfully: %s (%zu bytes)\n", header->filename,
         total_received);

  fclose(file);
}

void send_file_list(int client_sock) {
  FILE *fp;
  char buffer[BUFFER_SIZE];

  // Execute ls command and send output
  fp = popen("ls -la", "r");
  if (!fp) {
    perror("Failed to list files");
    return;
  }

  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    send(client_sock, buffer, strlen(buffer), 0);
  }

  // Send end marker
  send(client_sock, "END_OF_LIST\n", 12, 0);

  pclose(fp);
}

void send_file(int client_sock, const char *filename) {
  FILE *file;
  char buffer[BUFFER_SIZE];
  size_t bytes_read;
  struct stat file_stat;
  file_header_t header;

  // Check if file exists
  if (stat(filename, &file_stat) < 0) {
    perror("File not found");
    // Send error indication (0 file size)
    header.file_size = 0;
    send(client_sock, &header, sizeof(header), 0);
    return;
  }

  // Prepare header
  memset(&header, 0, sizeof(header));
  header.command = CMD_SEND_FILE;
  header.file_size = file_stat.st_size;
  strncpy(header.filename, filename, MAX_FILENAME - 1);

  // Send header
  send(client_sock, &header, sizeof(header), 0);

  // Open and send file
  file = fopen(filename, "rb");
  if (!file) {
    perror("Failed to open file");
    return;
  }

  while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
    send(client_sock, buffer, bytes_read, 0);
  }

  fclose(file);
}
