#ifndef __S_STACK_H__
#define __S_STACK_H__

#include "s_frame.h"
#include "s_node.h"
#include <stack>
#include <map>
#include <string>

class SStack{

  std::stack <SFrame *> _stack;
  std::map <std::string, SNode *> vars;
  std::map <std::string, SNode *> funcs;

  public:

  SStack();
  ~SStack();

  size_t depth();

  void push_frame(SFrame * f);

  void pop_frame();

  SFrame * top();

  void add_param(std::string n, SNode * v);

  void add_var(std::string n, SNode * v);

  void add_func(std::string n, SNode * v);

  SNode * read_param(std::string n);

  int param_exists(std::string n);

  SNode * read_var(std::string n);

  int var_exists(std::string n);

  SNode * read_func(std::string n);

  int func_exists(std::string n);

};

#endif
