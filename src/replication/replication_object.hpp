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
 * replication_object.hpp
 */

#ident "$Id$"

#ifndef _REPLICATION_OBJECT_HPP_
#define _REPLICATION_OBJECT_HPP_

#include "packable_object.hpp"
#include "dbtype.h"
#include "storage_common.h"
#include "thread_compat.hpp"
#include "dbtype_def.h"

#include <vector>
#include <string>

namespace cubreplication
{

  enum repl_entry_type
  {
    REPL_UPDATE = 0,
    REPL_INSERT,
    REPL_DELETE
  };
  typedef enum repl_entry_type REPL_ENTRY_TYPE;

  class replication_object : public cubpacking::packable_object
  {
    public:
      virtual int apply (void) = 0;
      virtual void log_me (const char *additional_msg) = 0;
  };

  class sbr_repl_entry : public replication_object
  {
    private:
      std::string m_statement;
      std::string m_db_user;
      std::string m_sys_prm_context;

    public:
      static const int ID = 1;

      sbr_repl_entry () = default;
      ~sbr_repl_entry () = default;

      template<typename STATEMENT_T, typename DB_USER_T, typename SYS_PRM_CTX_T>
      sbr_repl_entry (STATEMENT_T &&statement, DB_USER_T &&user,
		      SYS_PRM_CTX_T &&sys_prm_ctx) : m_statement (std::forward<STATEMENT_T> (statement)),
	m_db_user (std::forward<DB_USER_T> (user)),
	m_sys_prm_context (std::forward<SYS_PRM_CTX_T> (sys_prm_ctx))
      {
      };

      int apply ();

      bool is_equal (const cubpacking::packable_object *other);

      template<typename STATEMENT_T>
      inline void set_statement (STATEMENT_T &&str)
      {
	m_statement = std::forward<STATEMENT_T> (str);
      };

      int pack (cubpacking::packer *serializator);
      int unpack (cubpacking::packer *serializator);

      std::size_t get_packed_size (cubpacking::packer *serializator);

      virtual void log_me (const char *additional_msg) override final;
  };

  class single_row_repl_entry : public replication_object
  {
    public:
      int apply ();

      virtual bool is_equal (const cubpacking::packable_object *other);

      template<typename CLASS_NAME_T>
      inline void set_class_name (CLASS_NAME_T &&class_name)
      {
	m_class_name = std::forward<CLASS_NAME_T> (class_name);
      }

      void set_key_value (cubthread::entry &thread_entry, DB_VALUE *db_val);

    protected:
      single_row_repl_entry () = default;
      virtual ~single_row_repl_entry ();

      template<typename CLASS_NAME_T>
      single_row_repl_entry (const REPL_ENTRY_TYPE type,
			     CLASS_NAME_T &&class_name) :  m_type (type),
	m_class_name (std::forward<CLASS_NAME_T> (class_name))
      {
      }

      virtual int pack (cubpacking::packer *serializator);
      virtual int unpack (cubpacking::packer *serializator);
      virtual std::size_t get_packed_size (cubpacking::packer *serializator, std::size_t start_offset = 0);

      virtual void log_me (const char *additional_msg);

      REPL_ENTRY_TYPE m_type;
      std::string m_class_name;
      DB_VALUE m_key_value;

    private:
  };

  class rec_des_row_repl_entry : public single_row_repl_entry
  {
    public:
      static const int ID = 2;

      rec_des_row_repl_entry () = default;

      template<typename CLASS_NAME_T>
      rec_des_row_repl_entry (REPL_ENTRY_TYPE type,
			      CLASS_NAME_T &&class_name,
			      RECDES *rec_des) : single_row_repl_entry (type, std::forward<CLASS_NAME_T> (class_name))
      {
	if (type != cubreplication::REPL_ENTRY_TYPE::REPL_DELETE)
	  {
	    assert (rec_des != NULL);

	    m_rec_des.length = rec_des->length;
	    m_rec_des.area_size = rec_des->area_size;
	    m_rec_des.type = rec_des->type;

	    m_rec_des.data = (char *) malloc (m_rec_des.length);
	    if (m_rec_des.data == NULL)
	      {
		assert (false);
	      }
	    memcpy (m_rec_des.data, rec_des->data, m_rec_des.length);
	  }
	else
	  {
	    memset (&m_rec_des, 0, sizeof (m_rec_des));
	  }

      }
      ~rec_des_row_repl_entry ();

      virtual int pack (cubpacking::packer *serializator) override final;
      virtual int unpack (cubpacking::packer *serializator) override final;
      virtual std::size_t get_packed_size (cubpacking::packer *serializator) override final;

      virtual bool is_equal (const cubpacking::packable_object *other) override final;
      virtual void log_me (const char *additional_msg) override final;

    private:
      RECDES m_rec_des;
  };

  class changed_attrs_row_repl_entry : public single_row_repl_entry
  {
    public:
      static const int ID = 3;

      changed_attrs_row_repl_entry () = default;

      template<typename CLASS_NAME_T, typename ATT_INT_VEC_T, typename ATT_VAL_VEC_T>
      changed_attrs_row_repl_entry (REPL_ENTRY_TYPE type, CLASS_NAME_T &&class_name,
				    const OID *inst_oid,
				    ATT_INT_VEC_T &&changed_attrs,
				    ATT_VAL_VEC_T &&new_values) : changed_attrs_row_repl_entry (type,
					  std::forward<CLASS_NAME_T> (class_name),
					  inst_oid)
      {
	m_changed_attributes = std::forward<ATT_INT_VEC_T> (changed_attrs);
	m_new_values = std::forward<ATT_VAL_VEC_T> (new_values);
      }

      template<typename CLASS_NAME_T>
      changed_attrs_row_repl_entry (REPL_ENTRY_TYPE type,
				    CLASS_NAME_T &&class_name,
				    const OID *inst_oid = NULL) : single_row_repl_entry (type, std::forward<CLASS_NAME_T> (class_name))
      {
	if (inst_oid != NULL)
	  {
	    m_inst_oid = *inst_oid;
	  }
      }

      ~changed_attrs_row_repl_entry ();

      void copy_and_add_changed_value (cubthread::entry &thread_entry,
				       const ATTR_ID att_id,
				       DB_VALUE *db_val);

      virtual int pack (cubpacking::packer *serializator) override final;
      virtual int unpack (cubpacking::packer *serializator) override final;
      virtual std::size_t get_packed_size (cubpacking::packer *serializator) override final;

      virtual bool is_equal (const cubpacking::packable_object *other) override final;
      virtual void log_me (const char *additional_msg) override final;

      inline bool compare_inst_oid (const OID *other)
      {
	return m_inst_oid.pageid == other->pageid &&
	       m_inst_oid.slotid == other->slotid &&
	       m_inst_oid.volid == other->volid;
      }

      inline void set_inst_oid (const OID *other)
      {
	assert (other != NULL);

	m_inst_oid = *other;
      }

    private:
      std::vector <int> m_changed_attributes;
      std::vector <DB_VALUE> m_new_values;

      OID m_inst_oid;
  };

} /* namespace cubreplication */

#endif /* _REPLICATION_OBJECT_HPP_ */
