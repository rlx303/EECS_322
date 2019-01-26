#pragma once

#include <vector>

namespace L1 {

  struct Item {
    std::string labelName;
  };

  /*
   * Instruction interface.
   */
  struct Instruction{
  };

  /*
   * Instructions.
   */
  struct Instruction_ret : Instruction{
  };

  struct Instruction_assign : Instruction{
    std::string dst;
    std::string src;
    std::string op;
  };

  /*
   * Function.
   */
  struct Function{
    std::string name;
    int64_t arguments;
    int64_t locals;
    std::vector<Instruction *> instructions;
    std::string w;
    std::string s;
    std::string memxM;
    std::string op;
    std::string t;
    std::string sx;
    std::string N;
    std::string dst;
    std::string src;
  };

  /*
   * Program.
   */
  struct Program{
    std::string entryPointLabel;
    std::vector<Function *> functions;
  };

}
