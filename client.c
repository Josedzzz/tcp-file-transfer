#include "common.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

void handle_error(const char *msg);
void send_file_to_server(int sock, const char *filename);
void receive_file_from_server(int sock, const char *filename);
void list_server_files(int sock);
void show_menu();

int main(int argc, char *argv[]) {
  int sock;
  struct sockaddr_in server_addr;
  int choice;
  char filename[MAX_FILENAME];
  file_header_t header;

  if (argc != 2) {
    printf("Usage: %s <server_ip>\n", argv[0]);
    exit(1);
  }

  // Create socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    handle_error("Socket creation failed");
  }

  // Configure server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
    handle_error("Invalid address");
  }

  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    handle_error("Connection failed");
  }

  printf("Connected to server %s:%d\n", argv[1], PORT);

  // Client loop
  while (1) {
    show_menu();
    printf("Enter choice: ");
    scanf("%d", &choice);
    getchar(); // Clear new line

    memset(&header, 0, sizeof(header));
    header.command = choice;

    switch (choice) {
    case CMD_SEND_FILE:
      printf("Enter filename to send: ");
      fgets(filename, MAX_FILENAME, stdin);
      filename[strcspn(filename, "\n")] = 0; // Remove newline
      send_file_to_server(sock, filename);
      break;

    case CMD_LIST_FILES:
      header.command = CMD_LIST_FILES;
      send(sock, &header, sizeof(header), 0);
      list_server_files(sock);
      break;

    case CMD_DOWNLOAD:
      printf("Enter filename to download: ");
      fgets(filename, MAX_FILENAME, stdin);
      filename[strcspn(filename, "\n")] = 0;
      strncpy(header.filename, filename, MAX_FILENAME - 1);
      send(sock, &header, sizeof(header), 0);
      receive_file_from_server(sock, filename);
      break;

    case CMD_EXIT:
      send(sock, &header, sizeof(header), 0);
      printf("Exiting...\n");
      close(sock);
      return 0;

    default:
      printf("Invalid choice\n");
    }
  }

  close(sock);
  return 0;
}

void handle_error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

void send_file_to_server(int sock, const char *filename) {
  FILE *file;
  char buffer[BUFFER_SIZE];
  size_t bytes_read;
  struct stat file_stat;
  file_header_t header;

  // Check if files exists
  if (stat(filename, &file_stat) < 0) {
    perror("File not found");
    return;
  }

  // Prepare header
  memset(&header, 0, sizeof(header));
  header.command = CMD_SEND_FILE;
  header.file_size = file_stat.st_size;
  strncpy(header.filename, filename, MAX_FILENAME - 1);

  // Send header
  send(sock, &header, sizeof(header), 0);

  // Open and send file
  file = fopen(filename, "rb");
  if (!file) {
    perror("Failed to open file");
    return;
  }

  printf("Sending file: %s\n", filename);

  while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
    send(sock, buffer, bytes_read, 0);
    printf(".");
    fflush(stdout);
  }

  printf("\nFile sent successfully\n");
  fclose(file);
}

void receive_file_from_server(int sock, const char *filename) {
  char buffer[BUFFER_SIZE];
  FILE *file;
  ssize_t bytes_received;
  file_header_t header;
  size_t total_received = 0;

  // Receive header first
  bytes_received = recv(sock, &header, sizeof(header), 0);
  if (bytes_received <= 0) {
    perror("Failed to receive header");
    return;
  }

  if (header.file_size == 0) {
    printf("File not found on server\n");
    return;
  }

  // Open file for writing
  file = fopen(filename, "wb");
  if (!file) {
    perror("Failed to create file");
    return;
  }

  printf("Receiving file: %s (%u bytes)\n", filename, header.file_size);

  // Receive file data
  while (total_received < header.file_size) {
    bytes_received = recv(sock, buffer,
                          (header.file_size - total_received < BUFFER_SIZE)
                              ? header.file_size - total_received
                              : BUFFER_SIZE,
                          0);

    if (bytes_received <= 0) {
      perror("Receive error");
      break;
    }

    fwrite(buffer, 1, bytes_received, file);
    total_received += bytes_received;

    // Progress indicator
    printf("\rProgress: %.2f%%",
           (float)total_received / header.file_size * 100);
    fflush(stdout);
  }

  printf("\nFile received successfully\n");
  fclose(file);
}

void list_server_files(int sock) {
  char buffer[BUFFER_SIZE];
  ssize_t bytes_received;

  printf("\n=== Server Files ===\n");

  while (1) {
    bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0)
      break;

    buffer[bytes_received] = '\0';

    // Check for end marker
    if (strstr(buffer, "END_OF_LIST") != NULL)
      break;

    printf("%s", buffer);
  }
  printf("===================\n");
}

void show_menu() {
  printf("\n=== TCP File Transfer Client ===\n");
  printf("1. Send file to server\n");
  printf("2. List files on server\n");
  printf("3. Download file from server\n");
  printf("4. Exit\n");
}
