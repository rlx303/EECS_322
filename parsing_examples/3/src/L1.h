#pragma once

#include <vector>

namespace L1 {

  struct Item {
    std::string labelName;
  };

  struct Function{
    std::string name;
    int64_t arguments;
    int64_t locals;
  };

  struct Program{
    std::string entryPointLabel;
    std::vector<Function *> functions;
  };

}
