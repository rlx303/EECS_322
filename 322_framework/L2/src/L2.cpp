#include <vector>
#include <map>
#include <set>
#include <L2.h>

namespace L2 {
	std::map<std::string, std::string> reg_map = {
    	{"r8", "r8b"},
    	{"r9", "r9b"},
    	{"r10", "r10b"},
    	{"r11", "r11b"},
    	{"r12", "r12b"},
    	{"r13", "r13b"},
    	{"r14", "r14b"},
    	{"r15", "r15b"},
    	{"rax", "al"},
    	{"rbp", "bpl"},
    	{"rbx", "bl"},
	    {"rcx", "cl"},
	    {"rdi", "dil"},
	    {"rdx", "dl"},
	    {"rsi", "sil"},
  	};

  	std::vector<std::string> caller_saved = {
  		"r10", "r11", "r8", "r9", "rax", "rcx", "rdi", "rdx", "rsi", "rax"
  	};
  	std::vector<std::string> callee_saved = {
  		"rdi", "rsi", "rdx", "rcx", "r8", "r9"
  	};

  	std::string I_aop::get_x86()  {
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

    std::string I_sop::get_x86()  {
      if (data == "<<=") {
        return "salq";
      }
      else {
        return "sarq";
      } 
    }

    std::string I_inc_dec::get_x86() {
      if (data == "++") {
        return "inc";
      } 
      else {
        return "dec";
      }
    }

    std::string Instruction_cmp::print_x86() {
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

    std::string Instruction_two_label_jump::print_x86() {
      std::string cmp_sign = cmp->get_L2();
      std::string label1_x86 = label1->get_L2().replace(0, 1, "_");
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
        jmp + " " + label1_x86 + "\n" + 
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
        jmp + " " + label1_x86 + "\n" + 
        "jmp " + label2_x86;
        } 
        else { //t2 is also number
          int t1_val = stoi(t1->get_L2());
          int t2_val = stoi(t2->get_L2());
          std::string jmp_1 = "jmp " + label1_x86;
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


    std::string Instruction_one_label_jump::print_x86() {
      std::string cmp_sign = cmp->get_L2();
      std::string label1_x86 = label1->get_L2().replace(0, 1, "_");
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
        jmp + " " + label1_x86;
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
        jmp + " " + label1_x86;
        } 
        else { //t2 is also number
          int t1_val = stoi(t1->get_L2());
          int t2_val = stoi(t2->get_L2());
          std::string jmp_1 = "jmp " + label1_x86;
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

    std::string Instruction_call::print_x86() {
      std::string l = label->get_L2();
      std::string f = (l.at(0)==':') ? l.replace(0, 1, "_") : "*%"+l;
      int n = stoi(arg_num->get_L2());
      std::string offset = (n<=6) ? "8" : std::to_string((n-5)*8);
      return "subq $" + offset + ", " + "%rsp" + "\n" +
      "jmp " + f;
    }




    std::set<std::string> Instruction_one_label_jump::get_gen() {
      std::set<std::string> gen;
      std::string t1_string = t1->get_L2();
      std::string t2_string = t2->get_L2();
      if (t1->is_reg()) {
        if (t1_string != "rsp") {
          gen.insert(t1_string);
        }
      }
      else if (t1->is_var()) {
        gen.insert(t1_string);
      }

      if (t2->is_reg()) {
        if (t2_string != "rsp") {
          gen.insert(t2_string);
        }
      }
      else if (t2->is_var()) {
        gen.insert(t2_string);
      }
      return gen;
    }

    std::set<std::string> Instruction_two_label_jump::get_gen() {
      std::set<std::string> gen;
      std::string t1_string = t1->get_L2();
      std::string t2_string = t2->get_L2();
      if (t1->is_reg()) {
        if (t1_string != "rsp") {
          gen.insert(t1_string);
        }
      }
      else if (t1->is_var()) {
        gen.insert(t1_string);
      }

      if (t2->is_reg()) {
        if (t2_string != "rsp") {
          gen.insert(t2_string);
        }
      }
      else if (t2->is_var()) {
        gen.insert(t2_string);
      }
      return gen;
    }

    std::set<std::string> Instruction_cmp::get_gen() {
      std::set<std::string> gen;
      std::string t1_string = t1->get_L2();
      std::string t2_string = t2->get_L2();
      if (t1->is_reg()) {
        if (t1_string != "rsp") {
          gen.insert(t1_string);
        }
      }
      else if (t1->is_var()) {
        gen.insert(t1_string);
      }

      if (t2->is_reg()) {
        if (t2_string != "rsp") {
          gen.insert(t2_string);
        }
      }
      else if (t2->is_var()) {
        gen.insert(t2_string);
      }
      return gen;
    }

    std::set<std::string> Instruction_cmp::get_kill() { //
      std::set<std::string> kill;
      std::string dst_string = dst->get_L2();
      if (dst->is_reg() || dst->is_var()) {
        kill.insert(dst_string);
      }
      return kill;
    }

    std::set<std::string> Instruction_ret::get_gen() {
      std::set<std::string> gen;
      gen.insert("rax");
      gen.insert("r12");
      gen.insert("r13");
      gen.insert("r14");
      gen.insert("r15");
      gen.insert("rbp");
      gen.insert("rbx");
      return gen;
  	}

    std::set<std::string> Instruction_assign::get_gen() {
      std::set<std::string> gen;
      std::string src_string = src->get_L2();
      if (src->is_reg()) {
        if (src_string != "rsp") {
          gen.insert(src_string);
        }
      }
      else if (src->is_var()) {
        gen.insert(src_string);
      }
      else if (src->is_mem()) {
        if (src->data != "rsp") {
          gen.insert(src->data);
        }
      }

      if (dst->is_mem()) {
        if (dst->data != "rsp") {
          gen.insert(dst->data);
        }      
      }
      else if (!op->is_arrow()) {
	      if (dst->is_reg() || dst->is_var()) {
	         gen.insert(dst->data);
	      }
      }
      return gen;
    }

    std::set<std::string> Instruction_assign::get_kill() {
      std::set<std::string> kill;
      std::string dst_string = dst->get_L2();
      if (dst->is_reg() || dst->is_var()) {
        kill.insert(dst_string);
      }
      return kill;
    }

    std::set<std::string> Instruction_inc_dec::get_gen() {
      std::set<std::string> gen;
      std::string dst_string = dst->get_L2();
      gen.insert(dst_string);
      return gen;
    }

    std::set<std::string> Instruction_inc_dec::get_kill() {
      std::set<std::string> kill;
      std::string dst_string = dst->get_L2();
      kill.insert(dst_string);
      return kill;
    }

    std::set<std::string> Instruction_wwe::get_kill() {
      std::set<std::string> kill;
      std::string dst_string = dst->get_L2();
      if (dst->is_reg() || dst->is_var()) {
        kill.insert(dst_string);
      }
      return kill;
    }

    std::set<std::string> Instruction_wwe::get_gen() {
      std::set<std::string> gen;
      std::string w1_string = w1->get_L2();
      std::string w2_string = w2->get_L2();
      if (w1->is_reg()) {
        if (w1_string != "rsp") {
          gen.insert(w1_string);
        }
      }
      else if (w1->is_var()) {
        gen.insert(w1_string);
      }

      if (w2->is_reg()) {
        if (w2_string != "rsp") {
          gen.insert(w2_string);
        }
      }
      else if (w2->is_var()) {
        gen.insert(w2_string);
      }
      return gen;
    }

    std::set<std::string> Instruction_call::get_gen() {
      std::set<std::string> gen = std::set<std::string>(callee_saved.begin(), callee_saved.begin()+std::min(6, stoi(arg_num->get_L2())));
      if (label->is_reg() || label->is_var()) {
        gen.insert(label->get_L2());
      }
      return gen;
    }

    std::set<std::string> Instruction_call::get_kill() {
      std::set<std::string> kill = std::set<std::string>(caller_saved.begin(), caller_saved.end());
      return kill;
    }

    std::set<std::string> Instruction_runtime::get_gen() {
      std::set<std::string> gen = std::set<std::string>(callee_saved.begin(), callee_saved.begin()+std::min(6, stoi(arg_num)));
      return gen;
    }

    std::set<std::string> Instruction_runtime::get_kill() {
      std::set<std::string> kill = std::set<std::string>(caller_saved.begin(), caller_saved.end());
      return kill;
    }

}