#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <map>
#include <iterator>

#include <code_generator.h>
#include <utils.h>

namespace L2{

  void generate_code(Program p){

    /* 
     * Open the output file.
     */ 
    std::ofstream of;
    of.open("prog.S");
    
    std::string tab = "    ";
    of << ".text\n";
    of << ".globl go\n";
    of << "go:\n";
    of << "# save callee-saved registers\n";
    of << "pushq %rbx\n";
    of << "pushq %rbp\n";
    of << "pushq %r12\n";
    of << "pushq %r13\n";
    of << "pushq %r14\n";
    of << "pushq %r15\n\n";

    of << "call " << p.entryPointLabel.replace(0, 1, "_") << '\n';

    of <<"# restore callee-saved registers and return\n";
    of << "popq %r15\n";
    of << "popq %r14\n";
    of << "popq %r13\n";
    of << "popq %r12\n";
    of << "popq %rbp\n";
    of << "popq %rbx\n";
    of << "retq\n";

    for (const auto& f : p.functions){    
        of << f->name.replace(0, 1, "_") << ":\n";
        of << "subq $" << f->locals*8 << ", %rsp #Allocate locals\n";
        for (const auto& i : f->instructions) {
            of << i->print_L2() << "\n";
        }
    }

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

  std::map<std::string, std::set<std::string>> generate_interference_graph(Function* f){
    std::map<std::string, std::set<std::string>> ig;

    //Initialize with all GP regs
    for (const auto &pair1 : reg_map) {
        for (const auto &pair2 : reg_map) {
            ig_insert(ig, pair1.first, pair2.first);
        }
    }

    for (const auto &i : f->instructions) {
        ig_connect(ig, i->in, i->in);
        ig_connect(ig, i->out, i->out);
        if (!skip_kill_out(i)) {
            ig_connect(ig, i->get_kill(), i->out);
        } else {
            ig_init_set(ig, i->get_kill());
        }
        sop_constraint(i, ig);
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
}