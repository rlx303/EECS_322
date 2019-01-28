#include <string>
#include <iostream>
#include <fstream>

#include <code_generator.h>

using namespace std;

namespace L1{

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
    {"rsi", "sil"}
  };

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

    for (auto f : p.functions){    
        of << f->name.replace(0, 1, "_") << ":\n";
        of << "subq $" << f->locals*8 << ", %rsp #Allocate locals\n";
        for (auto i : f->instructions) {
            of << i->print_x86() << "\n";
        }
    }

    /* 
     * Close the output file.
     */ 
    of.close();
   
    return ;
  }
}