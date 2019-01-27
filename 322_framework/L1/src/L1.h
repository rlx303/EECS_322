#pragma once

#include <vector>

namespace L1 {

  enum class ItemType {Label, Number, Register};

  struct Item {
    std::string data;
    // ItemType type;
    Item(std::string input) {data = input;}

    virtual std::string get_L1() { return data; }
    virtual std::string get_x86() { return "";};
  };

  struct I_num : Item {
    I_num(std::string input) :Item(input) {}
    std::string get_x86() override {
      return "$" + data;
    }
  };

  struct I_label : Item {
    I_label(std::string input) :Item(input) {}
    std::string get_x86() override {
      std::string ret = data;
      return ret.replace(0, 1, "$_");
    }
  };

  struct I_reg : Item {
    I_reg(std::string input) :Item(input) {}
    std::string get_x86() override {
      return "%" + data;
    }
  };

  struct I_memxM : Item {
    std::string M;
    I_memxM(std::string input, std::string num) :Item(input) {
      M = num;
    }
    std::string get_L1() override {
      return "mem " + data + " " + M;
    }
    std::string get_x86() override {
      return M + "(%" + data + ")";
    }
  };

  struct I_arrow : Item {
    I_arrow(std::string input) :Item(input) {}
    std::string get_x86() override {
      return "movq";
    }
  };

  struct I_aop : Item {
    I_aop(std::string input) :Item(input) {}
    std::string get_x86() override {
      if (data == "+=") {
        return "addq";
      } 
      else if (data == "-=") {
        return "subq";
      }
      else if (data == "*=") {
        return "imulq";
      }
      else {
        return "andq";
      }
    }
  };

  struct I_sop : Item {
    I_sop(std::string input) :Item(input) {}
    std::string get_x86() override {
      if (data == "<<=") {
        return "salq";
      }
      else {
        return "sarq";
      } 
    }
  };

  struct I_cmp : Item {
    I_cmp(std::string input) :Item(input) {}
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

  struct Instruction_s2w_assign : Instruction{
    Item* src;
    Item* dst;
    Item* op;
  };

  /*
   * Function.
   */
  struct Function{
    std::string name;
    int64_t arguments;
    int64_t locals;
    std::vector<Instruction *> instructions;
  };

  /*
   * Program.
   */
  struct Program{
    std::string entryPointLabel;
    std::vector<Function *> functions;
  };

}
