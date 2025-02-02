/**
 * Copyright (c) 2021 OceanBase
 * OceanBase CE is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan PubL v2.
 * You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#ifndef _OB_RESOLVER_DEFINE_H
#define _OB_RESOLVER_DEFINE_H

#include "lib/ob_name_def.h"
#include "lib/string/ob_string.h"
#include "lib/container/ob_iarray.h"
#include "lib/container/ob_fast_array.h"
#include "lib/thread_local/ob_tsi_factory.h"
#include "lib/utility/ob_print_utils.h"
#include "lib/charset/ob_charset.h"
#include "common/object/ob_object.h"
#include "sql/resolver/ob_schema_checker.h"
#include "sql/plan_cache/ob_plan_cache_util.h"
#include "sql/parser/ob_item_type.h"

namespace oceanbase {
namespace sql {
enum ObStmtScope {
  /*
   * Expressions from different scope have different limitations,
   * we need a flag to distinguish where they are from.
   */
  T_NONE_SCOPE,
  T_FIELD_LIST_SCOPE,
  T_WHERE_SCOPE,
  T_ON_SCOPE,
  T_GROUP_SCOPE,
  T_HAVING_SCOPE,
  T_INSERT_SCOPE,
  T_UPDATE_SCOPE,
  T_AGG_SCOPE,
  T_VARIABLE_SCOPE,
  T_WHEN_SCOPE,
  T_ORDER_SCOPE,
  T_EXPIRE_SCOPE,
  T_PARTITION_SCOPE,
  T_FROM_SCOPE,
  T_LIMIT_SCOPE,
  T_PARTITION_RANGE_SCOPE,
  T_INTO_SCOPE,
  T_START_WITH_SCOPE,
  T_CONNECT_BY_SCOPE,
  T_WITH_CLAUSE_SCOPE,
  T_WITH_CLAUSE_SEARCH_SCOPE,
  T_WITH_CLAUSE_CYCLE_SCOPE,
  T_NAMED_WINDOWS_SCOPE,
  T_PL_SCOPE,
  T_LOAD_DATA_SCOPE,
  T_DBLINK_SCOPE
};

inline const char* get_scope_name(const ObStmtScope& scope)
{
  const char* str = "none";
  switch (scope) {
    case T_FROM_SCOPE:
      str = "from clause";
      break;
    case T_FIELD_LIST_SCOPE:
      str = "field list";
      break;
    case T_START_WITH_SCOPE:
      str = "start with clause";
      break;
    case T_CONNECT_BY_SCOPE:
      str = "connect by clause";
      break;
    case T_WHERE_SCOPE:
      str = "where clause";
      break;
    case T_ON_SCOPE:
      str = "on clause";
      break;
    case T_GROUP_SCOPE:
      str = "group statement";
      break;
    case T_HAVING_SCOPE:
      str = "having clause";
      break;
    case T_INSERT_SCOPE:
      str = "field list";
      break;
    case T_UPDATE_SCOPE:
      str = "field list";
      break;
    case T_AGG_SCOPE:
      str = "aggregate function";
      break;
    case T_VARIABLE_SCOPE:
      str = "set clause";
      break;
    case T_ORDER_SCOPE:
      str = "order clause";
      break;
    case T_EXPIRE_SCOPE:
      str = "expire expression";
      break;
    case T_PARTITION_SCOPE:
      str = "partition function";
      break;
    case T_NONE_SCOPE:
    default:
      break;
  }
  return str;
}

// don't use me in other place
enum { CSTRING_BUFFER_LEN = 1024 };
inline char* get_sql_string_buffer()
{
  char* ret = nullptr;
  const int64_t BUF_COUNT = 8;
  char* buf = GET_TSI(lib::ByteBuf<BUF_COUNT * CSTRING_BUFFER_LEN>);
  static RLOCAL(uint32_t, cur_buf_idx);
  if (OB_LIKELY(buf != nullptr)) {
    char(&BUFFERS)[BUF_COUNT][CSTRING_BUFFER_LEN] = *reinterpret_cast<char(*)[BUF_COUNT][CSTRING_BUFFER_LEN]>(buf);
    ret = BUFFERS[cur_buf_idx++ % BUF_COUNT];
  }
  return ret;
}

