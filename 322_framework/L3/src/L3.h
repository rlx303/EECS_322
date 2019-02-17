#pragma once

#include <iostream>
#include <vector>

namespace L3 {
  /*
   * Items
   */
	struct Item{
		std::string data;
		Item(std::string input) {data = input;}
		virtual bool is_var() {return false;}
		virtual bool is_runtime() {return false;}
		virtual bool need_flip() {return false;}
		virtual bool is_label() {return false;}
	};

	struct I_var : Item{
    I_var(std::string input) :Item(input) {}
		bool is_var() override {return true;}
	};

	struct I_label : Item{
    I_label(std::string input) :Item(input) {}
    bool is_label() override {return true;}
	};

	struct I_num : Item{
    I_num(std::string input) :Item(input) {}
	};
	struct I_op : Item{
    I_op(std::string input) :Item(input) {}
	};
	struct I_cmp : Item{
    I_cmp(std::string input) :Item(input) {}
    bool need_flip() override {
    	if (data==">=" || data==">") {
    		return true;
    	}
    	return false;
    }
	};

	struct I_runtime : Item{
		I_runtime(std::string input) :Item(input) {}
		bool is_runtime() override {return true;}
	};
	/*
	 * Instructions
	 */
	struct Instruction{
		std::set<I_var*> gen;
		std::set<I_var*> kill;
		std::set<I_var*> in;
		std::set<I_var*> out;
  	virtual ~Instruction() = default;
	};

	struct Instruction_assign : Instruction{
		I_var* dst;
		Item* src; 
		Instruction_assign(I_var* dst, Item* src) :dst(dst), src(src) {
			kill.insert(dst);
			if (src->is_var()) {
				gen.insert((I_var*)src);
			}
		}
	};

	struct Instruction_op : Instruction{
		I_var* dst;
		Item* t1;
		I_op* op;
		Item* t2;
		Instruction_op(I_var* dst, Item* t1, I_op* op, Item* t2) 
		:dst(dst), t1(t1), op(op), t2(t2) {
			kill.insert(dst);
			if (t1->is_var()) {
				gen.insert((I_var*)t1);
			}
			if (t2->is_var()) {
				gen.insert((I_var*)t2);
			}
		}
	};

	struct Instruction_cmp : Instruction{
		I_var* dst;
		Item* t1;
		I_cmp* cmp;
		Item* t2;
		Instruction_cmp(I_var* dst, Item* t1, I_cmp* cmp, Item* t2) 
		:dst(dst), t1(t1), cmp(cmp), t2(t2) {
			kill.insert(dst);
			if (t1->is_var()) {
				gen.insert((I_var*)t1);
			}
			if (t2->is_var()) {
				gen.insert((I_var*)t2);
			}
		}	
	};

	struct Instruction_load : Instruction{
		I_var* dst;
		I_var* src;
		Instruction_load(I_var* dst, I_var* src) : dst(dst), src(src) {
			kill.insert(dst);
			gen.insert(src);
		}
	};

	struct Instruction_store : Instruction{
		I_var* dst;
		Item* src;
		Instruction_store(I_var* dst, Item* src) : dst(dst), src(src) {
			gen.insert(dst);
			if (src->is_var()) {
				gen.insert((I_var*)src);
			}
		}
	};

	struct Instruction_ret : Instruction{
	};

	struct Instruction_ret_val : Instruction{
		Item* t;
		Instruction_ret_val(Item* t) :t(t) {
			if (t->is_var()) {
				gen.insert((I_var*)t);
			}
		}
	};

	struct Instruction_label : Instruction{
		I_label* label;
		Instruction_label(I_label* label) :label(label) {}
	};

	struct Instruction_br : Instruction{
		I_label* label;
		Instruction_br(I_label* label) :label(label) {}
	};

	struct Instruction_br_var : Instruction{
		I_var* var;
		I_label* label;
		Instruction_br_var(I_var* var, I_label* label) :var(var), label(label) {
			gen.insert(var);
		}
	};

	struct Instruction_call : Instruction{
		Item* callee;
		std::vector<Item*> args;
		bool is_runtime;
		Instruction_call(Item* callee, std::vector<Item*> args) : 
		callee(callee), args(args) {
			is_runtime = callee->is_runtime();
			if (callee->is_var()) {
				gen.insert((I_var*)callee);
			}
			for (auto &a : args) {
				if (a->is_var()) {
					gen.insert((I_var*)a);
				}
			}
		}
	};

	struct Instruction_call_var : Instruction{
		I_var* dst;
		Item* callee;
		std::vector<Item*> args;
		bool is_runtime;
		Instruction_call_var(I_var* dst, Item* callee, std::vector<Item*> args) :
		dst(dst), callee(callee), args(args) {
			is_runtime = callee->is_runtime();
			kill.insert(dst);
			if (callee->is_var()) {
				gen.insert((I_var*)callee);
			}
			for (auto &a : args) {
				if (a->is_var()) {
					gen.insert((I_var*)a);
				}
			}
		}
	};

	/*
	 * Function
	 */
	struct Function{
		std::string name;
		std::vector<Instruction *> instructions;
		std::vector<I_var *> args;
		std::vector<I_var *> vars_used;
		Function(std::string fname) : name(fname){}
	};

	/*
	 * Program
	 */
  struct Program{
    std::string entryPointLabel;
    std::vector<Function *> functions;
		std::string longest_label = ":a";
  };
}