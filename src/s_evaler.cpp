#include "s_evaler.h"
#include <time.h>

using namespace std;

SNode * SEvaler::empty_list = NULL;
SNode * SEvaler::_true = NULL;
SNode * SEvaler::_false = NULL;
SNode * SEvaler::nil = NULL;
map<string, int> SEvaler::primitives = map<string, int>();
long long SEvaler::object_counter = 0;

SEvaler::SEvaler() {
  if(empty_list==NULL){
    empty_list = new SNode(LIST);
  };
  if(_true==NULL){
    _true = new SNode(BOOL);
    _true->name = "true";
    *(_true->val.as_string) = "true";
  };
  if(_false==NULL){
    _false = new SNode(BOOL);
    _false->name = "false";
    *(_false->val.as_string) = "false";
  };
  if(nil==NULL){
    nil = new SNode(SYMBOL);
    nil->name = "NIL";
    *(nil->val.as_string) = "NIL";
  };
  int sfs = 47;
  if(primitives.size() != sfs) {
    const char * ps[] = {"if",
                   "and",
                   "or",
                   "cond",
                   "intern",
                   "pos",
                   "tlstr",
                   "cn",
                   "str",
                   "string?",
                   "n->string",
                   "string->n",
                   "set",
                   "value",
                   "simple-error",
                   "trap-error",
                   "error-to-string",
                   "cons",
                   "hd",
                   "tl",
                   "cons?",
                   "defun",
                   "lambda",
                   "let",
                   "=",
                   "eval-kl",
                   "freeze",
                   "type",
                   "absvector",
                   "address->",
                   "<-address",
                   "absvector?",
                   "pr",
                   "read-byte",
                   "open",
                   "close",
                   "get-time",
                   "+",
                   "-",
                   "*",
                   "/",
                   ">",
                   "<",
                   ">=",
                   "<=",
                   "number?",
                   "display"};
    int i;
    for(i=0; i<sfs; i++)
      primitives.insert(make_pair(string(ps[i]), 1));
  };
  enable_tco = 0;
}

SEvaler::~SEvaler(){
}

SNode * SEvaler::load_file(string file){
  SReader reader;
  SNode * ast = reader.read_file(file);
  return eval_top(ast); 
}

SNode * SEvaler::read_string(string in){
  SReader reader;
  SNode * ast = reader.read_string(in);
  return eval_top(ast);
}

SNode * SEvaler::eval_top(SNode * ast){
  SNode * res;
  list<SNode *>::iterator it;
  for(it=ast->val.as_seq->begin(); it != ast->val.as_seq->end(); it++) {
    try {
      res = eval(*it, TC_READY);
    } catch (SNode * exception) {
      printf("%s\n", exception->val.as_string->c_str());
      exit(-1);
    }
  }
  return res;
}

SNode * SEvaler::new_node(SNode * ast, s_node_type type){
  SNode * n = new SNode(type);
  n->pos = ast->pos;
  return n;
}

SNode * SEvaler::eval(SNode * ast, int tc){
  s_node_type type = ast->type;
  SNode * res;
  if(type == LIST) {
    if(ast->val.as_seq->size() == 0)
      return empty_list;
    else {
      res = eval_application(ast, tc);
      while(res->type == TC) {
        if(tc == TC_ON_CHAIN)
          return res;
        res = eval_tc(res);
      }
      return res;
    };
  } else if(type == SYMBOL && ast->name == "true")
    return _true;
  else if(type == SYMBOL && ast->name == "false")
    return _false;
  else if(type == SYMBOL && ast->name == "NIL")
    return nil;
  else if(param_exists(ast))
    return bind_param(ast);
  else
    return ast;
}

void SEvaler::throw_exception(SNode * ast, string msg){
  SNode * exception = new_node(ast, EXCEPTION);
  if(ast->pos != NULL) {
    char nmsg[1024 * 8];
    sprintf(nmsg, "%s [file: %s; line %d; col %d]", msg.c_str(),
                     ast->pos->filename.c_str(),
                     ast->pos->line,
                     ast->pos->col);
    *(exception->val.as_string) = nmsg;
  } else
    *(exception->val.as_string) = msg;;
  throw exception;
}

