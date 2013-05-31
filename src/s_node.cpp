#include "s_node.h"

using namespace std;

SNode::SNode(s_node_type t){
  type = t;
  name = "";
  if(t == SYMBOL || t == BOOL || t == STRING || t == EXCEPTION || t == TC) {
    val.as_string = new string("");
  } else if(t == NUMBER || t == STREAM_OUT || t == STREAM_IN) {
    val.as_number = (double *)malloc(sizeof(double));
  } else if(t == LIST || t == FUNCTION || t == CONS)
    val.as_seq = new list<SNode*>();
  else if(t == VECTOR)
    val.as_vector = new vector<SNode*>();
  else
    error("unsupported type");
  pos = NULL;
  scope = NULL;
}

SNode::~SNode(){
  if(val.as_string != NULL)
    delete val.as_string;
  if(pos != NULL)
    delete pos;
  if(scope != NULL)
    delete scope;
}

void SNode::error(string msg){
  if(pos != NULL)
    printf("%s [file: %s; line: %d; col: %d]\n", msg.c_str(), pos->filename.c_str(), pos->line, pos->col);
  else
    printf("%s\n", msg.c_str());
  exit(-1);
}

string SNode::to_string(){
  string s = "";
  if(type == NUMBER || type == STREAM_OUT || type == STREAM_IN) {
    char t[1024];
    sprintf(t, "%f", *(val.as_number));
    s = t;
  } else if(type == FUNCTION)
    s = name;
  else if(type == STRING || type == SYMBOL || type == EXCEPTION || type == BOOL)
    s = *(val.as_string);
  else if(type == LIST || type == TC || type == CONS) {
    s = "(";
    list<SNode *>::iterator it;
    for(it=val.as_seq->begin(); it != val.as_seq->end(); ++it)
      s += (*it)->to_string() + " ";
    s += ")";
  } else if(type == VECTOR) {
    s = "[";
    vector<SNode *>::iterator it;
    int i = 0;
    int max = 10;
    for(it=val.as_vector->begin(); it != val.as_vector->end() && i < max; ++it) {
      if((*it) != NULL)
        s += (*it)->to_string() + " ";
      i++;
    }
    if(i == max)
      s += " ... etc";
    s += "]";
  }
  return s;
}

void SNode::show(){
  printf("%s\n", to_string().c_str());
}

string SNode::type_string(){
  if(type == SYMBOL)
    return "symbol";
  if(type == NUMBER)
    return "number";
  if(type == STRING)
    return "string";
  if(type == BOOL)
    return "bool";
  if(type == LIST)
    return "list";
  if(type == CONS)
    return "cons";
  if(type == VECTOR)
    return "vector";
  if(type == TC)
    return "tc";
  if(type == EXCEPTION)
    return "exception";
  if(type == FUNCTION)
    return "function";
  return "unknown";
}
