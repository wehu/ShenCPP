#ifndef __S_FRAME_H__
#define __S_FRAME_H__

#include "s_node.h"
#include <string>
#include <map>

class SNode;

class SFrame {

  std::map <std::string, SNode*> params;

  public:

  SFrame();

  void add_param(std::string n, SNode * v);

  int param_exists(std::string n);

  SNode * read_param(std::string n);
  
  void copy(SFrame * o);

  void show();
};

#endif