SNode * SEvaler::eval_application(SNode * ast, int tc){
  SNode * f = ast->val.as_seq->front();
  s_node_type ftype = f->type;
  string fn = f->name;
  SNode * res;
  f = eval(f);
  ftype = f->type;
  fn = f->name;
  if(ftype == SYMBOL)
    f = bind_func(f);
  ftype = f->type;
  fn = f->name;
  if(ftype != FUNCTION && ftype != SYMBOL)
    throw_exception(ast, string("application expect as function or symbol as the first argument but got ") + ast->type_string());
  if(fn == "if")
    res = eval_if(ast, tc);
  else if(fn == "and")
    res = eval_and(ast);
  else if(fn == "or")
    res = eval_or(ast);
  else if(fn == "cond")
    res = eval_cond(ast, tc);
  else if(fn == "intern")
    res = eval_intern(ast);
  else if(fn == "pos")
    res = eval_pos(ast);
  else if(fn == "tlstr")
    res = eval_tlstr(ast);
  else if(fn == "cn")
    res = eval_cn(ast);
  else if(fn == "str")
    res = eval_str(ast);
  else if(fn == "string?")
    res = eval_string_p(ast);
  else if(fn == "n->string")
    res = eval_n_to_string(ast);
  else if(fn == "string->n")
    res = eval_string_to_n(ast);
  else if(fn == "set")
    res = eval_set(ast);
  else if(fn == "value")
    res = eval_value(ast);
  else if(fn == "simple-error")
    res = eval_simple_error(ast);
  else if(fn == "trap-error")
    res = eval_trap_error(ast);
  else if(fn == "error-to-string")
    res = eval_error_to_string(ast);
  else if(fn == "cons")
    res = eval_cons(ast);
  else if(fn == "hd")
    res = eval_hd(ast);
  else if(fn == "tl")
    res = eval_tl(ast);
  else if(fn == "cons?")
    res = eval_cons_p(ast);
  else if(fn == "defun" || fn == "lambda" || fn == "freeze")
    res = eval_func(ast);
  else if(fn == "let")
    res = eval_let(ast, tc);
  else if(fn == "=")
    res = eval_equal(ast);
  else if(fn == "eval-kl")
    res = eval_eval_kl(ast);
  else if(fn == "type")
    res = eval_type(ast);
  else if(fn == "absvector")
    res = eval_absvector(ast);
  else if(fn == "address->")
    res = eval_address_w(ast);
  else if(fn == "<-address")
    res = eval_address_r(ast);
  else if(fn == "absvector?")
    res = eval_absvector_p(ast);
  else if(fn == "pr")
    res = eval_pr(ast);
  else if(fn == "read-byte")
    res = eval_read_byte(ast);
  else if(fn == "open")
    res = eval_open(ast);
  else if(fn == "close")
    res = eval_close(ast);
  else if(fn == "get-time")
    res = eval_get_time(ast);
  else if(fn == "+" ||
          fn == "-" ||
          fn == "*" ||
          fn == "/" ||
          fn == ">" ||
          fn == "<" ||
          fn == ">=" ||
          fn == "<=")
    res = eval_arithmetic(ast);
  else if(fn == "number?")
    res = eval_number_p(ast);
  else if(fn == "display")
    res = eval_display(ast);
  else {
    size_t size = ast->val.as_seq->size();
    SNode * fargs = f->val.as_seq->front();
    size_t fargs_size = fargs->val.as_seq->size();
    if(fargs_size > size-1)
      return partial(ast, fargs_size+1);
    //if(fargs_size != size-1)
    //  throw_exception(ast, f->name + " real arguments mismatch with formal arguments");
    SNode * rargs[size-1];
    list<SNode*>::iterator it;
    it = ast->val.as_seq->begin();
    it++;
    int i = 0;
    for(; it != ast->val.as_seq->end(); it++) {
      SNode * arg = eval((*it));
      rargs[i] = arg;
      i++;
    };
    _stack.push_frame(f->scope);
    it = fargs->val.as_seq->begin();
    i = 0;
    for(; it != fargs->val.as_seq->end(); it++) {
      _stack.add_param(*((*it)->val.as_string), rargs[i]);
      i++;
    };
    if(f->val.as_seq->back()->type == LIST && tc != TC_RESET && enable_tco == 1)
      res = tco(f->val.as_seq->back());
    else
      res = eval(f->val.as_seq->back(), TC_READY);
    _stack.pop_frame();
    if(i<size-1) {
      SNode * rp = new_node(ast, LIST);
      rp->val.as_seq->push_back(res);
      for(;i<size-1;i++) {
        rp->val.as_seq->push_back(rargs[i]);
      };
      res = eval(rp);
    };
  }
  return res;
}

SNode * SEvaler::tco(SNode * ast){
  s_node_type type = ast->type;
  SNode * tc;
  SFrame * f;
  if(type != LIST)
    throw_exception(ast, string("expects a list for tco but got ") + ast->type_string());
  tc = new_node(ast, TC);
  tc->val = ast->val;
  f = new SFrame();
  if(_stack.top() != NULL)
    f->copy(_stack.top());
  tc->scope = f;
  return tc; 
}

SNode * SEvaler::eval_tc(SNode * tc){
  s_node_type type = tc->type;
  SNode * app;
  SNode * res;
  SFrame * f;
  if(type != TC)
    throw_exception(tc, string("expects a tc for evalution but got ") + tc->type_string());
  app = new_node(tc, LIST);
  app->val = tc->val;
  _stack.push_frame(tc->scope);
  res = eval(app, TC_ON_CHAIN);
  _stack.pop_frame();
  return res; 
}

string SEvaler::gen_name(){
  char n[1024];
  sprintf(n, "___ %d", object_counter);
  object_counter ++;
  return n;
}

