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
 * slave_replication_channel.hpp
 */

#ident "$Id$"

#ifndef _SLAVE_REPLICATION_CHANNEL_HPP_
#define _SLAVE_REPLICATION_CHANNEL_HPP_

#include "stream_provider.hpp"


struct stream_entry_header;
class replication_stream;
class serial_buffer;

class slave_replication_channel
{
public:
  int init (void);

  int receive_stream_entry_header (stream_entry_header &se_header);

  replication_stream * get_write_stream (void) { return receiving_stream; };


private:

  replication_stream *receiving_stream;

};


#endif /* _SLAVE_REPLICATION_CHANNEL_HPP_ */