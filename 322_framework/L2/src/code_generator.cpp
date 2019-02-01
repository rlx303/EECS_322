#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <iterator>

#include <code_generator.h>

using namespace std;

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


  std::set<std::string> set_diff(std::set<std::string> first, std::set<std::string> second){
    std::set<std::string> diff;
    std::set_difference(first.begin(), first.end(), second.begin(), second.end(), std::inserter(diff, diff.begin()));
    return diff;
  }

  std::set<std::string> set_u(std::set<std::string> first, std::set<std::string> second){
    std::set<std::string> u;
    std::set_union(first.begin(), first.end(), second.begin(), second.end(), std::inserter(u, u.begin()));
    return u;
  }

  std::vector<Instruction*> find_successors(std::vector<Instruction*>::iterator& ins, std::vector<Instruction*>& instructions) {
    std::vector<Instruction*> ret;
    if (dynamic_cast<Instruction_ret*>(*ins)) {
        return ret;
    }
    else if (auto g = dynamic_cast<Instruction_goto*>(*ins)) {
        for (auto& i : instructions) {
            if (auto l = dynamic_cast<Instruction_label*>(i)) {
                if (l->label->data == g->label->data) {
                    ret.push_back(i);
                    return ret;
                }
            }
        }
    }
    else if (auto c1 = dynamic_cast<Instruction_one_label_jump*>(*ins)) {
        for (auto& i : instructions) {
            if (auto l = dynamic_cast<Instruction_label*>(i)) {
                if (l->label->data == c1->label1->data) {
                    ret.push_back(i);
                    ret.push_back(*(ins+1));
                    return ret;
                }
            }
        }    
    }
    else if (auto c2 = dynamic_cast<Instruction_two_label_jump*>(*ins)) {
        for (auto& i : instructions) {
            if (auto l = dynamic_cast<Instruction_label*>(i)) {
                if (l->label->data == c2->label1->data) {
                    ret.push_back(i);
                }
                if (l->label->data == c2->label2->data) {
                    ret.push_back(i);
                }
            }
        }
        return ret;
    }

    ret.push_back(*(ins+1));
    return ret;
  }

  void generate_in_out(Program p){
    auto f = p.functions.back();
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
            std::vector<Instruction*> succs = find_successors(it, f->instructions);
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

}