SNode * SEvaler::partial(SNode * ast, int rsize){
  size_t size = ast->val.as_seq->size();
  SNode * p = new_node(ast, FUNCTION);
  SFrame * f = new SFrame();
  SNode * fargs = new_node(ast, LIST);
  SNode * body = new_node(ast, LIST);
  if(_stack.top() != NULL)
    f->copy(_stack.top());
  body->val.as_seq->push_back(ast->val.as_seq->front());
  list<SNode *>::iterator it;
  it = ast->val.as_seq->begin();
  it++;
  int i = 0;
  for(; it != ast->val.as_seq->end(); it++) {
    SNode * a = eval((*it));
    string n = gen_name();
    SNode * nn = new_node(ast, SYMBOL);
    f->add_param(n, a);
    *(nn->val.as_string) = n;
    nn->name = *(nn->val.as_string);
    body->val.as_seq->push_back(nn);
    i++;
  };
  for(; i<rsize-1; i++) {
    string n = gen_name();
    SNode * nn = new_node(ast, SYMBOL);
    *(nn->val.as_string) = n;
    nn->name = *(nn->val.as_string);
    fargs->val.as_seq->push_back(nn);
    body->val.as_seq->push_back(nn);
  };
  p->val.as_seq->push_back(fargs);
  p->val.as_seq->push_back(body);
  p->scope = f;
  p->name = string("#<function :lambda ") + fargs->to_string() + body->to_string() + ">";
  return p;
}

SNode * SEvaler::eval_if(SNode * ast, int tc){
  size_t size = ast->val.as_seq->size();
  SNode * res;
  list<SNode *>::iterator it;
  if(size != 3 && size != 4)
    throw_exception(ast, "if expects 2 or 3 arguments");
  it = ast->val.as_seq->begin();
  it++;
  SNode * c = eval((*it));
  if(c->type != BOOL)
    throw_exception(ast, string("if expects a boolean as the first argument but got ") + c->type_string());
  if(c == _true) {
    it++;
    SNode * l = *it;
    if(l->type == LIST && tc != TC_RESET && enable_tco == 1)
      res = tco(l);
    else
      res = eval(l, TC_READY);
  } else {
    if(size == 4) {
      it++;
      it++;
      SNode * r = *it;
      if(r->type == LIST && tc != TC_RESET && enable_tco == 1)
        res = tco(r);
      else
        res = eval(r, TC_READY);
    } else
      res = nil; 
  }
  return res;
}

SNode * SEvaler::eval_and(SNode * ast){
  size_t size = ast->val.as_seq->size();
  SNode * res;
  if(size < 3)
    return partial(ast, 3);
  list<SNode *>::iterator it;
  it = ast->val.as_seq->begin();
  it++;
  for(; it != ast->val.as_seq->end(); it++) {
    res = eval((*it));
    if(res->type != BOOL)
      throw_exception(ast, string("and expects boolean as arguments but got ") + res->type_string());
    if(res == _false)
      return _false;
  }
  return _true;
}

SNode * SEvaler::eval_or(SNode * ast){
  size_t size = ast->val.as_seq->size();
  SNode * res;
  if(size < 3)
    return partial(ast, 3);
  list<SNode *>::iterator it;
  it = ast->val.as_seq->begin();
  it++;
  for(; it != ast->val.as_seq->end(); it++) {
    res = eval((*it));
    if(res->type != BOOL)
      throw_exception(ast, string("or expects boolean as arguments but got ") + res->type_string());
    if(res == _true)
      return _true;
  }
  return _false;
}

SNode * SEvaler::eval_cond(SNode * ast, int tc){
  size_t size = ast->val.as_seq->size();
  SNode * res;
  SNode * p;
  if(size < 2)
    throw_exception(ast, "cond expects >= 1 arguments");
  list<SNode *>::iterator it;
  it = ast->val.as_seq->begin();
  it++;
  for(; it != ast->val.as_seq->end(); it++) {
    SNode * c;
    p = *it;
    if(p->type != LIST)
      throw_exception(ast, string("cond expects pair as arguments but got ") + p->type_string());
    size_t psize = p->val.as_seq->size();
    if(psize != 2)
      throw_exception(ast, "cond expects pair as arguments");
    c = eval(p->val.as_seq->front());
    if(c->type != BOOL)
      throw_exception(ast, string("cond's pair expects a boolean as the first argument but got ") + c->type_string());
    if(c == _true) {
      if(p->val.as_seq->back()->type == LIST && tc != TC_RESET && enable_tco == 1)
        res = tco(p->val.as_seq->back());
      else 
        res = eval(p->val.as_seq->back(), TC_READY);
      return res;
    }
  }
  return nil;
}

SNode * SEvaler::eval_pos(SNode * ast){
  size_t size = ast->val.as_seq->size();
  SNode * res;
  if(size < 3)
    return partial(ast, 3);
  if(size != 3)
    throw_exception(ast, "pos expects 2 arguments");
  list<SNode *>::iterator it;
  it = ast->val.as_seq->begin();
  it++;
  SNode * s = eval((*it));
  if(s->type != STRING)
    throw_exception(ast, string("pos expects a string as the first argument but got ") + s->type_string());
  it++;
  SNode * i = eval((*it));
  if(i->type != NUMBER)
    throw_exception(ast, string("pos expects a number as the second argument but got ") + i->type_string());
  res = new_node(ast, STRING);
  size_t p = *(i->val.as_number);
  if(p >= s->val.as_string->length())
    throw_exception(ast, "the index of pos is out of bounds");
  else
    *(res->val.as_string) = s->val.as_string->at(*(i->val.as_number));
  return res;
}

