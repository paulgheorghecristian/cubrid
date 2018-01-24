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


#include "replication_buffer.hpp"

/* system headers */
#include <iostream>
#include <typeinfo>
#include <array>

namespace test_replication
{

int test_replication_buffer (void)
{
    const size_t BUFFER_TEST_SIZE = 4 * 1024 * 1024;
    char *ptr = NULL;

    replication_buffer rpl_buffer (BUFFER_TEST_SIZE);

    ptr = rpl_buffer.reserve (BUFFER_TEST_SIZE);
    if (ptr != rpl_buffer.get_buffer ())
      {
         std::cout << "  ERROR: reserve,  = " <<  xarr_buf.get_size (); 
      }


}

}
