#include "csapp.h"

int open_clientfd(char *hostname, char *port) {
  int clientfd;
  struct addrinfo hints, *listp, *p;

  // Get a list of potential server addresses
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICSERV;
  hints.ai_flags |= AI_ADDRCONFIG;
  Getaddrinfo(hostname, port, &hints, &listp);

  // Walk the list for one that we can successfully connect to
  for (p = listp; p; p = p->ai_next) {

    // Create a socket descriptor
    // Case 1. Socket failed, try the next
    if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) { continue; }

    // Case 2. Connect to the server (Success)
    if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) { break; }

    // Case 3. Connect failed, try another
    Close(clientfd);
  }

  // Clean up
  Freeaddrinfo(listp);
  
  // All connects failed
  if (!p) { return -1; }
  // The last connect succeeded
  else { return clientfd; }
}

int open_listenfd(char *port) {
  struct addrinfo hints, *listp, *p;
  int listenfd, optval = 1;

  // Get a list of potential server address
  memset(&hints, 0, sizeof(struct addrinfo));

  // Accept Connections
  hints.ai_socktype = SOCK_STREAM;
  // ... on any IP Address
  hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
  // ... using port number
  hints.ai_flags = AI_NUMERICSERV;
  
  Getaddrinfo(NULL, port, &hints, &listp);

  // Walk the list for one that we can bind to
  for (p = listp; p; p = p->ai_next) {
    // Create a socket descriptor
    // Case 1. Socket failed, try the next
    if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) { continue; }
    // Case 2. Eliminates "Address already in use" error from bind
    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    // Case 3. Bind the descriptor to the address (Success)
    if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) { break; }

    // Bind failed, try the next
    Close(listenfd);
  }

  // Clean up
  Freeaddrinfo(listp);
  // No address worked
  if (!p) { return -1; }

  // Make it a listening socket ready to accept connection requests
  if (listen(listenfd, LISTENQ) < 0) {
    Close(listenfd);
    return -1;
  }
  
  return listenfd;
}