SNode * SEvaler::eval_tlstr(SNode * ast){
  size_t size = ast->val.as_seq->size();
  SNode * res;
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "tlstr expects 1 argument");
  list<SNode *>::iterator it;
  it = ast->val.as_seq->begin();
  it++;
  SNode * s = eval((*it));
  if(s->type != STRING)
    throw_exception(ast, string("tlstr expects a string as the first argument but got ") + s->type_string());
  res = new_node(ast, STRING);
  if(s->val.as_string->length() == 0)
    throw_exception(ast, "tlstr attempted to take tail fo an empty string ");
  else if(s->val.as_string->length() == 1)
    *(res->val.as_string) = "";
  else
    *(res->val.as_string) = s->val.as_string->substr(1, s->val.as_string->length()-1);
  return res;
}

SNode * SEvaler::eval_cn(SNode * ast){
  size_t size = ast->val.as_seq->size();
  SNode * res;
  if(size < 3)
    return partial(ast, 3);
  if(size != 3)
    throw_exception(ast, "cn expects 2 arguments");
  list<SNode *>::iterator it;
  it = ast->val.as_seq->begin();
  it++;
  SNode * l = eval((*it));
  if(l->type != STRING)
    throw_exception(ast, string("cn expects a string as the first argument but got ") + l->type_string());
  it++;
  SNode * r = eval((*it));
  if(r->type != STRING)
    throw_exception(ast, string("cn expects a string as the first argument but got ") + r->type_string());
  res = new_node(ast, STRING);
  *(res->val.as_string) = *(l->val.as_string) + *(r->val.as_string);
  return res;
}

SNode * SEvaler::eval_str(SNode * ast){
  size_t size = ast->val.as_seq->size();
  SNode * res;
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "str expects 1 argument");
  list<SNode *>::iterator it;
  it = ast->val.as_seq->begin();
  it++;
  SNode * s = eval((*it));
  res = new_node(ast, STRING);
  *(res->val.as_string) = s->to_string();
  return res;
}

SNode * SEvaler::eval_string_p(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "string? expects 1 argument");
  SNode * s = eval(ast->val.as_seq->back());
  if(s->type == STRING)
    return _true;
  else
    return _false;
}


SNode * SEvaler::eval_n_to_string(SNode * ast){
  size_t size = ast->val.as_seq->size();
  SNode * res;
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "n->string expects 1 argument");
  SNode * n = eval(ast->val.as_seq->back());
  if(n->type != NUMBER)
    throw_exception(ast, string("n->string expects a number as the first argument but got ") + n->type_string());
  if(*(n->val.as_number) < 0 || *(n->val.as_number) > 255)
    throw_exception(ast, "n->string expects a number > 0 and < 256");
  res = new_node(ast, STRING);
  *(res->val.as_string) = char(*(n->val.as_number));
  return res;
}

SNode * SEvaler::eval_string_to_n(SNode * ast){
  size_t size = ast->val.as_seq->size();
  SNode * res;
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "string->n expects 1 argument");
  SNode * s = eval(ast->val.as_seq->back());
  if(s->type != STRING || s->val.as_string->length() != 1)
    throw_exception(ast, string("string->n expects a char as the first argument but got ") + s->type_string());
  res = new_node(ast, NUMBER);
  *(res->val.as_number) = int(s->val.as_string->at(0));
  return res;
}

SNode * SEvaler::eval_intern(SNode *ast){
  size_t size = ast->val.as_seq->size();
  SNode * res;
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "intern expects 1 argument");
  SNode * s= eval(ast->val.as_seq->back());
  if(s->type != STRING)
    throw_exception(ast, string("intern expects a string as the argument but got ") + s->type_string());
  res = new_node(ast, SYMBOL);
  *(res->val.as_string) = *(s->val.as_string);
  res->name = *(res->val.as_string);
  return res;
}

SNode * SEvaler::eval_set(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 3)
    return partial(ast, 3);
  if(size != 3)
    throw_exception(ast, "set expects 2 arguments");
  list<SNode*>::iterator it = ast->val.as_seq->begin();
  it++;
  SNode * n = eval((*it));
  if(n->type != SYMBOL)
    throw_exception(ast, string("set expects a symbol as the first argument but got ") + n->type_string());
  it++;
  SNode * v = eval((*it));
  _stack.add_var(*(n->val.as_string), v);
  return v;
}

SNode * SEvaler::eval_value(SNode * ast){
  size_t size = ast->val.as_seq->size();
  SNode * res;
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "value expects 1 argument");
  SNode * n = eval(ast->val.as_seq->back());
  if(n->type != SYMBOL)
    throw_exception(ast, ("value expects a symbol as the argument but got") + n->type_string());
  res = bind_var(n);
  return res;
}

