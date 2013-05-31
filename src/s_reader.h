#ifndef __S_READER_H__
#define __S_READER_H__

#include "s_node.h"
#include <string>
#include <stdio.h>

class SReader {

  public:

  int mode;

  FILE * fid;

  std::string filename;
  size_t line;
  size_t col;
  size_t pos;
  size_t endp;

  int debug;

  SReader();

  void error(std::string msg="");
  void record_pos(SNode * n);

  char peek_char(int l=1);
  char get_char();
  void consume();
  void skip_blankspace();

  SNode * read(std::string f_or_s, int mode);
  SNode * read_file(std::string file);
  SNode * read_string(std::string input);

  SNode * lex();

  SNode * alist();
  SNode * astring();
  SNode * anumber();
  SNode * asymbol();

};

#endif
