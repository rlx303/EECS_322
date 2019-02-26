#pragma once

#include <stack>
#include <map>

#include <L2.h>

namespace L2{
	class Node;

	class IG {
	private:
		std::map<Node, std::set<Node>> ig;
		std::vector<std::string> colors = {
			"r10",
			"r11",
			"r8",
			"r9",
			"rax",
			"rcx",
			"rdi",
			"rdx",
			"rsi",
			"r12",
			"r13",
			"r14",
			"r15",
			"rbp",
			"rbx"
		};
		std::string find_first_free_color(const std::set<std::string> &neighbor_colors) const;
		void color_node(const Node &node, const std::string &color);
	public:
		IG(){};
		void insert(std::string key, std::string value);
		void connect(const std::set<std::string> &set1, const std::set<std::string> &set2);
		void init_set(const std::set<std::string> &set);
		void sop_constraint(Instruction* i);
		void print() const;
		int num_edges(const Node &key) const;
		Node remove(const Node &key);
		Node remove_next();
		bool empty() const;
		std::set<Node> add_colors_to_nodes(std::stack<Node> &nodes);
		void print_node_color() const;
		std::string longest_var_name() const;
		std::map<std::string, std::string> get_var_map() const;
	};

	class Node {
	private:
		std::string item;
		mutable std::string color;
		mutable bool removed=false;
		mutable bool spilled=false;
	public:
		Node(){};
		Node(std::string i) : item(i){
			if (i.at(0)!='%') color = i;
		}
		Node(std::string i, std::string c) : item(i), color(c){}
		std::string get_color() const;
		void set_color(std::string new_color) const;
		std::string get_item() const;
		bool operator==(const Node& rhs) const {
			return this->get_item()==rhs.get_item();
		}
		bool operator<(const Node& rhs) const {
			return this->get_item()<rhs.get_item();
		}
		void remove() const {removed=true;}
		void unremove() const {removed=false;}
		bool is_removed() const {return removed;}
		void spill() const {spilled=true;}
		bool is_spilled() const {return spilled;}
		bool has_color() const {return color!="";}
	};
}