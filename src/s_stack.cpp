#include "s_stack.h"

using namespace std;

SStack::SStack(){
  _stack = stack<SFrame*>();
  vars = map<string, SNode*>();
  funcs = map<string, SNode*>();
}

size_t SStack::depth(){
  return _stack.size();
}

void SStack::push_frame(SFrame *f){
  SFrame * nf = new SFrame();
  if(top() != NULL)
    nf->copy(top());
  nf->copy(f);
  _stack.push(nf);
}

void SStack::pop_frame(){
  if(!_stack.empty()) {
    SFrame * f = _stack.top();
    _stack.pop();
    delete f;
  }
}

SStack::~SStack(){
  while(!_stack.empty())
    pop_frame();
}

SFrame * SStack::top(){
  if(_stack.empty())
    return NULL;
  else
    return _stack.top();
}

void SStack::add_param(string n, SNode * v) {
  SFrame * st = top();
  if(st != NULL)
    st->add_param(n, v);
}

void SStack::add_var(string n, SNode * v) {
  if(var_exists(n))
    vars.erase(n);
  vars.insert(make_pair(n, v));
}

void SStack::add_func(string n, SNode * v) {
  if(func_exists(n))
    funcs.erase(n);
  funcs.insert(make_pair(n, v));
}

SNode * SStack::read_param(string n) {
  SFrame * st = top();
  if(param_exists(n))
    return st->read_param(n);
  else
    return NULL;
}

int SStack::param_exists(string n){
  SFrame * st = top();
  if(st != NULL && st->param_exists(n))
    return 1;
  else
    return 0;
}

SNode * SStack::read_var(string n){
  if(var_exists(n))
    return vars[n];
  else
    return NULL;
}

int SStack::var_exists(string n){
  if(vars.find(n) != vars.end())
    return 1;
  else
    return 0;
}

SNode * SStack::read_func(string n){
  if(func_exists(n))
    return funcs[n];
  else
    return NULL;
}

int SStack::func_exists(string n){
  if(funcs.find(n) != funcs.end())
    return 1;
  else
    return 0;
}

