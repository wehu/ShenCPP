#include "s_reader.h"
#include <stdlib.h>

#define FILE_MODE 0
#define STRING_MODE 1

using namespace std;

SReader::SReader(){
  filename = "";
  line = 1;
  col = 1;
  fid = NULL;
  debug = 1;
  mode = FILE_MODE;
}

void SReader::error(string msg){
  printf("ERROR: %s [file: %s; line: %d; col: %d]\n", msg.c_str(), filename.c_str(), line, col);
  exit(-1);
}

char SReader::peek_char(int l){
  char c;
  int i;
  int r;
  for(i=0; i<l; i++){
    if(mode == FILE_MODE)
      c = fgetc(fid);
    else {
      if(pos >= endp)
        c = EOF;
      else 
        c = filename.at(pos);
    };
  }
  if(c != EOF && mode == FILE_MODE)
    r = fseek(fid, 0-l, 1);
  return c;
}

char SReader::get_char(){
  char c;
  if(mode == FILE_MODE)
    c = fgetc(fid);
  else {
    if(pos >= endp)
      c = EOF;
    else {
      c = filename.at(pos);
      pos ++;
    };
  }
  return c;
}

void SReader::consume(){
  char c = get_char();
  if(c == '\n'){
    line = line + 1;
    col = 1;
  } else {
    col = col + 1;
  }
}

void SReader::skip_blankspace(){
  char c = peek_char();
  while(c != EOF) {
    if(c == ' ' || c == '\t' || c == '\n')
      consume();
    else
      break;
    c = peek_char();
  }
}

void SReader::record_pos(SNode * n) {
  if(debug) {
    if(n->pos == NULL)
      n->pos = new s_node_pos;
    n->pos->filename = filename;
    n->pos->line = line;
    n->pos->col  = col;
  }
}

SNode * SReader::read(string file, int m){
  char c;
  filename = file;
  mode = m;
  line = 1;
  col = 1;
  pos = 0;
  endp = 0;
  SNode * ast = new SNode(LIST);
  record_pos(ast);
  if(mode == FILE_MODE) {
    fid = fopen(file.c_str(), "r");
    if(fid == NULL) {
      error(string("cannot open file ") + file);
    }
  } else {
    endp = filename.length();
  };
  skip_blankspace();
  c = peek_char();
  while(c != EOF) {
    ast->val.as_seq->push_back(lex());
    skip_blankspace();
    c = peek_char();
  }
  return ast;
}

SNode * SReader::read_file(string file) {
  return read(file, FILE_MODE);
}

SNode * SReader::read_string(string in) {
  return read(in, STRING_MODE);
}

SNode * SReader::lex(){
  char c;
  skip_blankspace();
  c = peek_char();
  if(c == '(')
    return alist();
  else if(c == ')')
    error("unmatched )");
  else if(c == '\"')
    return astring();
  else if((c >= 48 && c <= 57) ||
          ((c == '+' || c == '-') &&
           peek_char(2) >= 48 && peek_char(2) <= 57))
    return anumber();
  else
    return asymbol();
}

SNode * SReader::alist(){
  SNode * l;
  char c;
  consume();
  skip_blankspace();
  l = new SNode(LIST);
  record_pos(l);
  c = peek_char();
  while(c != EOF && c != ')') {
    l->val.as_seq->push_back(lex());
    skip_blankspace();
    c = peek_char();
  }
  if(c != ')')
    error("expected )");
  consume();
  return l;
}

SNode * SReader::astring(){
  SNode * s;
  char c;
  consume();
  s = new SNode(STRING);
  record_pos(s);
  c = peek_char();
  while(c != EOF && c != '\"') {
    if(c == '\\') {
      consume();
      c = peek_char();
      if(c == 'n')
        *(s->val.as_string) += '\n';
      else if(c == 't')
        *(s->val.as_string) += '\t';
      else if(c == 'b')
        *(s->val.as_string) += '\b';
      else if(c == 'r')
        *(s->val.as_string) += '\r';
      else
        *(s->val.as_string) += c;
      consume();
    } else {
      *(s->val.as_string) += c;
      consume();
    }
    c = peek_char();
  }
  if(c != '\"')
    error("expected \"");
  s->name = *(s->val.as_string);
  consume();
  return s;
}

SNode * SReader::anumber(){
  SNode * n = new SNode(NUMBER);
  char c = peek_char();
  int dot = 0;
  record_pos(n);
  if(c == '+' || c == '-') {
    n->name += c;
    consume();
    c = peek_char();
  };
  while(c != EOF &&
        c != ' ' && c != '\t' && c != '\n' &&
        c != '(' && c != ')' && c != '\"') {
    if(c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' ||
       c == '7' || c == '8' || c == '9' ||
       c == '.') {
      n->name += c;
      if(c == '.')
        dot = 1;
      else if(dot == 1 && c == '.')
        error("invalid double letter . for number");
      consume();
      c = peek_char();
    } else
      error(string("invalid letter ") + c + " for number");
  }
  *(n->val.as_number) = strtod(n->name.c_str(), NULL);
  return n;
}

SNode * SReader::asymbol(){
  SNode * s = new SNode(SYMBOL);
  char c = peek_char();
  record_pos(s);
  while(c != EOF &&
        c != ' ' && c != '\t' && c != '\n' &&
        c != '(' && c != ')' && c != '\"') {
    if(c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f' || c == 'g' ||
       c == 'h' || c == 'i' || c == 'j' || c == 'k' || c == 'l' || c == 'm' || c == 'n' ||
       c == 'o' || c == 'p' || c == 'q' || c == 'r' || c == 's' || c == 't' || c == 'u' ||
       c == 'v' || c == 'w' || c == 'x' || c == 'y' || c == 'z' || 
       c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F' || c == 'G' ||
       c == 'H' || c == 'I' || c == 'J' || c == 'K' || c == 'L' || c == 'M' || c == 'N' ||
       c == 'O' || c == 'P' || c == 'Q' || c == 'R' || c == 'S' || c == 'T' || c == 'U' ||
       c == 'V' || c == 'W' || c == 'X' || c == 'Y' || c == 'Z' ||
       c == '=' || c == '-' || c == '*' || c == '/' || c == '+' || c == '_' || c == '?' ||
       c == '$' || c == '!' || c == '@' || c == '~' || c == '.' || c == '>' || c == '<' ||
       c == '&' || c == '%' || c == '\'' || c == '#' || c == '`' || c == ';' || c == ':' ||
       c == '{' || c == '}' ||
       c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' ||
       c == '7' || c == '8' || c == '9') {
      *(s->val.as_string) += c;
      consume();
      c = peek_char();
    } else
      error(string("invalid letter ") + c + " for symbol");
  }
  s->name = *(s->val.as_string);
  return s;
}


