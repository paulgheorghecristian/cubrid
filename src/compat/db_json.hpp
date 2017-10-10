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
 * db_json.hpp - functions related to json
 */

#ifndef _DB_JSON_H
#define _DB_JSON_H

#include "error_manager.h"

#if defined (__cplusplus)
class JSON_DOC;
class JSON_VALIDATOR;
#else
typedef void JSON_DOC;
typedef void JSON_VALIDATOR;
#endif

#if defined (__cplusplus)

#include <functional>

enum DB_JSON_TYPE
{
  DB_JSON_NULL = 0,
  DB_JSON_STRING,
  DB_JSON_DOUBLE,
  DB_JSON_INT,
  DB_JSON_OBJECT,
  DB_JSON_ARRAY,
  DB_JSON_UNKNOWN,
};

/* C functions */
bool db_json_is_valid (const char *json_str);
const char *db_json_get_type_as_str (const JSON_DOC *document);
unsigned int db_json_get_length (const JSON_DOC *document);
unsigned int db_json_get_depth (const JSON_DOC *doc);
int db_json_extract_document_from_path (JSON_DOC *document, const char *raw_path, JSON_DOC *&result);
char *db_json_get_raw_json_body_from_document (const JSON_DOC *doc);
JSON_DOC *db_json_get_paths_for_search_func (const JSON_DOC *doc, const char *search_str, bool all);

void db_json_add_member_to_object (JSON_DOC *doc, const char *name, const char *value);
void db_json_add_member_to_object (JSON_DOC *doc, char *name, int value);
void db_json_add_member_to_object (JSON_DOC *doc, char *name, double value);
void db_json_add_member_to_object (JSON_DOC *doc, char *name, const JSON_DOC *value);

void db_json_add_element_to_array (JSON_DOC *doc, char *value);
void db_json_add_element_to_array (JSON_DOC *doc, int value);
void db_json_add_element_to_array (JSON_DOC *doc, double value);
void db_json_add_element_to_array (JSON_DOC *doc, const JSON_DOC *value);

int db_json_get_json_from_str (const char *json_raw, JSON_DOC *&doc);
JSON_DOC *db_json_get_copy_of_doc (const JSON_DOC *doc);
void db_json_copy_doc (JSON_DOC *dest, const JSON_DOC *src);

int db_json_insert_func (const JSON_DOC *value, JSON_DOC *doc, char *raw_path);
int db_json_remove_func (JSON_DOC *doc, char *raw_path);
int db_json_merge_func (const JSON_DOC *source, JSON_DOC *dest);

void db_json_merge_two_json_objects (JSON_DOC *obj1, const JSON_DOC *obj2);
void db_json_merge_two_json_arrays (JSON_DOC *array1, const JSON_DOC *array2);
void db_json_merge_two_json_by_array_wrapping (JSON_DOC *j1, const JSON_DOC *j2);

int db_json_object_contains_key (JSON_DOC *obj, const char *key, int &result);
const char *db_json_get_schema_raw_from_validator (JSON_VALIDATOR *val);
int db_json_validate_json (const char *json_body);

int db_json_load_validator (const char *json_schema_raw, JSON_VALIDATOR *&validator);
JSON_VALIDATOR *db_json_copy_validator (JSON_VALIDATOR *validator);
JSON_DOC *db_json_allocate_doc ();
void db_json_delete_doc (JSON_DOC *&doc);
void db_json_delete_validator (JSON_VALIDATOR *&validator);
int db_json_validate_doc (JSON_VALIDATOR *validator, JSON_DOC *doc);
bool db_json_are_validators_equal (JSON_VALIDATOR *val1, JSON_VALIDATOR *val2);

DB_JSON_TYPE db_json_get_type (const JSON_DOC *doc);

int db_json_get_int_from_document (const JSON_DOC *doc);
double db_json_get_double_from_document (const JSON_DOC *doc);
const char *db_json_get_string_from_document (const JSON_DOC *doc);
/* end of C functions */

template <typename Fn, typename... Args>
inline int
db_json_convert_string_and_call (const char *json_raw, Fn &&func, Args &&... args)
{
  JSON_DOC *doc;
  int error_code;

  error_code = db_json_get_json_from_str (json_raw, doc);
  if (error_code != NO_ERROR)
    {
      return error_code;
    }

  return func (doc, std::forward<Args> (args)...);
}
#endif /* defined (__cplusplus) */

#endif /* db_json.hpp */
