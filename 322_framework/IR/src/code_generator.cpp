#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <code_generator.h>

namespace IR{
  int var_counter = 0;
  std::string ARRAY_INIT_VAL = "1";
  int START_OF_SECOND_LENGTH = 24;
  int START_OF_LENGTHS = 16;

  std::string new_var_name(std::string prefix) {
    std::string ret = prefix + std::to_string(var_counter);
    var_counter++;
    return ret;
  }

  std::string encode(int num) {
    num *= 2;
    ++num;
    return std::to_string(num);
  }

  std::string array_first_arg(std::vector<Item*> args) {
    long long total = 1;
    for (const auto& arg : args) {
      total *= (std::stoll(arg->data)-1);
    }

  }

  std::string assign_ins(std::string dst, std::string src) {
    return dst + " <- " + src + '\n';
  }

  std::string decode_ins(std::string var) {
    return var + " <- " + var + " >> 1\n";
  }

  std::string encode_ins(std::string var) {
    return var + " <- " + var + " << 1\n" +
           var + " <- " + var + " + 1\n";
  }

  std::string op_ins(std::string dst, std::string t1, std::string op, std::string t2) {
    return dst + " <- " + t1 + ' ' + op + ' ' + t2 + '\n';
  }

  std::string store_ins(std::string dst, std::string src) {
    return "store " + dst + " <- " + src + '\n';
  }

  std::string allocate_ins(std::string dst, std::string first_arg, std::string second_arg) {
    return dst + " <- call allocate(" + first_arg + ", " + second_arg + ")\n";
  }

  std::string load_ins(std::string dst, std::string src) {
    return dst + " <- load " + src + '\n';
  }

  std::string calculate_index_and_add_ins(std::string &array_name, std::vector<Item*> &indecies, 
                                          std::string &var_prefix, std::string &ret) {
    std::vector<std::string> dims = std::vector<std::string>();
    for (int j=0; j<indecies.size()-1; j++) {
      int offset = START_OF_SECOND_LENGTH + j*8;
      auto mem_var = new_var_name(var_prefix);
      auto var_itself = new_var_name(var_prefix);
      ret += op_ins(mem_var, array_name, "+", std::to_string(offset)) +
             load_ins(var_itself, mem_var) +
             decode_ins(var_itself);
      dims.push_back(var_itself);
    }

    auto index = new_var_name(var_prefix);
    ret += assign_ins(index, "0"); //initialize index to 0

    for (int j=0; j<indecies.size(); j++) {
      auto tmp_var = new_var_name(var_prefix);
      ret += assign_ins(tmp_var, indecies.at(j)->data);
      for (int k=j; k<dims.size(); k++) {
        ret += op_ins(tmp_var, tmp_var, "*", dims.at(k));
      }
      ret += op_ins(index, index, "+", tmp_var);
    }
    ret += op_ins(index, index, "*", "8");

    int offset = START_OF_LENGTHS + indecies.size()*8;
    ret += op_ins(index, index, "+", std::to_string(offset));
    return index;
  }

