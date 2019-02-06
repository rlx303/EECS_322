#pragma once

#include <vector>
#include <map>
#include <set>

namespace L2 {

extern std::map<std::string, std::string> reg_map;

  struct Item {
    std::string data;
    Item(std::string input) {data = input;}

    virtual std::string get_L2() { return data; }
    virtual std::string get_x86() { return ""; }
    virtual bool is_int() {return false;}
    virtual bool is_var() {return false;}
    virtual bool is_reg() {return false;}
    virtual bool is_mem() {return false;}
    virtual bool is_arrow() {return false;}
    virtual bool is_label() {return false;}
    virtual bool is_sop() {return false;}
  };

  struct I_num : Item {
    I_num(std::string input) :Item(input) {}
    std::string get_x86() override {
      return "$" + data;
    }

    bool is_int() override {
      return true;
    }
  };

  struct I_label : Item {
    I_label(std::string input) :Item(input) {}
    std::string get_x86() override {
      std::string ret = data;
      return ret.replace(0, 1, "$_");
    }
    bool is_label() override {return true;}
  };

  struct I_reg : Item {
    I_reg(std::string input) :Item(input) {}
    std::string get_x86() override {
      return "%" + data;
    }
    bool is_reg() override{
      return true;
    }
  };

  struct I_var : Item {
    I_var(std::string input) :Item(input) {}
    std::string get_x86() override {
      return data;
    }
    bool is_var() override {
      return true;
    }
  };

  struct I_rcx : Item {
    I_rcx(std::string input) :Item(input) {}
    std::string get_x86() override {
      return "%" + reg_map[data];
    }
  };

  struct I_memxM : Item {
    std::string M;
    I_memxM(std::string input, std::string num) :Item(input) {
      M = num;
    }
    std::string get_L2() override {
      return "mem " + data + " " + M;
    }
    std::string get_x86() override {
      return M + "(%" + data + ")";
    }
    bool is_mem() override {
      return true;
    }
  };

  struct I_arrow : Item {
    I_arrow(std::string input) :Item(input) {}
    std::string get_x86() override {
      return "movq";
    }
    bool is_arrow() override {
      return true;
    }
  };

  struct I_aop : Item {
    I_aop(std::string input) :Item(input) {}
    std::string get_x86() override;
  };

  struct I_sop : Item {
    I_sop(std::string input) :Item(input) {}
    std::string get_x86() override;
    bool is_sop() override {
      return true;
    }
  };

  struct I_cmp : Item {
    I_cmp(std::string input) :Item(input) {}
  };

  struct I_inc_dec : Item {
    I_inc_dec(std::string input) :Item(input) {}
    std::string get_x86() override;
  };

  struct I_stack_arg : Item {
    I_stack_arg(std::string input) :Item(input) {}
    std::string get_L2() override {
      return "stack-arg " + data;
    }
    std::string get_x86() override {
      return "stack-arg " + data; ///////////TBD
    }  
  };




  /*
   * Instruction interface.
   */
  struct Instruction{
    std::set<std::string> in = std::set<std::string>();
    std::set<std::string> out = std::set<std::string>();
    virtual std::string print_L2() {return "";}
    virtual std::string print_x86() {return "";}
    virtual std::set<std::string> get_gen() {return std::set<std::string>();}
    virtual std::set<std::string> get_kill() {return std::set<std::string>();}
  };

  /*
   * Instructions.
   */
  struct Instruction_ret : Instruction{
    int64_t arguments;
    int64_t locals;
    std::string print_L2() override {
      return "return";
    }
    std::string print_x86() override {
      int64_t offset = (arguments<=6) ? locals*8 : (arguments-6)*8+locals*8;
      return "addq $" + std::to_string(offset) + ", %rsp\n" + "retq";
    }
    std::set<std::string> get_gen() override;
  };

