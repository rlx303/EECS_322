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

    void ig_insert(std::map<std::string, std::set<std::string>>& ig, std::string key, std::string value);

    void ig_connect(std::map<std::string, std::set<std::string>>& ig, std::set<std::string> set1, std::set<std::string> set2);

    void ig_init_set(std::map<std::string, std::set<std::string>>& ig, std::set<std::string> set);

    bool skip_kill_out(Instruction* i);

    void sop_constraint(Instruction* i, std::map<std::string, std::set<std::string>>& ig);

    bool ins_contains(Instruction* i, std::string var);

    Instruction_assign *read_mem_ins(std::string spilled_var, int offset);

    Instruction_assign *write_mem_ins(std::string spilled_var, int offset);

    Instruction *ins_replace(Instruction* i, std::string var, std::string spilled_var);
}