inline common::ObString concat_qualified_name(
    const common::ObString& db_name, const common::ObString& tbl_name, const common::ObString& col_name)
{
  char* buffer = get_sql_string_buffer();
  int64_t pos = 0;
  if (OB_LIKELY(buffer != nullptr)) {
    if (tbl_name.length() > 0 && db_name.length() > 0) {
      common::databuff_printf(
          buffer, CSTRING_BUFFER_LEN, pos, "%s.%s.%s", to_cstring(db_name), to_cstring(tbl_name), to_cstring(col_name));
    } else if (tbl_name.length() > 0) {
      common::databuff_printf(buffer, CSTRING_BUFFER_LEN, pos, "%s.%s", to_cstring(tbl_name), to_cstring(col_name));
    } else {
      common::databuff_printf(buffer, CSTRING_BUFFER_LEN, pos, "%s", to_cstring(col_name));
    }
  }
  return common::ObString(pos, buffer);
}

inline common::ObString concat_table_name(const common::ObString& db_name, const common::ObString& tbl_name)
{
  char* buffer = get_sql_string_buffer();
  int64_t pos = 0;
  if (OB_LIKELY(buffer != nullptr)) {
    if (db_name.length() > 0) {
      common::databuff_printf(buffer, CSTRING_BUFFER_LEN, pos, "%s.%s", to_cstring(db_name), to_cstring(tbl_name));
    } else {
      common::databuff_printf(buffer, CSTRING_BUFFER_LEN, pos, "%s", to_cstring(tbl_name));
    }
  }
  return common::ObString(pos, buffer);
}
}  // end namespace sql

namespace common {
class ObMySQLProxy;
class ObOptStatManager;
const char* get_stmt_scope_str(const sql::ObStmtScope& scope);

inline const char* get_stmt_scope_str(const sql::ObStmtScope& scope)
{
  const char* str = "NONE";
  switch (scope) {
    case sql::T_FIELD_LIST_SCOPE:
      str = "FIELD";
      break;
    case sql::T_WHERE_SCOPE:
      str = "WHERE";
      break;
    case sql::T_ON_SCOPE:
      str = "ON";
      break;
    case sql::T_GROUP_SCOPE:
      str = "GROUP";
      break;
    case sql::T_HAVING_SCOPE:
      str = "HAVING";
      break;
    case sql::T_INSERT_SCOPE:
      str = "INSERT";
      break;
    case sql::T_UPDATE_SCOPE:
      str = "UPDATE";
      break;
    case sql::T_AGG_SCOPE:
      str = "AGG";
      break;
    case sql::T_VARIABLE_SCOPE:
      str = "VARIABLE";
      break;
    case sql::T_WHEN_SCOPE:
      str = "WHEN";
      break;
    case sql::T_ORDER_SCOPE:
      str = "ORDER";
      break;
    case sql::T_EXPIRE_SCOPE:
      str = "EXPIRE";
      break;
    case sql::T_PARTITION_SCOPE:
      str = "PARTITION";
      break;
    case sql::T_NONE_SCOPE:
    default:
      break;
  }
  return str;
}

template <>
inline int databuff_print_obj(char* buf, const int64_t buf_len, int64_t& pos, const sql::ObStmtScope& scope)
{

  return databuff_printf(buf, buf_len, pos, "\"%s\"", get_stmt_scope_str(scope));
}
template <>
inline int databuff_print_key_obj(char* buf, const int64_t buf_len, int64_t& pos, const char* key,
    const bool with_comma, const sql::ObStmtScope& scope)
{
  return databuff_printf(buf, buf_len, pos, WITH_COMMA("%s:\"%s\""), key, get_stmt_scope_str(scope));
}
}  // namespace common

