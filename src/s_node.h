#ifndef __S_NODE_H__
#define __S_NODE_H__
#include <string>
#include <list>
#include <vector>

#include "s_frame.h"

class SFrame;

union s_node_u;
class SNode;

typedef union s_node_u{
  std::string * as_string;
  double * as_number;
  std::list <SNode *> * as_seq;
  std::vector <SNode *> * as_vector;
} s_node;

typedef enum {
  SYMBOL,
  NUMBER,
  STRING,
  BOOL,
  LIST,
  CONS,
  VECTOR,
  TC,
  EXCEPTION,
  FUNCTION,
  STREAM_OUT,
  STREAM_IN 
} s_node_type;

typedef struct {
  std::string filename;
  size_t line;
  size_t col;
} s_node_pos;

class SNode {

  public:

  s_node_type type;
  std::string name;
  s_node val;
  s_node_pos * pos;

  SFrame * scope;

  SNode(s_node_type t);
  ~SNode();

  void error(std::string msg="");

  std::string to_string();
  void show();

  std::string type_string();
};

#endif
