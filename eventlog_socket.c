#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <Rts.h>

static int listen_fd = -1;
static int client_fd = -1;

#define LISTEN_BACKLOG 5

void initEventLogging(const EventLogWriter *ev_writer);
void endEventLogging(void);

static void writer_init(void)
{
}

static void open_socket(void)
{
  char *sock_path = getenv("GHC_EVENTLOG_SOCKET");
  if (!sock_path)
    return;

  listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);

  struct sockaddr_un local, remote;
  local.sun_family = AF_UNIX;
  strncpy(local.sun_path, sock_path, sizeof(local.sun_path) - 1);
  unlink(sock_path);
  if (bind(listen_fd, (struct sockaddr *) &local,
           sizeof(struct sockaddr_un)) == -1) {
    fprintf(stderr, "failed to bind socket %s: %s\n", sock_path, strerror(errno));
    abort();
  }

  if (listen(listen_fd, LISTEN_BACKLOG) == -1)
    abort();

  fprintf(stderr, "waiting for connection to %s...", sock_path);
  int len;
  client_fd = accept(listen_fd, (struct sockaddr *) &remote, &len);
}

static bool writer_write(void *eventlog, size_t sz)
{
  if (client_fd < 0) {
    return true;
  }

  if (write(client_fd, eventlog, sz) < sz)
    return false;
  return true;
}

static void writer_flush(void)
{
}

static void writer_stop(void)
{
  if (client_fd >= 0)
    close(client_fd);
  close(listen_fd);
}

const EventLogWriter socket_writer = {
  .initEventLogWriter = writer_init,
  .writeEventLog = writer_write,
  .flushEventLog = writer_flush,
  .stopEventLogWriter = writer_stop
};

void eventlog_socket_start(void);

void eventlog_socket_start()
{
  if (RtsFlags.TraceFlags.tracing == TRACE_EVENTLOG) {
    endEventLogging();
    open_socket();
    initEventLogging(&socket_writer);
  }
}
