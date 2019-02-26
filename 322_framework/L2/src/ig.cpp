#include <iostream>
#include <map>
#include <stack>

#include <ig.h>
#include <utils.h>

namespace L2 {

	std::string IG::find_first_free_color(const std::set<std::string> &neighbor_colors) const{
		for (const auto &color : colors) {
			if (neighbor_colors.count(color)==0) {
				return color;
			}
		}
		return "";
	}

	void IG::insert(std::string key, std::string value) {
		if(key != value) {
		    ig.insert(std::make_pair(Node(key), std::set<Node>()));
		    ig.at(Node(key)).insert(Node(value));
		    ig.insert(std::make_pair(Node(value), std::set<Node>()));
		    ig.at(Node(value)).insert(Node(key));
		}
	}

	void IG::connect(const std::set<std::string> &set1, const std::set<std::string> &set2) {
		if (set1.empty()){
			this->init_set(set2);
		} 
		else if (set2.empty()) {
			this->init_set(set1);
		}
	  	for (const auto &val1 : set1) {
	      	for (const auto &val2 : set2) {
	          	this->insert(val1, val2);
	     	}
	  	} 	
	}

	void IG::init_set(const std::set<std::string> &set) {
		for (const auto &val : set) {
		    ig.insert(std::make_pair(Node(val), std::set<Node>()));
		}
	}

	void IG::sop_constraint(Instruction* i) {
		if (auto a = dynamic_cast<Instruction_assign*>(i)) {
		    if (a->op->is_sop()){
		        if (a->src->is_var()) {
		            std::string var = a->src->get_L2();
		            for (const auto &pair : reg_map) {
		                if (pair.first != "rcx") {
		                    this->insert(var, pair.first);
		                }
		            }
		        }
		    }
		}	
	}

	int IG::num_edges(const Node &key) const {
		return ig.at(key).size();
	}

	Node IG::remove(const Node &key) {
		for (auto &map_pair : ig) {
			if (map_pair.first==key) {
				map_pair.first.remove();
				return key;
			}
		}
		return Node();
	}

	Node IG::remove_next() {
		bool under15 = false;
		Node max_key;
		int max_num = 0;
		Node max_under_key;
		int max_under_num = 0;
		for (auto &pair : ig) {
			if (!pair.first.is_removed()) {
				int edges = num_edges(pair.first);
				if (edges<15) {
					under15 = true;
					if (edges>=max_under_num) {
						max_under_key = pair.first;
						max_under_num = edges;
					}
				}
				else {
					if (edges>=max_num) {
						max_key = pair.first;
						max_num = edges;
					}
				}
			}
		}
		return under15 ? remove(max_under_key) : remove(max_key);
	}

	std::set<std::string> get_colors(const std::set<Node> &nodes) {
		std::set<std::string> colors;
		for (const auto &node : nodes) {
			if (node.has_color()){
				colors.insert(node.get_color());
			}
		}
		return colors;
	}

	void IG::color_node(const Node &node, const std::string &color) {
		for (auto &ig_entry : ig) {
			if(ig_entry.first == node) {
				ig_entry.first.set_color(color);
			}
			for (auto &neighbor : ig_entry.second) {
				if (neighbor == node) {
					neighbor.set_color(color);
				}
			}
		}
	}

	std::set<Node> IG::add_colors_to_nodes(std::stack<Node> &nodes){
		std::set<Node> spilled;
		while(!nodes.empty()) {
			auto it = ig.find(nodes.top());
			if (!it->first.has_color()) {
				auto neighbor_colors = get_colors(it->second);
				// std::cout << "name " << it->first.get_item() << std::endl;
				// if (it->first.get_item()=="%myV2") {
				// 	for (const auto &c : neighbor_colors) {
				// 		std::cout << c << ' ';
				// 	}
				// 	std::cout << std::endl;
				// }
				if (neighbor_colors.size()==colors.size()){
					it->first.spill();
					spilled.insert(it->first);
				}
				else {
					auto this_color = find_first_free_color(neighbor_colors);
					color_node(nodes.top(), this_color);
				}			
			}
			nodes.pop();
		}
		return spilled;
	}

	bool IG::empty() const {
		for (auto const& ig_entry : ig) {
			if(!ig_entry.first.is_removed()) {
				return false;
			}
		}
		return true;
	}

	void IG::print() const {
		for (auto const& ig_entry : ig) {
		  std::cout << remove_percentage(ig_entry.first.get_item()) << ' ';
		  for (auto const& node : ig_entry.second){
		    std::cout << remove_percentage(node.get_item()) << ' ';
		  }
		  std::cout << std::endl;
		}
	}

	void IG::print_node_color() const {
		for (auto const& ig_entry : ig) {
			std::cout << remove_percentage(ig_entry.first.get_item()) 
			<< ": " << ig_entry.first.get_color() << std::endl;
		}
	}

	std::string IG::longest_var_name() const {
		std::string longest = "%S";
		for (auto const& ig_entry : ig) {
			std::string var_name = ig_entry.first.get_item();
			if (var_name.at(0)=='%' && var_name.length()>longest.length()) {
				longest = var_name;
			}
		}
		return longest;
	}

	std::map<std::string, std::string> IG::get_var_map() const {
		std::map<std::string, std::string> var_map;
		for (const auto& ig_entry : ig) {
			Node var = ig_entry.first;
			if (var.get_item().at(0)=='%') {
				var_map.insert(std::make_pair(var.get_item(), var.get_color()));
			}
		}
		return var_map;
	}

	std::string Node::get_color() const {
		return this->color;
	}

	std::string Node::get_item() const {
		return this->item;
	}

	void Node::set_color(std::string new_color) const {
		this->color = new_color;
	}
}