namespace sql {
class ObSQLSessionInfo;
class ObSchemaChecker;
class ObRawExprFactory;
class ObStmtFactory;
struct ObQueryCtx;
class ObRawExpr;
class ObConstRawExpr;
const int64_t FAST_ARRAY_COUNT = OB_DEFAULT_SE_ARRAY_COUNT;
typedef common::ObFastArray<int64_t, FAST_ARRAY_COUNT> IntFastArray;
typedef common::ObFastArray<uint64_t, FAST_ARRAY_COUNT> UIntFastArray;
typedef common::ObFastArray<ObRawExpr*, FAST_ARRAY_COUNT> RawExprFastArray;
struct ObResolverParams {
  ObResolverParams()
      : allocator_(NULL),
        schema_checker_(NULL),
        session_info_(NULL),
        query_ctx_(NULL),
        param_list_(NULL),
        select_item_param_infos_(NULL),
        prepare_param_count_(0),
        external_param_info_(),
        sql_proxy_(NULL),
        database_id_(common::OB_INVALID_ID),
        disable_privilege_check_(PRIV_CHECK_FLAG_NORMAL),
        force_trace_log_(false),
        expr_factory_(NULL),
        stmt_factory_(NULL),
        show_tenant_id_(common::OB_INVALID_ID),
        show_seed_(false),
        is_from_show_resolver_(false),
        is_restore_(false),
        is_from_create_view_(false),
        is_from_create_table_(false),
        is_prepare_protocol_(false),
        is_prepare_stage_(false),
        is_dynamic_sql_(false),
        is_dbms_sql_(false),
        statement_id_(common::OB_INVALID_ID),
        resolver_scope_stmt_type_(ObItemType::T_INVALID),
        cur_sql_(),
        contain_dml_(false),
        is_ddl_from_primary_(false),
        is_cursor_(false),
        have_same_table_name_(false),
        is_default_param_(false),
        new_gen_did_(common::OB_INVALID_ID - 1),
        new_gen_cid_(common::OB_MAX_TMP_COLUMN_ID),
        new_gen_qid_(1),
        new_cte_tid_(common::OB_MIN_CTE_TABLE_ID),
        new_gen_wid_(1),
        is_multi_table_insert_(false),
        is_resolve_table_function_expr_(false),
        has_cte_param_list_(false),
        has_recursive_word_(false),
        is_column_ref_(true),
        mv_log_table_name_ptr_(nullptr),
        mv_log_table_name_len_(0)
  {}
  bool is_force_trace_log()
  {
    return force_trace_log_;
  }

public:
  common::ObIAllocator* allocator_;
  ObSchemaChecker* schema_checker_;
  ObSQLSessionInfo* session_info_;
  ObQueryCtx* query_ctx_;
  const ParamStore* param_list_;
  const SelectItemParamInfoArray* select_item_param_infos_;
  int64_t prepare_param_count_;
  common::ObSEArray<std::pair<ObRawExpr*, ObConstRawExpr*>, 8> external_param_info_;
  common::ObMySQLProxy* sql_proxy_;
  uint64_t database_id_;
  // internal user set disable privilege check
  bool disable_privilege_check_;
  bool force_trace_log_;
  ObRawExprFactory* expr_factory_;
  ObStmtFactory* stmt_factory_;
  uint64_t show_tenant_id_;
  bool show_seed_;
  bool is_from_show_resolver_;
  bool is_restore_;
  bool is_from_create_view_;
  bool is_from_create_table_;
  bool is_prepare_protocol_;
  bool is_prepare_stage_;
  bool is_dynamic_sql_;
  bool is_dbms_sql_;
  uint64_t statement_id_;
  ObItemType resolver_scope_stmt_type_;
  common::ObString cur_sql_;
  bool contain_dml_;
  bool is_ddl_from_primary_;
  bool is_cursor_;
  bool have_same_table_name_;
  bool is_default_param_;

private:
  uint64_t new_gen_did_;
  uint64_t new_gen_cid_;
  uint64_t new_gen_qid_;
  uint64_t new_cte_tid_;
  int64_t new_gen_wid_;  // when number
  friend class ObStmtResolver;

public:
  bool is_multi_table_insert_;           // used to mark is multi table insert
  bool is_resolve_table_function_expr_;  // used to mark resolve table function expr.
  bool has_cte_param_list_;
  bool has_recursive_word_;
  bool is_column_ref_;                   // used to mark normal column ref
  // Only for ObCreateTableResolver & create materialized view mode
  // uint64_t mv_log_table_id_;             // used to mark normal column ref
  char* mv_log_table_name_ptr_;
  uint64_t mv_log_table_name_len_;
};
}  // end namespace sql
}  // end namespace oceanbase

#endif /* _OB_RESOLVER_DEFINE_H */
