// For POLLRDHUP
#define _GNU_SOURCE

#include <assert.h>
#include <stdbool.h>
#include <poll.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

#include <Rts.h>

static bool initialized = false;

static pthread_t listen_thread;
static pthread_cond_t new_conn_cond;

static pthread_mutex_t mutex;
static int listen_fd = -1;
static volatile int client_fd = -1;

#define LISTEN_BACKLOG 5

#define PRINT_ERR(...) \
  fprintf(stderr, "ghc-eventlog-socket: " __VA_ARGS__)

/*********************************************************************************
 * EventLogWriter
 *********************************************************************************/

static void writer_init(void)
{
  // no-op
}

static bool writer_write(void *eventlog, size_t sz)
{
  pthread_mutex_lock(&mutex);
  int fd = client_fd;
  if (fd < 0) {
    pthread_mutex_unlock(&mutex);
    return true;
  }

  while (sz > 0) {
    int ret = write(fd, eventlog, sz);
    if (ret == -1) {
      PRINT_ERR("failed to write: %s\n", strerror(errno));
      pthread_mutex_unlock(&mutex);
      return false;
    }

    sz -= ret;
  }

  pthread_mutex_unlock(&mutex);
  return true;
}

static void writer_flush(void)
{
  // no-op
}

static void writer_stop(void)
{
  pthread_mutex_lock(&mutex);
  if (client_fd >= 0)
    close(client_fd);

  client_fd = -1;
  pthread_mutex_unlock(&mutex);
}

const EventLogWriter socket_writer = {
  .initEventLogWriter = writer_init,
  .writeEventLog = writer_write,
  .flushEventLog = writer_flush,
  .stopEventLogWriter = writer_stop
};

/*********************************************************************************
 * Initialization
 *********************************************************************************/

static void *listen_socket(void * _unused)
{
  while (true) {
    struct sockaddr_un remote;
    int len;
    int fd = accept(listen_fd, (struct sockaddr *) &remote, &len);
    printf("lock1\n");
    pthread_mutex_lock(&mutex);
    client_fd = fd;
    // Drop lock to allow initial batch of events to be written.
    pthread_mutex_unlock(&mutex);
    startEventLogging(&socket_writer);

    // Announce new connection
    printf("lock2\n");
    pthread_cond_broadcast(&new_conn_cond);
    printf("broadcasted\n");

    // Wait for socket to disconnect before listening again.
    struct pollfd pfd = {
      .fd = client_fd,
      .events = POLLRDHUP,
      .revents = 0,
    };
    if (poll(&pfd, 1, -1) == -1) {
      PRINT_ERR("poll() failed: %s\n", strerror(errno));
    }
    printf("closed\n");
    endEventLogging();
  }

  return NULL;
}

static void open_socket(const char *sock_path)
{
  listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);

  struct sockaddr_un local;
  local.sun_family = AF_UNIX;
  strncpy(local.sun_path, sock_path, sizeof(local.sun_path) - 1);
  unlink(sock_path);
  if (bind(listen_fd, (struct sockaddr *) &local,
           sizeof(struct sockaddr_un)) == -1) {
    PRINT_ERR("failed to bind socket %s: %s\n", sock_path, strerror(errno));
    abort();
  }

  if (listen(listen_fd, LISTEN_BACKLOG) == -1)
    abort();

  int ret = pthread_create(&listen_thread, NULL, listen_socket, NULL);
  if (ret != 0) {
    PRINT_ERR("failed to spawn thread: %s\n", strerror(ret));
  }
}


static void wait_for_connection(void)
{
  pthread_mutex_lock(&mutex);
  while (client_fd == -1) {
    printf("waiting\n");
    assert(pthread_cond_wait(&new_conn_cond, &mutex) == 0);
    printf("waited\n");
  }
  pthread_mutex_unlock(&mutex);
}

/*********************************************************************************
 * Entrypoint
 *********************************************************************************/

void eventlog_socket_start(const char *sock_path, bool wait)
{
  if (!initialized) {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&new_conn_cond, NULL);
    initialized = true;
  }

  if (!sock_path) {
    sock_path = getenv("GHC_EVENTLOG_SOCKET");
  }
  if (!sock_path)
    return;

  if (eventLogStatus() == EVENTLOG_NOT_SUPPORTED) {
    PRINT_ERR("eventlog is not supported.\n");
    return;
  }

  if (eventLogStatus() == EVENTLOG_RUNNING) {
    endEventLogging();
  }

  open_socket(sock_path);
  if (wait) {
    printf("ghc-eventlog-socket: Waiting for connection to %s...\n", sock_path);
    wait_for_connection();
  }
}
