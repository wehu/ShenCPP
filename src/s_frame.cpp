#include "s_frame.h"

using namespace std;

SFrame::SFrame(){
  params = map<string, SNode *>();
}

void SFrame::add_param(string n, SNode *v){
  if(param_exists(n))
    params.erase(n);
  params.insert(make_pair(n, v));
}

int SFrame::param_exists(string n){
  if(params.find(n) != params.end())
    return 1;
  else
    return 0;
}

SNode * SFrame::read_param(string n){
  if(param_exists(n))
    return params[n];
}

void SFrame::copy(SFrame *o){
  map<string, SNode *>::iterator it;
  for(it=o->params.begin(); it!=o->params.end(); it++) {
    if(param_exists(it->first))
      params.erase(it->first);
    params.insert((*it));
  };
}

void SFrame::show(){
  map<string, SNode*>::iterator it;
  for(it=params.begin(); it != params.end(); it++) {
    printf("%s : ", it->first.c_str());
    it->second->show();
  };
} 
