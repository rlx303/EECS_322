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
    std::ofstream outputFile;
    outputFile.open("prog.S");
   
    /* 
     * Generate target code
     */ 
    //TODO

    /* 
     * Close the output file.
     */ 
    outputFile.close();
   
    return ;
  }
}
