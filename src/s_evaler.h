#ifndef __S_EVALER_H__
#define __S_EVALER_H__

#include "s_node.h"
#include "s_stack.h"
#include "s_reader.h"

#include <map>
#include <string>

#define TC_READY 0
#define TC_ON_CHAIN 1
#define TC_RESET 2

class SEvaler{

  SStack _stack;

  public:

  static SNode * empty_list;
  static SNode * _true;
  static SNode * _false;
  static SNode * nil;
  static std::map <std::string, int> primitives;
  static long long object_counter;

  int enable_tco;

  SEvaler();

  ~SEvaler();

  SNode * load_file(std::string file);
  SNode * read_string(std::string in);
  SNode * eval_top(SNode * ast);

  SNode * new_node(SNode * ast, s_node_type type);

  SNode * eval(SNode * ast, int tc=TC_RESET);

  void throw_exception(SNode *ast, std::string msg);

  SNode * eval_application(SNode * ast, int tc);

  SNode * tco(SNode * ast);
  SNode * eval_tc(SNode * tc);

  std::string gen_name();

  SNode * partial(SNode * ast, int rsize);

  SNode * eval_if(SNode * ast, int tc);
  SNode * eval_and(SNode * ast);
  SNode * eval_or(SNode * ast);
  SNode * eval_cond(SNode * ast, int tc);

  SNode * eval_pos(SNode * ast);
  SNode * eval_tlstr(SNode * ast);
  SNode * eval_cn(SNode * ast);
  SNode * eval_str(SNode * ast);
  SNode * eval_string_p(SNode * ast);
  SNode * eval_n_to_string(SNode * ast);
  SNode * eval_string_to_n(SNode * ast);

  SNode * eval_intern(SNode * ast);

  SNode * eval_set(SNode * ast);
  SNode * eval_value(SNode * ast);

  SNode * eval_simple_error(SNode * ast);
  SNode * eval_trap_error(SNode *ast);
  SNode * eval_error_to_string(SNode * ast);
  
  SNode * eval_cons(SNode * ast);
  SNode * eval_hd(SNode * ast);
  SNode * eval_tl(SNode * ast);
  SNode * eval_cons_p(SNode * ast);

  SNode * eval_display(SNode * ast);

  SNode * eval_func(SNode * ast);

  SNode * eval_let(SNode * ast, int tc);

  SNode * eval_equal(SNode * ast, int d=0);

  SNode * eval_eval_kl(SNode * ast);
  SNode * translate_ast(SNode * ast);

  SNode * eval_type(SNode * ast);

  SNode * eval_absvector(SNode * ast);
  SNode * eval_address_w(SNode * ast);
  SNode * eval_address_r(SNode * ast);
  SNode * eval_absvector_p(SNode * ast);

  SNode * eval_pr(SNode * ast);
  SNode * eval_read_byte(SNode * ast);
  SNode * eval_open(SNode * ast);
  SNode * eval_close(SNode * ast);

  SNode * eval_get_time(SNode * ast);

  SNode * eval_arithmetic(SNode * ast);
  SNode * eval_number_p(SNode * ast);

  int param_exists(SNode * ast);
  SNode * bind_param(SNode * ast);

  SNode * bind_var(SNode * ast);

  SNode * bind_func(SNode * ast);

};

#endif
