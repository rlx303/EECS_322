#pragma once

#include <vector>
#include <map>

namespace L2 {

extern std::map<std::string, std::string> reg_map;

  struct Item {
    std::string data;
    Item(std::string input) {data = input;}

    virtual std::string get_L2() { return data; }
    virtual std::string get_x86() { return ""; }
    virtual bool is_int() {return false;}
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
  };

  struct I_reg : Item {
    I_reg(std::string input) :Item(input) {}
    std::string get_x86() override {
      return "%" + data;
    }
  };

  struct I_sx : Item {
    I_sx(std::string input) :Item(input) {}
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

  struct I_inc_dec : Item {
    I_inc_dec(std::string input) :Item(input) {}
    std::string get_x86() override {
      if (data == "++") {
        return "inc";
      } 
      else {
        return "dec";
      }
    }
  };

  /*
   * Instruction interface.
   */
  struct Instruction{
    virtual std::string print_L2() {return "";}
    virtual std::string print_x86() {return "";}
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
    std::string print_x86() override {
      std::string reg = "%" + reg_map[dst->get_L2()];
      std::string cmp_sign = cmp->get_L2();
      if (!t1->is_int()) { //t1 is not number
        std::string set;
        if (cmp_sign=="<") {
          set = "setl";
        } else if (cmp_sign=="<=") {
          set = "setle";
        } else {
          set = "sete";
        }
        return "cmpq " + t2->get_x86() + ", " + t1->get_x86() + "\n" + 
        set + " " + reg + "\n" + "movzbq " + reg + ", " + dst->get_x86();
      } else { //t1 is number
        if (!t2->is_int()) { //t2 is not number
          std::string set;
          if (cmp_sign=="<") {
            set = "setg";
          } else if (cmp_sign=="<=") {
            set = "setge";
          } else {
            set = "sete";
          }
          return "cmpq " + t1->get_x86() + ", " + t2->get_x86() + "\n" + 
          set + " " + reg + "\n" + "movzbq " + reg + ", " + dst->get_x86();
        } else { //t2 is also number
            int t1_val = stoi(t1->get_L2());
            int t2_val = stoi(t2->get_L2());
            std::string move_1 = "movq $1, " + dst->get_x86();
            std::string move_0 = "movq $0, " + dst->get_x86();
            if (cmp_sign=="<") {
              return (t1_val < t2_val) ? move_1 : move_0;
            } else if (cmp_sign=="<=") {
              return (t1_val <= t2_val) ? move_1 : move_0;
            } else {
              return (t1_val == t2_val) ? move_1 : move_0;
            }
          }
      }
      return "";
    }
  };

  struct Instruction_two_label_jump : Instruction {
    Item* t1;
    Item* cmp;
    Item* t2;
    Item* labeL2;
    Item* label2; 
    std::string print_L2() override {
      return "cjump " + t1->get_L2() + " " + cmp->get_L2() + " " + 
      t2->get_L2() + " " + labeL2->get_L2() + " " + label2->get_L2();
    }

    std::string print_x86() override {
      std::string cmp_sign = cmp->get_L2();
      std::string labeL2_x86 = labeL2->get_L2().replace(0, 1, "_");
      std::string label2_x86 = label2->get_L2().replace(0, 1, "_");
      if (!t1->is_int()) { //t1 is not number
        std::string jmp;
        if (cmp_sign=="<") {
          jmp = "jl";
        } else if (cmp_sign=="<=") {
          jmp = "jle";
        } else {
          jmp = "je";
        }
        return "cmpq " + t2->get_x86() + ", " + t1->get_x86() + "\n" + 
        jmp + " " + labeL2_x86 + "\n" + 
        "jmp " + label2_x86;
      } 
      else { //t1 is number
        if (!t2->is_int()) { //t2 is not number
          std::string jmp;
          if (cmp_sign=="<") {
            jmp = "jg";
          } else if (cmp_sign=="<=") {
            jmp = "jge";
          } else {
            jmp = "je";
          }
        return "cmpq " + t1->get_x86() + ", " + t2->get_x86() + "\n" + 
        jmp + " " + labeL2_x86 + "\n" + 
        "jmp " + label2_x86;
        } 
        else { //t2 is also number
          int t1_val = stoi(t1->get_L2());
          int t2_val = stoi(t2->get_L2());
          std::string jmp_1 = "jmp " + labeL2_x86;
          std::string jmp_2 = "jmp " + label2_x86;
          if (cmp_sign=="<") {
            return (t1_val < t2_val) ? jmp_1 : jmp_2;
          } else if (cmp_sign=="<=") {
            return (t1_val <= t2_val) ? jmp_1 : jmp_2;
          } else {
            return (t1_val == t2_val) ? jmp_1 : jmp_2;
          }
        }
      }
    }
  };


  struct Instruction_one_label_jump : Instruction {
    Item* t1;
    Item* cmp;
    Item* t2;
    Item* labeL2;
    std::string print_L2() override {
      return "cjump " + t1->get_L2() + " " + cmp->get_L2() + " " + 
      t2->get_L2() + " " + labeL2->get_L2();
    }

    std::string print_x86() override {
      std::string cmp_sign = cmp->get_L2();
      std::string labeL2_x86 = labeL2->get_L2().replace(0, 1, "_");
      if (!t1->is_int()) { //t1 is not number
        std::string jmp;
        if (cmp_sign=="<") {
          jmp = "jl";
        } else if (cmp_sign=="<=") {
          jmp = "jle";
        } else {
          jmp = "je";
        }
        return "cmpq " + t2->get_x86() + ", " + t1->get_x86() + "\n" + 
        jmp + " " + labeL2_x86;
      } 
      else { //t1 is number
        if (!t2->is_int()) { //t2 is not number
          std::string jmp;
          if (cmp_sign=="<") {
            jmp = "jg";
          } else if (cmp_sign=="<=") {
            jmp = "jge";
          } else {
            jmp = "je";
          }
        return "cmpq " + t1->get_x86() + ", " + t2->get_x86() + "\n" + 
        jmp + " " + labeL2_x86;
        } 
        else { //t2 is also number
          int t1_val = stoi(t1->get_L2());
          int t2_val = stoi(t2->get_L2());
          std::string jmp_1 = "jmp " + labeL2_x86;
          if (cmp_sign=="<") {
            return (t1_val < t2_val) ? jmp_1 : "";
          } else if (cmp_sign=="<=") {
            return (t1_val <= t2_val) ? jmp_1 : "";
          } else {
            return (t1_val == t2_val) ? jmp_1 : "";
          }
        }
      }
    }  
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
  };

  struct Instruction_call : Instruction{
    Item* label;
    Item* arg_num;
    std::string print_L2() override {
      return "call " + label->get_L2() + " " + arg_num->get_L2();
    }
    std::string print_x86() override {
      std::string l = label->get_L2();
      std::string f = (l.at(0)==':') ? l.replace(0, 1, "_") : "*%"+l;
      int n = stoi(arg_num->get_L2());
      std::string offset = (n<=6) ? "8" : std::to_string((n-5)*8);
      return "subq $" + offset + ", " + "%rsp" + "\n" +
      "jmp " + f;
    }
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
