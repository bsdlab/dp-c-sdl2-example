// The TCP server components to hook the paradigm up with DAREPLANE
//
#include "server.h"
#include "paradigm.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <string.h> // for memset
#include <unistd.h> // for close
#define PORT "8081" // have this defined dynamically from args later
#define HOST "127.0.0.1"
#define BACKLOG 5 // how many pending connections queue will hold

const int MAX_BUF_SIZE = 2048; // For the simple exampel this buffer is enough,
                               // as we will not process complex json payloads

// The primary commands w
const char *const PCOMMS[] = {
    "START", // Starting the paradigm
    "STOP",  // Stopping the running paradigm
    "CLOSE"  // Closing the server
};

// Remove newline and carriage return from the message
void clean_msg(char *msg) {
  for (int i = 0; i < strlen(msg); i++) {
    if (msg[i] == '\n' || msg[i] == '\r') {
      msg[i] = '\0';
    }
  }
}

// run the server as subthread, interaction with main thread is done via global
// vars
int run_server(void *unused) {

  int sockfd = 0;
  int newfd = 0;
  char buffer[MAX_BUF_SIZE];

  int status;

  socklen_t addr_size;
  struct addrinfo hints, *servinfo;
  struct sockaddr_storage client_addr;

  memset(&hints, 0, sizeof hints); // make sure the struct is empty
  hints.ai_family = AF_INET;       // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(HOST, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  // Print information gathered into servinfo
  struct sockaddr_in *saddr = (struct sockaddr_in *)servinfo->ai_addr;
  printf("Server information: %s\n", inet_ntoa(saddr->sin_addr));
  printf("Server ai_socktype: %d\n", servinfo->ai_socktype);
  printf("Server ai_protocol: %d\n", servinfo->ai_protocol);
  printf("Server ai_family: %d\n", servinfo->ai_family);

  // Socket file descriptor
  sockfd =
      socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

  // Ensure we can reuse the port / connection
  int reuse = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse,
                 sizeof(reuse)) < 0)
    perror("setsockopt(SO_REUSEADDR) failed");

#ifdef SO_REUSEPORT
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char *)&reuse,
                 sizeof(reuse)) < 0)
    perror("setsockopt(SO_REUSEPORT) failed");
#endif

  // bind to port
  if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) != 0) {
    perror("Error binding to port\n");
    return -1;
  } else {
    printf("Bound to:  %s\n", servinfo->ai_addr->sa_data);
  }

  if (listen(sockfd, BACKLOG) != 0) {
    perror("Listening failed...\n");
    return -1;
  } else {
    printf("Listening on port %s\n", PORT);
  }

  // Accept a connection
  if ((newfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size)) <
      0) {
    perror("Accept failed ...");
    return -1;
  }

  char *welcome_msg = "Welcome to the DAREPLANE <MYPARADIGM> server\n";
  send(newfd, welcome_msg, strlen(welcome_msg), 0);

  // Server main loop
  int run_server = 1;
  while (run_server) {
    bzero(buffer, MAX_BUF_SIZE);

    ssize_t bytes_received = read(
        newfd, buffer, MAX_BUF_SIZE - 1); // leave space for null terminator
    printf("Bytes received: %ld\n", bytes_received);
    if (bytes_received >= 0) {
      clean_msg(buffer);
      printf("Received: %s\n", buffer);

      if (strcmp(buffer, "CLOSE") == 0) {
        run_server = 0;
        paradigm_stop_event = -1;
      } else if (strcmp(buffer, "START") == 0) {
        printf("Starting paradigm\n");
        paradigm_stop_event = 0;

      } else if (strcmp(buffer, "STOP") == 0) {
        printf("Stopping paradigm\n");
        paradigm_stop_event = 1;
      } else {
        printf("Received: %s\n", buffer);
        send(newfd, buffer, strlen(buffer), 0);
      }
    }
  }
  close(newfd);
  close(sockfd);

  return 0;
}
