

void createHTTP(char *hostname, char *page, char *dataToSend, int port);
ssize_t processHTTP(int sock, char *hostname, char *page, char *dataToSend);
