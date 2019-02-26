#pragma once

#include <set>
#include <map>

#include <L2.h>


namespace L2{
	std::string remove_percentage(std::string v);

	std::set<std::string> set_diff(std::set<std::string> first, std::set<std::string> second);

	std::set<std::string> set_u(std::set<std::string> first, std::set<std::string> second);

	std::vector<Instruction*> find_successors(std::vector<Instruction*>::iterator& ins, 
    std::vector<Instruction*>::iterator end, std::vector<Instruction*>& instructions);

    bool ins_contains(Instruction* i, std::string var);

    Instruction_assign *read_mem_ins(std::string spilled_var, int offset);

    Instruction_assign *write_mem_ins(std::string spilled_var, int offset);

    Instruction *ins_replace(Instruction* ins, std::string var, std::string spilled_var);

    Instruction *ins_replace_var_with_reg(Instruction* ins, std::map<std::string, std::string> &var_map);
}