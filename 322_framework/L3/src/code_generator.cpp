#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <stack>
#include <string>

#include <code_generator.h>

namespace L3{
	std::string return_register = "rax";
	std::string stack_pointer = "rsp";
	std::vector<std::string> argument_registers {
		"rdi",
		"rsi",
		"rdx",
		"rcx",
		"r8",
		"r9"
	};
	int counter = 0;

	std::string translate_ins(Instruction * ins, std::string label_prefix, std::string tmp_var) {
		if (auto i = dynamic_cast<Instruction_assign*>(ins)) {
			return i->dst->data + " <- " + i->src->data;
		}
		else if (auto i = dynamic_cast<Instruction_op*>(ins)) {
			return tmp_var + " <- " + i->t1->data + '\n' + 
			tmp_var + ' ' + i->op->data + "= " + i->t2->data + '\n' +
			i->dst->data + " <- " + tmp_var;
		}
		else if (auto i = dynamic_cast<Instruction_cmp*>(ins)) {
			if (i->cmp->data==">=") {
				return i->dst->data + " <- " + i->t2->data + " <= " + i->t1->data;
	 		}
	 		else if (i->cmp->data==">") {
				return i->dst->data + " <- " + i->t2->data + " < " + i->t1->data;
	 		}
	 		else {
				return i->dst->data + " <- " + i->t1->data + ' ' + i->cmp->data + ' ' + i->t2->data;
	 		}
		}  
		else if (auto i = dynamic_cast<Instruction_load*>(ins)) {
			return i->dst->data + " <- mem " + i->src->data + " 0" ;
		}  
		else if (auto i = dynamic_cast<Instruction_store*>(ins)) {
			return "mem " + i->dst->data + " 0 <- " + i->src->data;
		}  
		else if (auto i = dynamic_cast<Instruction_ret*>(ins)) {
			return "return";
		}  
		else if (auto i = dynamic_cast<Instruction_ret_val*>(ins)) {
			return return_register + " <- " + i->t->data + "\nreturn";
		}  
		else if (auto i = dynamic_cast<Instruction_label*>(ins)) {
			return i->label->data;
		}  
		else if (auto i = dynamic_cast<Instruction_br*>(ins)) {
			return "goto " + i->label->data;
		}  
		else if (auto i = dynamic_cast<Instruction_br_var*>(ins)) {
			return "cjump 1 = " + i->var->data + ' ' + i->label->data; 
		}
		else if (auto i = dynamic_cast<Instruction_call*>(ins)) {
			std::string ret;
			if (i->callee->is_runtime()) {
				for (int j=0; j<i->args.size(); j++) {
					ret += argument_registers.at(j) + " <- " + i->args.at(j)->data + '\n';
				}
				ret += "call " + i->callee->data + ' ' + std::to_string(i->args.size());
			}
			else {
				std::string ret_label = label_prefix + "ret" + std::to_string(counter);
				ret += "mem " + stack_pointer + " -8 <- " + 
				ret_label + '\n';
				int extra = (int)i->args.size() - (int)argument_registers.size();
				for (int j=0; j<i->args.size(); j++) {
					if(j<argument_registers.size()) {
						ret += argument_registers.at(j) + " <- " + i->args.at(j)->data + '\n';
					} 
					else {
						int offset = j-(int)argument_registers.size();
						std::cout << -8-8*(j-(int)argument_registers.size()+1) << std::endl;
						ret += "mem " + stack_pointer + ' ' + 
						std::to_string(-8-8*extra + offset*8) + 
						" <- " + i->args.at(j)->data + '\n';
					}
				}
				ret += "call " + i->callee->data + ' ' + std::to_string(i->args.size()) + '\n';
				ret += ret_label;
				counter++;
			}
			return ret;
		}  
		else if (auto i = dynamic_cast<Instruction_call_var*>(ins)) {
			std::string ret;
			if (i->callee->is_runtime()) {
				for (int j=0; j<i->args.size(); j++) {
					ret += argument_registers.at(j) + " <- " + i->args.at(j)->data + '\n';
				}
				ret += "call " + i->callee->data + ' ' + std::to_string(i->args.size()) + '\n';
			}
			else {
				std::string ret_label = label_prefix + "ret" + std::to_string(counter);
				ret += "mem " + stack_pointer + " -8 <- " + 
				ret_label + '\n';
				int extra = (int)i->args.size() - (int)argument_registers.size();
				for (int j=0; j<i->args.size(); j++) {
					if(j<argument_registers.size()) {
						ret += argument_registers.at(j) + " <- " + i->args.at(j)->data + '\n';
					} 
					else {
						int offset = j-(int)argument_registers.size();
						ret += "mem " + stack_pointer + ' ' + 
						std::to_string(-8-8*extra + offset*8) + 
						" <- " + i->args.at(j)->data + '\n';
					}
				}
				ret += "call " + i->callee->data + ' ' + std::to_string(i->args.size()) + '\n';
				ret += ret_label + '\n';
				counter++;
			}
			ret += i->dst->data + " <- " + return_register;
			return ret;	
		}    
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

  void labels_unique(Program &p) {
  	 int counter = 0;
  	std::set<std::string> f_names;
  	 for (auto &f: p.functions) {
  	 	f_names.insert(f->name);
  	 }
  	for (auto &f: p.functions) {
  		std::string prefix = p.longest_label;
  		std::map<std::string, std::string> label_map;
  		for (auto &i: f->instructions) {
  			if (auto a = dynamic_cast<Instruction_assign*>(i)) {
				if (a->src->is_label()) {
					if (f_names.count(a->src->data)==0) {
						if (label_map.count(a->src->data)==0) {
							std::string new_label = prefix + std::to_string(counter);
							label_map.insert(std::pair<std::string, std::string>(a->src->data, new_label));
							counter++;
						}
						a->src->data = label_map[a->src->data];
					}
				}
			}
			else if (auto a = dynamic_cast<Instruction_store*>(i)) {
				if (a->src->is_label()) {
					if (f_names.count(a->src->data)==0) {
						if (label_map.count(a->src->data)==0) {
							std::string new_label = prefix + std::to_string(counter);
							label_map.insert(std::pair<std::string, std::string>(a->src->data, new_label));
							counter++;
						}
						a->src->data = label_map[a->src->data];
					}	
				}
			}
			else if (auto a = dynamic_cast<Instruction_label*>(i)) {
				if (a->label->is_label()) {
					if (f_names.count(a->label->data)==0) {
						if (label_map.count(a->label->data)==0) {
							std::string new_label = prefix + std::to_string(counter);
							label_map.insert(std::pair<std::string, std::string>(a->label->data, new_label));
							counter++;
						}
						a->label->data = label_map[a->label->data];
					}
				}
			}
			else if (auto a = dynamic_cast<Instruction_br*>(i)) {
				if (a->label->is_label()) {
					if (f_names.count(a->label->data)==0) {
						if (label_map.count(a->label->data)==0) {
							std::string new_label = prefix + std::to_string(counter);
							label_map.insert(std::pair<std::string, std::string>(a->label->data, new_label));
							counter++;
						}
						a->label->data = label_map[a->label->data];
					}
				}				
			}
			else if (auto a = dynamic_cast<Instruction_br_var*>(i)) {
				if (a->label->is_label()) {
					if (f_names.count(a->label->data)==0) {
						if (label_map.count(a->label->data)==0) {
							std::string new_label = prefix + std::to_string(counter);
							label_map.insert(std::pair<std::string, std::string>(a->label->data, new_label));
							counter++;
						}					
						a->label->data = label_map[a->label->data];
					}
				}			
			}
			else if (auto a = dynamic_cast<Instruction_call*>(i)) {
				if (a->callee->is_label()) {
					if (f_names.count(a->callee->data)==0) {
						if (label_map.count(a->callee->data)==0) {
							std::string new_label = prefix + std::to_string(counter);
							label_map.insert(std::pair<std::string, std::string>(a->callee->data, new_label));
							counter++;
						}
						a->callee->data = label_map[a->callee->data];
					}
				}
			}
			else if (auto a = dynamic_cast<Instruction_call_var*>(i)) {
				if (a->callee->is_label()) {
					if (f_names.count(a->callee->data)==0) {
						if (label_map.count(a->callee->data)==0) {
							std::string new_label = prefix + std::to_string(counter);
							label_map.insert(std::pair<std::string, std::string>(a->callee->data, new_label));
							counter++;
						}
						a->callee->data = label_map[a->callee->data];
					}
				}
			}
  		}
  	}
  }

  void generate_code(Program p) {
  	std::ofstream of;
    of.open("prog.L2");
  	of << "(:main\n";
  	labels_unique(p);
  	for (auto &f : p.functions) {
  		int arg_size = f->args.size();
  		of << '(' << f->name << '\n';
  		of << std::to_string(arg_size) << " 0\n";
  		for (int i=0; i<arg_size; i++) {
				if(i<argument_registers.size()) {
					of << f->args.at(i)->data << " <- " << argument_registers.at(i) << '\n';
				} 
				else {
					of << f->args.at(i)->data << " <- stack-arg " << 8*(i-(int)argument_registers.size()) << '\n';
				}
  		}
  		auto tmp_var = longest_var(f) + "tmp";
  		for (auto &i : f->instructions) {
  			of << translate_ins(i, p.longest_label, tmp_var) << std::endl;
  		}
  		of << ")\n";
  	}
  	of << ")\n";
    of.close();
    return;
  }
}
