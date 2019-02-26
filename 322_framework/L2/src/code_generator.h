#pragma once

#include <L2.h>
#include <ig.h>

namespace L2{

  void generate_code(Program p);
  void generate_in_out(Function* f);
  IG generate_interference_graph(Function* f);
  void spill(Function* f, std::string var, std::string prefix);
  std::set<Node> color_graph(Function* f, IG &ig);
  void clear_in_out(Function* f);
}
