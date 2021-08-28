#ifndef EVENGLOG_SOCKET_H
#define EVENGLOG_SOCKET_H

void eventlog_socket_wait(void);
void eventlog_socket_start(const char *sock_path, bool wait);

#endif
