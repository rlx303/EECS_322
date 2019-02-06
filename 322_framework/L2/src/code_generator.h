#pragma once

#include <L2.h>

namespace L2{

  void generate_code(Program p);
  void generate_in_out(Function* f);
  std::map<std::string, std::set<std::string>> generate_interference_graph(Function* f);
  void spill(Function* f, std::string var, std::string prefix);

}
