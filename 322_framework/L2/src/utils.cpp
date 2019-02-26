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
    else if (auto a = dynamic_cast<Instruction_cmp*>(ins)) {
    	if (a->dst->data==var) {
    		a->dst->data=spilled_var;
    	}
    	if (a->t1->data==var) {
    		a->t1->data=spilled_var;
    	}
    	if (a->t2->data==var) {
    		a->t2->data=spilled_var;
    	}
    }
    return ins;
  }

  Instruction *ins_replace_var_with_reg(Instruction* ins, std::map<std::string, std::string> &var_map) {
  	if (auto a = dynamic_cast<Instruction_assign*>(ins)) {
        if (var_map.count(a->dst->data)==1) {
        	a->dst->data=var_map[a->dst->data];
        }
        if (var_map.count(a->src->data)==1) {
        	a->src->data=var_map[a->src->data];
        }
    }
    else if (auto a = dynamic_cast<Instruction_inc_dec*>(ins)) {
        if (var_map.count(a->dst->data)==1) {
        	a->dst->data=var_map[a->dst->data];
        }
    }
    else if (auto a = dynamic_cast<Instruction_two_label_jump*>(ins)) {
        if (var_map.count(a->t1->data)==1) {
        	a->t1->data=var_map[a->t1->data];
        }
        if (var_map.count(a->t2->data)==1) {
        	a->t2->data=var_map[a->t2->data];
        }    
    }
    else if (auto a = dynamic_cast<Instruction_one_label_jump*>(ins)) {
        if (var_map.count(a->t1->data)==1) {
        	a->t1->data=var_map[a->t1->data];
        }
        if (var_map.count(a->t2->data)==1) {
        	a->t2->data=var_map[a->t2->data];
        } 
    }
    else if (auto a = dynamic_cast<Instruction_wwe*>(ins)) {
		if (var_map.count(a->dst->data)==1) {
        	a->dst->data=var_map[a->dst->data];
        }        
        if (var_map.count(a->w1->data)==1) {
        	a->w1->data=var_map[a->w1->data];
        }
        if (var_map.count(a->w2->data)==1) {
        	a->w2->data=var_map[a->w2->data];
        }
    }
    else if (auto a = dynamic_cast<Instruction_call*>(ins)) {
        if (var_map.count(a->label->data)==1) {
        	a->label->data=var_map[a->label->data];
        } 
    }
    else if (auto a = dynamic_cast<Instruction_cmp*>(ins)) {
    	if (var_map.count(a->dst->data)==1) {
    		a->dst->data=var_map[a->dst->data];
    	}
        if (var_map.count(a->t1->data)==1) {
        	a->t1->data=var_map[a->t1->data];
        }
        if (var_map.count(a->t2->data)==1) {
        	a->t2->data=var_map[a->t2->data];
        } 
    }
    return ins;  
  }

}