  struct Instruction_assign : Instruction{
    Item* src;
    Item* dst;
    Item* op;
    std::string print_L2() override {
      return dst->get_L2() + " " + op->get_L2() + " " + src->get_L2();
    }
    std::string print_x86() override {
      return op->get_x86() + " " + src->get_x86() + ", " + dst->get_x86();
    }
    std::set<std::string> get_gen() override;

    std::set<std::string> get_kill() override;
  };

  struct Instruction_inc_dec: Instruction{
    Item* dst;
    Item* op;
    std::string print_L2() override {
      return dst->get_L2() + op->get_L2();
    }
    std::string print_x86() override {
      return op->get_x86() + " " + dst->get_x86();
    }

    std::set<std::string> get_gen() override;

    std::set<std::string> get_kill() override;
  };

  struct Instruction_label: Instruction{
    Item* label;
    std::string print_L2() override {
      return label->get_L2();
    }
    std::string print_x86() override {
      return label->get_L2().replace(0, 1, "_") + ":";
    } 
  };

  struct Instruction_goto: Instruction{
    Item* label;
    std::string print_L2() override {
      return "goto " + label->get_L2();
    }
    std::string print_x86() override {
      return "jmp " + label->get_L2().replace(0, 1, "_");
    } 
  };

  struct Instruction_cmp: Instruction{
    Item* dst;
    Item* op;
    Item* t1;
    Item* cmp;
    Item* t2;
    std::string print_L2() override {
      return dst->get_L2() + " " + op->get_L2() + " " + 
      t1->get_L2() + " " + cmp->get_L2() + " " + t2->get_L2();
    }
    std::string print_x86() override;
    std::set<std::string> get_kill() override;
    std::set<std::string> get_gen() override;
  };

  struct Instruction_two_label_jump : Instruction {
    Item* t1;
    Item* cmp;
    Item* t2;
    Item* label1;
    Item* label2; 
    std::string print_L2() override {
      return "cjump " + t1->get_L2() + " " + cmp->get_L2() + " " + 
      t2->get_L2() + " " + label1->get_L2() + " " + label2->get_L2();
    }

    std::string print_x86() override;

    std::set<std::string> get_gen() override;
  };


  struct Instruction_one_label_jump : Instruction {
    Item* t1;
    Item* cmp;
    Item* t2;
    Item* label1;
    std::string print_L2() override {
      return "cjump " + t1->get_L2() + " " + cmp->get_L2() + " " + 
      t2->get_L2() + " " + label1->get_L2();
    }

    std::string print_x86() override;

    std::set<std::string> get_gen() override;
  };

  struct Instruction_wwe : Instruction{
    Item* dst;
    Item* w1;
    Item* w2;
    Item* E;
    std::string print_L2() override {
      return dst->get_L2() + " @ " + w1->get_L2() + " " + w2->get_L2() + " " + E->get_L2();
    }
    std::string print_x86() override {
      return "lea (" + w1->get_x86() + ", " + w2->get_x86() + ", " + E->get_L2() + "), " 
      + dst->get_x86();
    }

    std::set<std::string> get_kill() override;
    std::set<std::string> get_gen() override;
  };

  struct Instruction_call : Instruction{
    Item* label;
    Item* arg_num;
    std::string print_L2() override {
      return "call " + label->get_L2() + " " + arg_num->get_L2();
    }
    std::string print_x86() override;

    std::set<std::string> get_gen() override;

    std::set<std::string> get_kill() override;
  };


  struct Instruction_runtime : Instruction{
    std::string name;
    std::string arg_num;
    std::string print_L2() override {
      return "call " + name + " " + arg_num;
    }
    std::string print_x86() override {
      std::string f = (name == "array-error") ? "array_error" : name;
      return "call " + f;
    }

    std::set<std::string> get_gen() override;

    std::set<std::string> get_kill() override;
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

  struct Spill_function : Function{
    std::string var;
    std::string prefix;
  };

  /*
   * Program.
   */
  struct Program{
    std::string entryPointLabel;
    std::vector<Function *> functions;
  };

}
