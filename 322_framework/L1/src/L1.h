#pragma once

#include <vector>
#include <map>

namespace L1 {

extern std::map<std::string, std::string> reg_map;

  // reg_map["r8"] = "r8b";
  // reg_map["r9"] = "r9b";
  // reg_map["r10"] = "r10b";
  // reg_map["r11"] = "r11b";
  // reg_map["r12"] = "r12b";
  // reg_map["r13"] = "r13b";
  // reg_map["r14"] = "r14b";
  // reg_map["r15"] = "r15b";
  // reg_map["rax"] = "al";
  // reg_map["rbp"] = "bpl";
  // reg_map["rbx"] = "bl";
  // reg_map["rcx"] = "cl";
  // reg_map["rdi"] = "dil";
  // reg_map["rdx"] = "dl";
  // reg_map["rsi"] = "sil";

  struct Item {
    std::string data;
    Item(std::string input) {data = input;}

    virtual std::string get_L1() { return data; }
    virtual std::string get_x86() { return ""; }
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
    virtual std::string print_L1() {return "";}
    virtual std::string print_x86() {return "";}
  };

  /*
   * Instructions.
   */
  struct Instruction_ret : Instruction{
    std::string print_L1() override {
      return "return";
    }
    std::string print_x86() override {
      return "retq";
    }
  };

  struct Instruction_assign : Instruction{
    Item* src;
    Item* dst;
    Item* op;
    std::string print_L1() override {
      return dst->get_L1() + " " + op->get_L1() + " " + src->get_L1();
    }
    std::string print_x86() override {
      return op->get_x86() + " " + src->get_x86() + ", " + dst->get_x86();
    }
  };

  struct Instruction_inc_dec: Instruction{
    Item* dst;
    Item* op;
    std::string print_L1() override {
      return dst->get_L1() + op->get_L1();
    }
    std::string print_x86() override {
      return op->get_x86() + " " + dst->get_x86();
    }
  };

  struct Instruction_label: Instruction{
    Item* label;
    std::string print_L1() override {
      return label->get_L1();
    }
    std::string print_x86() override {
      return label->get_L1().replace(0, 1, "_") + ":";
    } 
  };

  struct Instruction_goto: Instruction{
    Item* label;
    std::string print_L1() override {
      return "goto " + label->get_L1();
    }
    std::string print_x86() override {
      return "jmp " + label->get_L1().replace(0, 1, "_");
    } 
  };

  struct Instruction_cmp: Instruction{
    Item* dst;
    Item* op;
    Item* t1;
    Item* cmp;
    Item* t2;
    std::string print_L1() override {
      return dst->get_L1() + " " + op->get_L1() + " " + 
      t1->get_L1() + " " + cmp->get_L1() + " " + t2->get_L1();
    }
    std::string print_x86() override {
      char *t1_string = const_cast<char*>(t1->get_L1().c_str());
      char *t2_string = const_cast<char*>(t2->get_L1().c_str());
      char* p1;
      long int t1_val = strtol(t1_string, &p1, 10);
      std::string reg = "%" + reg_map[dst->get_L1()];
      std::string cmp_sign = cmp->get_L1();
      if (*p1) { //t1 is not number
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
        char* p2;
        long int t2_val = strtol(t2_string, &p2, 10);
        if (*p2) { //t2 is not number
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
