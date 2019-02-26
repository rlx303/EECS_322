#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <stack>
#include <string>

#include <code_generator.h>
#include <utils.h>
#include <ig.h>

namespace L2{

  void generate_code(Program p){

    /* 
     * Open the output file.
     */ 
    std::ofstream of;
    of.open("prog.L1");

    of << '(' << p.entryPointLabel << '\n';

    for (auto &f : p.functions) {
      std::set<L2::Node> spilled_vars;
      L2::IG ig;
      do {
        L2::clear_in_out(f);
        L2::generate_in_out(f);
        ig = L2::generate_interference_graph(f);
        spilled_vars = L2::color_graph(f, ig);
        if (!spilled_vars.empty()) {
          std::string suffix = ig.longest_var_name();
          for (auto &node : spilled_vars) {
            L2::spill(f, node.get_item(), node.get_item() + suffix.erase(0,1));
          }
        }
        else {
            break;
        }
        //f->print();
        //std::cout << "4" << std::endl;
        L2::clear_in_out(f);
        L2::generate_in_out(f);
        //std::cout << "5" << std::endl;
        ig = L2::generate_interference_graph(f);
        //std::cout << "6" << std::endl;
        spilled_vars = L2::color_graph(f, ig);
        //std::cout << "7" << std::endl;
      }while(!spilled_vars.empty());

      of << '(' << f->name << '\n';

      of << f->arguments << ' ' << f->locals << '\n';

      auto var_map = ig.get_var_map();
      for (auto& i : f->instructions) {
        i = L2::ins_replace_var_with_reg(i, var_map);
        if (auto a = dynamic_cast<Instruction_assign*>(i)) {
          if (auto s = dynamic_cast<I_stack_arg*>(a->src)) {
            s->get_locals(f->locals);
          }
        }
        of << i->print_L2() << '\n';
      }

      of << ')' << '\n';

    }

    of << ')' << '\n';

    /* 
     * Close the output file.
     */ 
    of.close();
   
    return ;
  }

  void generate_in_out(Function* f){
    bool changed = false;
    do{
        changed = false;
        for (auto it = f->instructions.begin(); it != f->instructions.end(); ++it) {
            std::set<std::string> new_in = set_u((*it)->get_gen(), set_diff((*it)->out, (*it)->get_kill()));
            if (new_in != (*it)->in) {
                changed = true;
                (*it)->in = new_in;
            }
            std::set<std::string> new_out;
            std::vector<Instruction*> succs = find_successors(it, f->instructions.end(), f->instructions);
            for (auto suc : succs) {
                std::set<std::string> merged = set_u(new_out, suc->in);
                new_out = merged;
            }
            if (new_out != (*it)->out) {
                changed = true;
                (*it)->out = new_out;
            }
        }
    }while(changed);
  }

  void clear_in_out(Function* f){
    for (auto &i : f->instructions) {
        i->in = std::set<std::string>();
        i->out = std::set<std::string>();
    }
  }

  IG generate_interference_graph(Function* f){
    IG ig = IG();
 
    //Initialize with all GP regs
    for (const auto &pair1 : reg_map) {
        for (const auto &pair2 : reg_map) {
            ig.insert(pair1.first, pair2.first);
        }
    }

    for (const auto &i : f->instructions) {
        ig.connect(i->in, i->in);
        ig.connect(i->out, i->out);
        ig.connect(i->get_kill(), i->out);
        ig.sop_constraint(i);
    }
    return ig;
  }

  void spill(Function* f, std::string var, std::string prefix) {
    std::vector<Instruction *> new_instructions = std::vector<Instruction *>();
    int count = 0;
    int offset = 8 * f->locals;
    bool found_var = false;
    for (auto i : f->instructions) {
        if (ins_contains(i, var)) {
            found_var = true;
            std::string spilled_var = prefix + std::to_string(count);
            if (i->get_gen().count(var) > 0) {
                auto gg = read_mem_ins(spilled_var, offset);
                new_instructions.push_back(gg);
            }
            new_instructions.push_back(ins_replace(i, var, spilled_var));
            if (i->get_kill().count(spilled_var) > 0) {
                new_instructions.push_back(write_mem_ins(spilled_var, offset));
            }
            count++;
        }
        else {
            new_instructions.push_back(i);
        }
    }
    f->instructions = new_instructions;
    if (found_var) {
        f->locals++;
    }
    return;
  }
  
  std::set<Node> color_graph(Function* f, IG &ig) {
    std::stack<Node> nodes;
    while(!ig.empty()) {
        nodes.push(ig.remove_next());
    }
    return ig.add_colors_to_nodes(nodes);
  } 
}