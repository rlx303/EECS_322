#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <string>

#include <utils.h>
#include <L2.h>


namespace L2{

  
  std::string remove_percentage(std::string v) {
	if (v.at(0)=='%') {
		return v.erase(0, 1);
	}
	else {
		return v;
	}
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

  std::vector<Instruction*> find_successors(std::vector<Instruction*>::iterator& ins, 
    std::vector<Instruction*>::iterator end, std::vector<Instruction*>& instructions) 
  {
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
    if ((ins+1) != end) {
        ret.push_back(*(ins+1));
    }
    return ret;
  }

  void ig_insert(std::map<std::string, std::set<std::string>>& ig, std::string key, std::string value) {
    if(key != value) {
        ig.insert(std::make_pair(key, std::set<std::string>()));
        ig[key].insert(value);
        ig.insert(std::make_pair(value, std::set<std::string>()));
        ig[value].insert(key);
    }
  }

  void ig_connect(std::map<std::string, std::set<std::string>>& ig, std::set<std::string> set1, std::set<std::string> set2) {
  	if (set1.empty()){
  		ig_init_set(ig, set2);
  	} 
  	else if (set2.empty()) {
  		ig_init_set(ig, set1);
  	}
    for (const auto &val1 : set1) {
        for (const auto &val2 : set2) {
            ig_insert(ig, val1, val2);
        }
    } 
  }

  void ig_init_set(std::map<std::string, std::set<std::string>>& ig, std::set<std::string> set) {
    for (const auto &val : set) {
        ig.insert(std::make_pair(val, std::set<std::string>()));
    }   
  }


  bool skip_kill_out(Instruction* i) {
    if (auto a = dynamic_cast<Instruction_assign*>(i)) {
        if (a->op->is_arrow()){
            if (a->src->is_var() || a->src->is_reg()) {
                if (a->dst->is_var() || a->dst->is_reg()) {
                    return true;
                }
            }
        }
    }
    return false;
  }

  void sop_constraint(Instruction* i, std::map<std::string, std::set<std::string>>& ig) {
    if (auto a = dynamic_cast<Instruction_assign*>(i)) {
        if (a->op->is_sop()){
            if (a->src->is_var()) {
                std::string var = a->src->get_L2();
                for (const auto &pair : reg_map) {
                    if (pair.first != "rcx") {
                        ig_insert(ig, var, pair.first);
                    }
                }
            }
        }
    }
  }
   
  bool ins_contains(Instruction* i, std::string var) {
  	return i->print_L2().find(var) != std::string::npos;
  }

  Instruction_assign* read_mem_ins(std::string spilled_var, int offset) {
  	Instruction_assign* ins = new Instruction_assign();
  	ins->src = new I_memxM("rsp", std::to_string(offset));
  	ins->dst = new I_var(spilled_var);
  	ins->op = new I_arrow("<-");
  	return ins;
  }

  Instruction_assign* write_mem_ins(std::string spilled_var, int offset) {
  	Instruction_assign* ins = new Instruction_assign();
  	ins->src = new I_var(spilled_var);
  	ins->dst = new I_memxM("rsp", std::to_string(offset));
  	ins->op = new I_arrow("<-");
  	return ins;
  }

  Instruction *ins_replace(Instruction *ins, std::string var, std::string spilled_var) {
  	if (auto a = dynamic_cast<Instruction_assign*>(ins)) {
        if (a->dst->data==var) {
        	a->dst->data=spilled_var;
        }
        if (a->src->data==var) {
        	a->src->data=spilled_var;
        }
    }
    else if (auto a = dynamic_cast<Instruction_inc_dec*>(ins)) {
        if (a->dst->data==var) {
        	a->dst->data=spilled_var;
        }
    }
    else if (auto a = dynamic_cast<Instruction_two_label_jump*>(ins)) {
        if (a->t1->data==var) {
        	a->t1->data=spilled_var;
        }
        if (a->t2->data==var) {
        	a->t2->data=spilled_var;
        }    
    }
    else if (auto a = dynamic_cast<Instruction_one_label_jump*>(ins)) {
        if (a->t1->data==var) {
        	a->t1->data=spilled_var;
        }
        if (a->t2->data==var) {
        	a->t2->data=spilled_var;
        } 
    }
    else if (auto a = dynamic_cast<Instruction_wwe*>(ins)) {
        if (a->dst->data==var) {
        	a->dst->data=spilled_var;
        }        
        if (a->w1->data==var) {
        	a->w1->data=spilled_var;
        }
        if (a->w2->data==var) {
        	a->w2->data=spilled_var;
        } 
    }
    else if (auto a = dynamic_cast<Instruction_call*>(ins)) {
        if (a->label->data==var) {
        	a->label->data=spilled_var;
        } 
    }
    return ins;
  }

}