SNode * SEvaler::eval_simple_error(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "simple-error expects 1 argument");
  SNode * msg = eval(ast->val.as_seq->back());
  if(msg->type != STRING)
    throw_exception(ast, string("simple-error expects a string as the argument but got") + msg->type_string());
  SNode * e = new_node(ast, EXCEPTION);
  *(e->val.as_string) = *(msg->val.as_string);
  throw e;
}

SNode * SEvaler::eval_trap_error(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 3)
    return partial(ast, 3);
  if(size != 3)
    throw_exception(ast, "trap-error expects 2 arguments");
  list<SNode*>::iterator it = ast->val.as_seq->begin();
  it++;
  SNode * res;
  size_t stack_depth = _stack.depth();
  try {
    res = eval((*it));
  } catch (SNode * exception){
    while(_stack.depth() > stack_depth) {
      _stack.pop_frame();
    }
    it++;
    SNode * cb = eval((*it));
    if(cb->type != FUNCTION)
      throw_exception(ast, string("trap-error expects a function as the second argument but got") + cb->type_string());
    SNode * fc = new_node(ast, LIST);
    fc->val.as_seq->push_back(cb);
    fc->val.as_seq->push_back(exception);
    res = eval(fc);
  }
  return res;
}

SNode * SEvaler::eval_error_to_string(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "error-to-string expects 1 argument");
  SNode * e = eval(ast->val.as_seq->back());
  if(e->type != EXCEPTION)
    throw_exception(ast, string("error-to-string expects an exception as the argument but got ") + e->type_string());
  SNode * res = new_node(ast, STRING);
  *(res->val.as_string) = *(e->val.as_string);
  return res;
}

SNode * SEvaler::eval_cons(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 3)
    return partial(ast, 3);
  if(size != 3)
    throw_exception(ast, "cons expects 2 arguments");
  list<SNode*>::iterator it = ast->val.as_seq->begin();
  it++;
  SNode * i = eval((*it));
  it++;
  SNode * l = eval((*it));
  SNode * res = new_node(ast, CONS);
  res->val.as_seq->push_back(i);
  res->val.as_seq->push_back(l);
  return res;
}

SNode * SEvaler::eval_hd(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "hd expects 1 argument");
  SNode * l = eval(ast->val.as_seq->back());
  if(l->type != CONS)
    throw_exception(ast, string("hd expects a list as the second argument but got ") + l->type_string());
  SNode * res = l->val.as_seq->front();
  return res;
}

SNode * SEvaler::eval_tl(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "tl expects 1 argument");
  SNode * l = eval(ast->val.as_seq->back());
  if(l->type != CONS)
    throw_exception(ast, string("tl expects a list as the second argument but got ") + l->type_string());
  SNode * res = l->val.as_seq->back();
  return res;
}

SNode * SEvaler::eval_cons_p(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "cons? expects 1 argument");
  SNode * l = eval(ast->val.as_seq->back());
  if(l->type == CONS)
    return _true;
  else
    return _false;
}

SNode * SEvaler::eval_display(SNode * ast){
  size_t size = ast->val.as_seq->size();
  SNode * res;
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "display expects 1 argument arguments");
  res = eval(ast->val.as_seq->back());
  res->show();
  return res;
}

SNode * SEvaler::eval_func(SNode * ast){
  size_t fsize = ast->val.as_seq->size();
  SNode * f = new_node(ast, FUNCTION);
  SNode * fargs;
  string defn;
  SFrame * scope = new SFrame();
  string fn = ast->val.as_seq->front()->name;
  list<SNode*>::iterator it;
  it = ast->val.as_seq->begin();
  if(fn == "defun") {
    SNode * n;
    s_node_type ntype;
    if(fsize < 4)
      return partial(ast, 4);
    if(fsize != 4)
      throw_exception(ast, "defun expects 3 arguments arguments");
    it++;
    n = eval((*it));
    ntype = n->type;
    if(ntype != SYMBOL)
      throw_exception(ast, string("defun expect a symbol as function name but got ") + n->type_string());
    defn = *(n->val.as_string);
    it++;
    fargs = (*it);
  } else if(fn == "lambda") {
    if(fsize < 3)
      return partial(ast, 3);
    if(fsize != 3)
      throw_exception(ast, "lambda expects 2 arguments arguments");
    fargs = new_node(ast, LIST);
    it++;
    fargs->val.as_seq->push_back((*it));
  } else {
    if(fsize < 2)
      return partial(ast, 2);
    if(fsize != 2)
      throw_exception(ast, "freeze expects 1 argument arguments");
    fargs = new_node(ast, LIST);
  };
  if(fargs->type != LIST)
    throw_exception(ast, string("defun or lambda expects a list as function formal arguments but got ") + fargs->type_string());
  list<SNode*>::iterator fit = fargs->val.as_seq->begin();
  for(; fit != fargs->val.as_seq->end(); fit++) {
    if((*fit)->type != SYMBOL)
      throw_exception(ast, string("defun or lambda expects a symbol as function formal argument but got ") + (*fit)->type_string());
  };
  f->val.as_seq->push_back(fargs);
  it++;
  f->val.as_seq->push_back((*it));
  if(fn == "defun") {
    f->name = defn;
  } else if(fn == "lambda") {
    f->name = string("#<function :lambda ") + fargs->to_string() + (*it)->to_string() + ">";
  } else {
    f->name = string("#<continuation :") + (*it)->to_string() + ">";
  };
  if(fn == "defun")
    _stack.add_func(defn, f);
  if(_stack.top() != NULL)
    scope->copy(_stack.top());
  f->scope = scope;
  return f;
}

