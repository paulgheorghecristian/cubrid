#include "communication_channel.hpp"

#include "connection_support.h"
#include "system_parameter.h"
#include "thread_manager.hpp"
#include <string>
#include "connection_sr.h"

#if defined(WINDOWS)
#include "wintcp.h"
#else /* WINDOWS */
#include "tcp.h"
#endif /* WINDOWS */

communication_channel::communication_channel (const char *hostname,
    int port,
    int max_timeout_in_ms,
    CSS_COMMAND_TYPE command_type,
    const char *server_name) : m_conn_entry (NULL),
  m_max_timeout_in_ms (max_timeout_in_ms),
  m_hostname (hostname == NULL ? nullptr : strdup (hostname)),
  m_server_name (server_name == NULL ? nullptr : strdup (server_name)),
  m_port (port),
  m_request_id (0),
  m_type (CHANNEL_TYPE::INITIATOR),
  m_command_type (command_type)
{
  m_conn_entry = css_make_conn (-1);
}

communication_channel::communication_channel (int sock_fd,
    int max_timeout_in_ms) : m_max_timeout_in_ms (max_timeout_in_ms),
  m_hostname (nullptr),
  m_server_name (nullptr),
  m_port (-1),
  m_request_id (0),
  m_type (CHANNEL_TYPE::LISTENER),
  m_command_type (NULL_REQUEST)
{
  m_conn_entry = css_make_conn (sock_fd);
}

communication_channel::communication_channel (communication_channel &&comm) : m_max_timeout_in_ms (
    comm.m_max_timeout_in_ms)
{
  m_hostname = std::move (comm.m_hostname);
  m_server_name = std::move (comm.m_server_name);
  m_port = comm.m_port;
  m_request_id = comm.m_request_id;
  m_type = comm.m_type;
  m_command_type = comm.m_command_type;

  m_conn_entry = comm.m_conn_entry;
  comm.m_conn_entry = NULL;
}

communication_channel &communication_channel::operator= (communication_channel &&comm)
{
  this->~communication_channel ();

  new (this) communication_channel (std::forward <communication_channel> (comm));
  return *this;
}

communication_channel::~communication_channel ()
{
  close_connection ();
  assert (m_conn_entry == NULL);
}

int communication_channel::send (const char *message, int message_length)
{
  return css_net_send (m_conn_entry, message, message_length, m_max_timeout_in_ms);
}

int communication_channel::send (const std::string &message)
{
  return css_net_send (m_conn_entry, message.c_str (), message.length (), m_max_timeout_in_ms);
}

int communication_channel::recv (char *buffer, int &received_length)
{
  return css_net_recv (m_conn_entry->fd, buffer, &received_length, m_max_timeout_in_ms);
}

int communication_channel::connect ()
{
  int length = 0;

  assert (m_type == CHANNEL_TYPE::INITIATOR);
  assert (m_command_type > NULL_REQUEST && m_command_type < MAX_REQUEST);

  if (m_conn_entry == NULL || m_type != CHANNEL_TYPE::INITIATOR)
    {
      return ER_FAILED;
    }

  if (m_server_name == nullptr)
    {
      length = 0;
    }
  else
    {
      length = strlen (m_server_name.get ());
    }

  if (css_common_connect (m_conn_entry, &m_request_id, m_hostname.get (),
			  m_command_type, m_server_name.get (), length, m_port) == NULL)
    {
      close_connection ();
      return REQUEST_REFUSED;
    }

  _er_log_debug (ARG_FILE_LINE, "connect_to_master:" "connected to master_hostname:%s\n", m_hostname.get ());

  return NO_ERRORS;
}

void communication_channel::close_connection ()
{
  if (m_conn_entry != NULL)
    {
      css_free_conn (m_conn_entry);
      m_conn_entry = NULL;
    }
}

void communication_channel::set_command_type (CSS_COMMAND_TYPE cmd)
{
  m_command_type = cmd;
}

const int &communication_channel::get_max_timeout_in_ms ()
{
  return m_max_timeout_in_ms;
}

int communication_channel::wait_for (unsigned short int events, unsigned short int &revents)
{
  POLL_FD poll_fd = {0, 0, 0};
  int rc = 0;
  revents = 0;

  if (!is_connection_alive ())
    {
      return -1;
    }

  poll_fd.fd = m_conn_entry->fd;
  poll_fd.events = events;
  poll_fd.revents = 0;

  rc = css_platform_independent_poll (&poll_fd, 1, m_max_timeout_in_ms);
  revents = poll_fd.revents;

  return rc;
}

bool communication_channel::is_connection_alive ()
{
  if (m_conn_entry == NULL)
    {
      return false;
    }

  return !IS_INVALID_SOCKET (m_conn_entry->fd);
}

CSS_CONN_ENTRY *communication_channel::get_conn_entry ()
{
  return m_conn_entry;
}