  std::string translate(Instruction * ins, std::string label_prefix, std::string var_prefix) {
    if (auto i = dynamic_cast<Instruction_assign*>(ins)) {
      return i->dst->data + " <- " + i->src->data;
    }
    else if (auto i = dynamic_cast<Instruction_op*>(ins)) {
      return i->dst->data + " <- " + i->t1->data + ' ' + i->op->data + ' ' + i->t2->data + '\n';
    }
    else if (auto i = dynamic_cast<Instruction_cmp*>(ins)) {
      return i->dst->data + " <- " + i->t1->data + ' ' + i->cmp->data + ' ' + i->t2->data + '\n';
    }
    else if (auto i = dynamic_cast<Instruction_ret*>(ins)) {
      return "return";
    }  
    else if (auto i = dynamic_cast<Instruction_ret_val*>(ins)) {
      return "return " + i->t->data;
    }  
    else if (auto i = dynamic_cast<Instruction_label*>(ins)) {
      return i->label->data;
    }
    else if (auto i = dynamic_cast<Instruction_br*>(ins)) {
      return "br " + i->label->data;
    }  
    else if (auto i = dynamic_cast<Instruction_br_var*>(ins)) {
      return "br " + i->var->data + ' ' + i->label1->data + '\n' +
             "br " + i->label2->data; 
    }
    else if (auto i = dynamic_cast<Instruction_call*>(ins)) {
      std::string ret = "call " + i->callee->data + '(';
      for (int j=0; j<i->args.size(); j++) {
        ret += i->args.at(j)->data;
        if (j!=i->args.size()-1) {
          ret += ", ";
        }
      }
      ret += ')';
      return ret;
    }
    else if (auto i = dynamic_cast<Instruction_call_var*>(ins)) {
      std::string ret = i->dst->data + " <- call " + i->callee->data + '(';
      for (int j=0; j<i->args.size(); j++) {
        ret += i->args.at(j)->data;
        if (j!=i->args.size()-1) {
          ret += ", ";
        }
      }
      ret += ')';      
      return ret; 
    }
    else if (auto i = dynamic_cast<Instruction_read_array*>(ins)) {
      std::string ret;
      std::string dst = i->dst->data;
      std::string array_name = i->src->data;      
      if (i->src->is_tuple()) {
        auto tmp_var = new_var_name(var_prefix);
        ret += op_ins(tmp_var, i->indecies.back()->data, "*", "8") +
               op_ins(tmp_var, tmp_var, "+", "8") +
               op_ins(tmp_var, array_name, "+", tmp_var) +
               load_ins(dst, tmp_var);
      }
      else {
        auto index_var = calculate_index_and_add_ins(array_name, i->indecies, var_prefix, ret);
        auto addr = new_var_name(var_prefix);
        ret += op_ins(addr, array_name, "+", index_var) +
               load_ins(dst, addr);
      }
      return ret;
    }
    else if (auto i = dynamic_cast<Instruction_write_array*>(ins)) {
      std::string ret;
      std::string array_name = i->dst->data;
      std::string value = i->src->data;
      if (i->dst->is_tuple()) {
        auto tmp_var = new_var_name(var_prefix);
        ret += op_ins(tmp_var, i->indecies.back()->data, "*", "8") +
               op_ins(tmp_var, tmp_var, "+", "8") +
               op_ins(tmp_var, array_name, "+", tmp_var) +
               store_ins(tmp_var, value);
      }
      else {
        auto index_var = calculate_index_and_add_ins(array_name, i->indecies, var_prefix, ret);
        auto addr = new_var_name(var_prefix);
        ret += op_ins(addr, array_name, "+", index_var) +
               store_ins(addr, value);
      }
      return ret;
    }
    else if (auto i = dynamic_cast<Instruction_length*>(ins)) {
      std::string ret;
      auto offset_var = new_var_name(var_prefix);
      ret += op_ins(offset_var, i->t->data, "*", "8") + 
             op_ins(offset_var, offset_var, "+", "16") +
             op_ins(offset_var, offset_var, "+", i->array->data) +
             load_ins(i->dst->data, offset_var);
      return ret;
    }
    else if (auto i = dynamic_cast<Instruction_new_array*>(ins)) {
      std::string ret;
      auto first_arg = new_var_name(var_prefix);
      auto array_name = i->dst->data;
      ret += assign_ins(first_arg, i->args.back()->data) +
             decode_ins(first_arg);
      for (int j=0; j<i->args.size()-1; j++) {
        auto tmp_var = new_var_name(var_prefix);
        ret += assign_ins(tmp_var, i->args.at(j)->data) +
               decode_ins(tmp_var) +
               op_ins(first_arg, first_arg, "*", tmp_var);
      }
      ret += op_ins(first_arg, first_arg, "+", std::to_string(1+i->args.size())) +
             encode_ins(first_arg);
      ret += allocate_ins(array_name, first_arg, ARRAY_INIT_VAL);
      auto array_mem = new_var_name(var_prefix);
      ret += op_ins(array_mem, array_name, "+", "8");
      ret += store_ins(array_mem, encode(i->args.size()));
      for (const auto& arg : i->args) {
        ret += op_ins(array_mem, array_mem, "+", "8");
        ret += store_ins(array_mem, arg->data);
      }
      return ret;
    }
    else if (auto i = dynamic_cast<Instruction_new_tuple*>(ins)) {
      std::string ret;
      auto tuple_name = i->dst->data;
      ret += allocate_ins(tuple_name, i->t->data, ARRAY_INIT_VAL);
      return ret;
    }
    return "";
  }

  std::string longest_var(Function* &f) {
    std::string longest_var = "%v";
    for (const auto &var : f->vars_used) {
      if (var->data.length()>longest_var.length()) {
        longest_var = var->data;
      }
    }
    return longest_var;
  }

  void generate_code(Program p) {
    std::ofstream of;
    of.open("prog.L3");
    for (auto &f : p.functions) {
      int arg_size = f->args.size();
      of << "define " << f->name << '(';
      for (int j=0; j<f->args.size(); j++) {
        of << f->args.at(j)->data;
        if (j!=f->args.size()-1) {
          of << ", ";
        }
      }
      of << ") {\n";
      auto tmp_var = longest_var(f) + "tmp";
      for (auto &i : f->instructions) {
        of << translate(i, p.longest_label, tmp_var) << '\n';
      }
      of << "}\n";
    }
    of.close();
    return;
  }
}