SNode * SEvaler::eval_let(SNode * ast, int tc){
  size_t size = ast->val.as_seq->size();
  if(size != 4)
    throw_exception(ast, "let expects 3 arguments");
  list<SNode*>::iterator it = ast->val.as_seq->begin();
  it++;
  SNode * n = (*it);
  if(n->type != SYMBOL)
    throw_exception(ast, string("let expects a symbol as the first argument but got ") + n->type_string());
  it++;
  SNode * v = eval((*it));
  SFrame * f = new SFrame();
  _stack.push_frame(f);
  _stack.add_param(*(n->val.as_string), v);
  it++;
  SNode * res;
  if((*it)->type == LIST && tc != TC_RESET && enable_tco == 1)
    res = tco((*it));
  else
    res = eval((*it), TC_READY);
  _stack.pop_frame();
  return res;
}

SNode * SEvaler::eval_equal(SNode * ast, int d){
  size_t size = ast->val.as_seq->size();
  if(size < 3)
    return partial(ast, 3);
  if(size != 3)
    throw_exception(ast, "= expects 2 arguments");
  list<SNode*>::iterator it = ast->val.as_seq->begin();
  it++;
  SNode * l;
  if(d == 0)
    l = eval((*it));
  else
    l = (*it);
  SNode * r;
  it++;
  if(d == 0)
    r = eval((*it));
  else
    r = (*it);
  if(l->type != r->type)
    return _false;
  else if(l->type == STRING || l->type == SYMBOL) {
    if(*(l->val.as_string) == *(r->val.as_string))
      return _true;
  } else if(l->type == NUMBER) {
    if(*(l->val.as_number) == *(r->val.as_number))
      return _true;
  } else if(l->type == VECTOR) {
    if(l->val.as_vector->size() != r->val.as_vector->size())
      return _false;
    vector<SNode*>::iterator it = l->val.as_vector->begin();
    vector<SNode*>::iterator it2 = r->val.as_vector->begin();
    for(; it != l->val.as_vector->end(); it++){
      SNode * e = new_node(ast, LIST);
      SNode * ef = new_node(ast, SYMBOL);
      *(ef->val.as_string) = "=";
      ef->name = *(ef->val.as_string);
      e->val.as_seq->push_back(ef);
      e->val.as_seq->push_back((*it));
      e->val.as_seq->push_back((*it2));
      it2++;
      SNode * er = eval_equal(e, d+1);
      if(er == _false)
        return _false;
    }
    return _true;
  } else if(l->type == CONS) {
    if(l->val.as_seq->size() != r->val.as_seq->size())
      return _false;
    list<SNode*>::iterator it = l->val.as_seq->begin();
    list<SNode*>::iterator it2 = r->val.as_seq->begin();
    for(; it != l->val.as_seq->end(); it++) {
      SNode * e = new_node(ast, LIST);
      SNode * ef = new_node(ast, SYMBOL);
      *(ef->val.as_string) = "=";
      ef->name = *(ef->val.as_string);
      e->val.as_seq->push_back(ef);
      e->val.as_seq->push_back((*it));
      e->val.as_seq->push_back((*it2));
      it2++;
      SNode * er = eval_equal(e, d+1);
      if(er == _false)
        return _false;
    }
    return _true;
  } else if(l == r) {
    return _true;
  }
  return _false;
}

SNode * SEvaler::eval_eval_kl(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "eval-kl expects 1 argument");
  SNode * res = eval(ast->val.as_seq->back());
  if(res->type == CONS)
    res = eval(translate_ast(res));
  return res;
}

SNode * SEvaler::translate_ast(SNode * ast){
  SNode * res;
  if(ast->type == CONS) {
    SNode * t = ast;
    res = new_node(ast, LIST);
    while(t->type == CONS) {
      res->val.as_seq->push_back(translate_ast(t->val.as_seq->front()));
      t = t->val.as_seq->back();
    }
  } else {
    res = ast;
  };
  return res;
}

SNode * SEvaler::eval_type(SNode * ast){
  return nil;
}

SNode * SEvaler::eval_absvector(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "absvector expects 1 argument");
  SNode * res = new_node(ast, VECTOR); 
  SNode * i = eval(ast->val.as_seq->back());
  if(i->type != NUMBER)
    throw_exception(ast, string("absvector expects a number as the first argument but got ") + i->type_string());
  if(*(i->val.as_number) == 0)
    throw_exception(ast, "absvector size should > 0");
  *(res->val.as_vector) = vector<SNode*>(size_t(*(i->val.as_number)), NULL);
  return res;
}

