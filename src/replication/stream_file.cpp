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
 * stream_file.cpp
 */

#ident "$Id$"


#include <cstdio>
#include <cassert>
#if defined (WINDOWS)
#include <io.h>
#else
#include <unistd.h>
#endif
#include "stream_file.hpp"

namespace cubstream
{

void stream_file::init (const std::string& base_name, const size_t file_size, const int print_digits)
{
  m_base_filename = base_name;
  m_desired_file_size = file_size;
  m_filename_digits_seqno = print_digits;

  m_curr_append_position = 0;
  m_curr_append_file_size = 0;

  m_start_file_seqno = 1;
  m_curr_file_seqno = 1;
}

void stream_file::finish (void)
{
  std::map<int,int>::iterator it;

  for (it = m_file_descriptors.begin (); it != m_file_descriptors.end (); it ++)
    {
      assert (it->second > 0);
      close (it->second);
    }
  m_file_descriptors.clear ();
}

int stream_file::get_file_desc_from_file_seqno (const int file_seqno)
{
  std::map<int,int>::iterator it;

  it = m_file_descriptors.find (file_seqno);
  if (it != m_file_descriptors.end ())
    {
      return it->second;
    }

  return -1;
}

int stream_file::get_file_seqno_from_stream_pos (const stream_position &pos)
{
  stream_position start_available_pos;
  int file_seqno;

  start_available_pos = m_start_file_seqno * m_desired_file_size;

  if (pos < start_available_pos)
    {
      /* not not on disk anymore */
      return -1;
    }

  if (pos >= m_curr_append_position)
    {
      /* not yet produced */
      return -1;
    }

  file_seqno = (int) (pos / m_desired_file_size);

  return file_seqno;
}

int stream_file::get_file_seqno_from_stream_pos_ext (const stream_position &pos, size_t &amount, size_t &file_offset)
{
  int file_seqno = get_file_seqno_from_stream_pos (pos);
  
  file_offset = pos - file_seqno * m_desired_file_size;

  amount = (file_seqno + 1) * m_desired_file_size - pos;

  return file_seqno;
}

int stream_file::create_files_to_pos (const stream_position &pos)
{
  /* TODO */
  return write_internal (m_curr_append_position, NULL, pos - m_curr_append_position, WRITE_MODE_CREATE);
}

int stream_file::get_filename_with_position (char *filename, const size_t max_filename, const stream_position &pos)
{
  int file_id;

  file_id = get_file_seqno_from_stream_pos (pos);
  if (file_id > 0)
    {
      return get_filename_with_file_seqno (filename, max_filename, file_id);
    }

  return ER_FAILED;
}

int stream_file::get_filename_with_file_seqno (char *filename, const size_t max_filename, const int file_seqno)
{
  assert (file_seqno >= 0);

  snprintf (filename, max_filename, "%s_%*d", m_base_filename.c_str (), m_filename_digits_seqno, file_seqno);
  return NO_ERROR;
}

int stream_file::open_file_seqno (const int file_seqno)
{
  int fd;
  char file_name [PATH_MAX];
  
  get_filename_with_file_seqno (file_name, sizeof (file_name) - 1, file_seqno);

  fd = open_file (file_name);
  if (fd < 0)
    {
      return ER_FAILED;
    }

  m_file_descriptors[file_seqno] = fd;

  return NO_ERROR;
}

int stream_file::open_file (const char *file_path)
{
  int fd;

#if defined (WINDOWS)
  fd = open (file_path, O_RDWR | O_BINARY);
#else
  fd = open (file_path, O_RDWR);
#endif

  if (fd < 0)
    {
      /* TODO[arnia] : error */
    }

  return fd;
}

int stream_file::create_file (const char *file_path)
{
  int fd;

#if defined (WINDOWS)
  fd = open (file_path, O_RDWR | O_BINARY | O_CREAT | O_EXCL);
#else
  fd = open (file_path, O_RDWR | O_CREAT | O_EXCL);
#endif

  if (fd < 0)
    {
      /* TODO[arnia] : error */
    }

  return fd;
}

size_t stream_file::read_buffer (const int file_seqno, const size_t file_offset, const char *buf, const size_t amount)
{
  size_t actual_read;
  int fd;

  fd = get_file_desc_from_file_seqno (file_seqno);

  if (fd <= 0)
    {
      fd = open_file_seqno (file_seqno);
    }

  if (fd <= 0)
    {
      return 0;
    }

#if defined (WINDOWS)
  /* TODO : use Windows API for paralel reads */
  lseek (fd, (long) file_offset, SEEK_SET);
  actual_read = read (fd, buf, amount);
#else
  actual_read = pread (fd, buf, amount, file_offset);
#endif

  return actual_read;
}

size_t stream_file::write_buffer (const int file_seqno, const size_t file_offset, const char *buf, const size_t amount)
{
  size_t actual_write;
  int fd;

  fd = get_file_desc_from_file_seqno (file_seqno);

  if (fd <= 0)
    {
      fd = open_file_seqno (file_seqno);
    }

  if (fd <= 0)
    {
      return 0;
    }

#if defined (WINDOWS)  
  lseek (fd, (long ) file_offset, SEEK_SET);
  actual_write = write (fd, buf, amount);
#else
  actual_write = pwrite (fd, buf, amount, file_offset);
#endif

  return actual_write;
}

int stream_file::write (const stream_position &pos, const char *buf, const size_t amount)
{
  stream_position curr_pos;
  size_t file_offset, rem_amount, available_amount_in_file;
  int file_id;
  int err = NO_ERROR;

  if (pos + amount >= m_curr_append_position)
    {
      create_files_to_pos (pos + amount);
    }

  curr_pos = pos;
  rem_amount = amount;

  while (rem_amount > 0)
    {
      size_t current_to_write;
      size_t actual_write;

      file_id = get_file_seqno_from_stream_pos_ext (curr_pos, available_amount_in_file, file_offset);
      if (file_id < 0)
        {
          /* TODO[arnia] : not found */
          return ER_FAILED;
        }

      current_to_write = MIN (available_amount_in_file, rem_amount);

      actual_write = write_buffer (file_id, file_offset, buf, current_to_write);
      if (actual_write < current_to_write)
        {
          return ER_FAILED;
        }

      rem_amount -= actual_write;
      curr_pos += current_to_write;
      buf += current_to_write;
    }

  return NO_ERROR;
}

int stream_file::write_internal (const stream_position &pos, const char *buf, const size_t amount, const WRITE_MODE wr_mode)
{
  stream_position curr_pos;
  size_t file_offset, rem_amount, available_amount_in_file;
  int file_id;
  int err = NO_ERROR;
  char zero_buffer[4 * 1024] = { 0 };

  curr_pos = pos;
  rem_amount = amount;

  file_id = get_file_seqno_from_stream_pos_ext (curr_pos, available_amount_in_file, file_offset);

  /*TODO */

  return NO_ERROR;
}

int stream_file::read (const stream_position &pos, const char *buf, const size_t amount)
{
  stream_position curr_pos;
  size_t available_amount_in_file, file_offset, rem_amount;
  int file_id;
  int err = NO_ERROR;

  curr_pos = pos;
  rem_amount = amount;

  while (rem_amount > 0)
    {
      size_t current_to_read;
      size_t actual_read;

      file_id = get_file_seqno_from_stream_pos_ext (curr_pos, available_amount_in_file, file_offset);
      if (file_id < 0)
        {
          /* TODO[arnia] : not found */
          return ER_FAILED;
        }

      current_to_read = MIN (available_amount_in_file, rem_amount);

      actual_read = read_buffer (file_id, file_offset, buf, current_to_read);
      if (actual_read < current_to_read)
        {
          return ER_FAILED;
        }

      rem_amount -= current_to_read;
      curr_pos += current_to_read;
      buf += current_to_read;
    }

  return NO_ERROR;
}

} /* namespace cubstream */
