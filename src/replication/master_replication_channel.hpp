/*
 * Copyright (C) 2008 Search Solution Corporation. All rights reserved by Search Solution.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

/*
 * master_replication_channel.hpp
 */

#ident "$Id$"

#ifndef _MASTER_REPLICATION_CHANNEL_HPP_
#define _MASTER_REPLICATION_CHANNEL_HPP_

#include <vector>

#include "stream_common.hpp"
#include "buffer_provider.hpp"

class cubstream::packing_stream;
class cubstream::stream_buffer;

namespace cubreplication
{

class log_file;

class master_replication_channel_manager : public cubstream::buffer_provider
{
public:
  master_replication_channel_manager (const cubstream::stream_position &start_position);

  int init (const cubstream::stream_position &start_position);

  int add_buffers (std::vector <cubstream::buffer_context> &bufferred_ranges);

  int update_last_read_pos (void);

  int deffer_buffers_to_log_file (void);


  static master_replication_channel_manager *get_instance (void);


  int fetch_data (char *ptr, const size_t &amount);
  
  int flush_old_stream_data (void);

  cubstream::packing_stream * get_write_stream (void) { return generator_stream; };

private:
  /* file attached to log_generator (only for global instance) */
  log_file *m_file; 

  /* in this stream we append buffers generated by log_generator
   * At each append, notify all MRC of the last append position
   * If after this notification, there is at least one buffer not pinned by all MRCs, it means
   * some MRC is delayed (compared to log_generator), so we must flush that buffer to disk
   * Buffers which are pinned by all MRCs are maintained by MRC_M and detached (and freed) at a 
   * next iteration (call) of add_buffers after all MRC unpinns them (after received by slave)
   *
   * After buffers are flushed to disk, MRC_M is no longer concerned with old stream data
   * Each MRC is responsible to read using its own stream from disk
   */
  cubstream::packing_stream *generator_stream;

  std::vector<cubstream::stream_buffer*> send_pending_buffers;

  std::vector<cubstream::stream_buffer*> flush_pending_buffers;
};

} /* namespace cubreplication */

#endif /* _MASTER_REPLICATION_CHANNEL_HPP_ */