SNode * SEvaler::eval_address_w(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 4)
    return partial(ast, 4);
  if(size != 4)
    throw_exception(ast, "address-> expects 3 arguments");
  list<SNode*>::iterator it = ast->val.as_seq->begin();
  it++;
  SNode * vec = eval((*it));
  if(vec->type != VECTOR)
    throw_exception(ast, string("address-> expects an absvector as the first argument but got ") + vec->type_string());
  it++;
  SNode * i = eval((*it));
  if(i->type != NUMBER)
    throw_exception(ast, string("address-> expects an number as the second argument but got ") + i->type_string());
  if(*(i->val.as_number) >= vec->val.as_vector->size())
    throw_exception(ast, "address-> index > size"); 
  it++;
  SNode * v = eval((*it));
  (*(vec->val.as_vector))[int(*(i->val.as_number))] = v;
  return vec;
}

SNode * SEvaler::eval_address_r(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 3)
    return partial(ast, 3);
  if(size != 3)
    throw_exception(ast, "<-address expects 2 arguments");
  list<SNode*>::iterator it = ast->val.as_seq->begin();
  it++;
  SNode * vec = eval((*it));
  if(vec->type != VECTOR)
    throw_exception(ast, string("<-address expects an absvector as the first argument but got ") + vec->type_string());
  it++;
  SNode * i = eval((*it));
  if(i->type != NUMBER)
    throw_exception(ast, string("<-address expects an number as the second argument but got ") + i->type_string());
  if(*(i->val.as_number) >= vec->val.as_vector->size())
    throw_exception(ast, "<-address index > size ");
  SNode * v = vec->val.as_vector->at(int(*(i->val.as_number)));
  return v;
}

SNode * SEvaler::eval_absvector_p(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "absvector? expects 1 argument");
  SNode * vec = eval(ast->val.as_seq->back());
  if(vec->type == VECTOR)
    return _true;
  else
    return _false;
}

SNode * SEvaler::eval_pr(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 3)
    return partial(ast, 3);
  if(size != 3)
    throw_exception(ast, "pr expects 2 arguments");
  list<SNode*>::iterator it = ast->val.as_seq->begin();
  it++;
  SNode * str = eval((*it));
  if(str->type != STRING)
    throw_exception(ast, string("pr expects a string as the first argument but got ") + str->type_string());
  it++;
  SNode * s = eval((*it));
  if(s->type != STREAM_OUT && s->type != NUMBER)
    throw_exception(ast, string("pr expects a stream-out or number as the first argument but got ") + s->type_string());
  if(s->type == NUMBER)
    printf("%s", str->val.as_string->c_str());
  else {
    FILE * sid = (FILE *)(long)(*(s->val.as_number));
    fprintf(sid, "%s", str->val.as_string->c_str());
  };
  return str;
}

SNode * SEvaler::eval_read_byte(SNode * ast){
  size_t size = ast->val.as_seq->size();
  char c;
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "read-byte expects 1 argument");
  SNode * s = eval(ast->val.as_seq->back());
  if(s->type != STREAM_IN && s->type != NUMBER)
    throw_exception(ast, string("read-byte expects a stream-in or number as the first argument but got ") + s->type_string());
  if(s->type == NUMBER)
    c = getc(stdin);
  else {
    FILE * sid = (FILE *)(long)(*(s->val.as_number));
    c = fgetc(sid);
  }
  SNode * res = new_node(ast, NUMBER);
  *(res->val.as_number) = c;
  return res;
}

SNode * SEvaler::eval_open(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 4)
    return partial(ast, 4);
  if(size != 4)
    throw_exception(ast, "open expects 3 arguments");
  list<SNode*>::iterator it = ast->val.as_seq->begin();
  it++;
  SNode * a1 = eval((*it));
  if(a1->type != SYMBOL || *(a1->val.as_string) != "file")
    throw_exception(ast, string("open expects a symbol file as the first argument but got ") + a1->type_string());
  it++;
  SNode * a2 = eval((*it));
  if(a2->type != STRING)
    throw_exception(ast, string("open expects a string as the second argument but got ") + a2->type_string());
  it++;
  SNode * a3 = eval((*it));
  if(a3->type != SYMBOL || (*(a3->val.as_string) != "in" && *(a3->val.as_string) != "out"))
    throw_exception(ast, string("open expects a symbol in or out as the third argument but got ") + a3->type_string());
  FILE * sid;
  SNode * s;
  if(*(a3->val.as_string) == "in") {
    sid = fopen(a2->val.as_string->c_str(), "r");
    s = new_node(ast, STREAM_IN);
  } else {
    sid = fopen(a2->val.as_string->c_str(), "w");
    s = new_node(ast, STREAM_OUT);
  }
  if(sid == NULL)
    throw_exception(ast, string("cannot open file ") + *(a2->val.as_string));
  *(s->val.as_number) = long(sid);
  return s;
}

