#include "s_evaler.h"
#include <string>

using namespace std;

int main(){
  SEvaler p;
  string cf = __FILE__;
  string path = cf.substr(0, cf.length()-8);
  string shenpath = path + "../shen/release/k_lambda/";
  p.read_string(
  "(set *language* \"c++\")"
  "(set *implementation* \"g++-4.2.2\")"
  "(set *release* 0.1)"
  "(set *port* 0.1)"
  "(set *porters* \"Wei Hu\")"
  "(set *home-directory* \".\")"
  "(set *stinput* 0)"
  "(set *stoutput* 1)");
  p.load_file(shenpath + "toplevel.kl");
  p.load_file(shenpath + "core.kl");
  p.load_file(shenpath + "sys.kl");
  p.load_file(shenpath + "sequent.kl");
  p.load_file(shenpath + "yacc.kl");
  p.load_file(shenpath + "reader.kl");
  p.load_file(shenpath + "prolog.kl");
  p.load_file(shenpath + "track.kl");
  p.load_file(shenpath + "load.kl");
  p.load_file(shenpath + "writer.kl");
  p.load_file(shenpath + "macros.kl");
  p.load_file(shenpath + "declarations.kl");
  p.load_file(shenpath + "types.kl");
  p.load_file(shenpath + "t-star.kl");
  p.read_string("(shen.shen)");
  return 0;
}