SNode * SEvaler::eval_close(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "close expects 1 argument");
  SNode * s = eval(ast->val.as_seq->back());
  if(s->type != STREAM_IN && s->type != STREAM_OUT)
    throw_exception(ast, string("close expects a stream as the first argument but got ") + s->type_string());
  FILE * sid = (FILE *)(long)(*(s->val.as_number));
  fclose(sid);
  return empty_list;
}

SNode * SEvaler::eval_get_time(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size != 2)
    throw_exception(ast, "get-time expects 1 arguments");
  SNode * mod = eval(ast->val.as_seq->back());
  if(mod->type != SYMBOL || (*(mod->val.as_string) != "run" && *(mod->val.as_string) != "real"))
    throw_exception(ast, string("get-time expects a symbol run or real as the first argument but got ") + mod->type_string());
  time_t t = time(NULL);
  SNode * res = new_node(ast, NUMBER);
  *(res->val.as_number) = t;
  return res;
}

SNode * SEvaler::eval_arithmetic(SNode * ast){
  size_t size = ast->val.as_seq->size();
  string fn = *(eval(ast->val.as_seq->front())->val.as_string);
  if(size < 3)
    return partial(ast, 3);
  if(size != 3)
    throw_exception(ast, fn + " expects 2 arguments");
  list<SNode*>::iterator it = ast->val.as_seq->begin();
  it++;
  SNode * l = eval((*it));
  if(l->type != NUMBER)
    throw_exception(ast, fn + " expects a number as the first argument but got " + l->type_string());
  it++;
  SNode * r = eval((*it));
  if(r->type != NUMBER)
    throw_exception(ast, fn + " expects a number as the second argument but got " + r->type_string());
  SNode * res;
  if(fn == "+") {
    res = new_node(ast, NUMBER);
    *(res->val.as_number) = *(l->val.as_number) + *(r->val.as_number);
  } else if(fn == "-") {
    res = new_node(ast, NUMBER);
    *(res->val.as_number) = *(l->val.as_number) - *(r->val.as_number);
  } else if(fn == "*") {
    res = new_node(ast, NUMBER);
    *(res->val.as_number) = *(l->val.as_number) * *(r->val.as_number);
  } else if(fn == "/") {
    if(*(r->val.as_number) == 0)
      throw_exception(ast, "/ get a 0 as the second argument");
    res = new_node(ast, NUMBER);
    *(res->val.as_number) = *(l->val.as_number) / *(r->val.as_number);
  } else if(fn == ">") {
    if(*(l->val.as_number) > *(r->val.as_number))
      return _true;
    else
      return _false;
  } else if(fn == "<") {
    if(*(l->val.as_number) < *(r->val.as_number))
      return _true;
    else
      return _false;
  } else if(fn == ">=") {
    if(*(l->val.as_number) >= *(r->val.as_number))
      return _true;
    else
      return _false;
  } else if(fn == "<=") {
    if(*(l->val.as_number) <= *(r->val.as_number))
      return _true;
    else
      return _false;
  }
  return res;
}

SNode * SEvaler::eval_number_p(SNode * ast){
  size_t size = ast->val.as_seq->size();
  if(size < 2)
    return partial(ast, 2);
  if(size != 2)
    throw_exception(ast, "number? expects 1 argument");
  SNode * n = eval(ast->val.as_seq->back());
  if(n->type == NUMBER)
    return _true;
  else
    return _false;
}

int SEvaler::param_exists(SNode * ast){
  s_node_type type = ast->type;
  if(type == SYMBOL) {
    string str = *(ast->val.as_string);
    return _stack.param_exists(str);
  } else
    return 0;
}

SNode * SEvaler::bind_param(SNode * ast){
  s_node_type type = ast->type;
  if(type == SYMBOL) {
    string str = *(ast->val.as_string);
    return _stack.read_param(str);
  } else
    throw_exception(ast, string("expect a symbol but got ") + ast->type_string());
}

SNode * SEvaler::bind_var(SNode * ast){
  s_node_type type = ast->type;
  if(type == SYMBOL) {
    string str = *(ast->val.as_string);
    if(_stack.var_exists(str))
      return _stack.read_var(str);
    throw_exception(ast, string("cannot find variable ") + str);
  } else
    throw_exception(ast, string("expect a symbol but got ") + ast->type_string());
}

SNode * SEvaler::bind_func(SNode * ast){
  s_node_type type = ast->type;
  if(type == SYMBOL) {
    string str = *(ast->val.as_string);
    if(primitives.find(str) != primitives.end())
      return ast;
    else if(_stack.param_exists(str)) {
      SNode * f = _stack.read_param(str);
      if(f->type == SYMBOL)
        return bind_func(f);
      else
        return f;
    } else {
      if(_stack.func_exists(str))
        return _stack.read_func(str);
      throw_exception(ast, string("cannot find function ") + str);
    }
  } else
    throw_exception(ast, string("expect a symbol but got ") + ast->type_string());
}


