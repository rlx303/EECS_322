#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include <L3.h>
#include <parser.h>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace L3 {
	/*
	 * Data required to parse
	 */
	std::vector<Item*> parsed_registers;
  /*
   * Grammar rules from now on.
   */
  struct space:
    pegtl::star<
      internal::one< 
        internal::result_on_found::SUCCESS, 
        internal::peek_char, 
        ' ', 
        '\t' 
      >
    >{};

  struct name:
    pegtl::seq<
      pegtl::plus<
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >
        >
      >,
      pegtl::star<
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >,
          pegtl::digit
        >
      >
    > {};

  struct label:
    pegtl::seq<
      pegtl::one<':'>,
      name
    > {};

  struct function_name:
  	label {};

  struct number:
    pegtl::seq<
      pegtl::opt<
        pegtl::sor<
          pegtl::one< '-' >,
          pegtl::one< '+' >
        >
      >,
      pegtl::plus<
        pegtl::digit
      >
    >{};

  struct comment:
    pegtl::disable<
      TAOCPP_PEGTL_STRING( "//" ),
      pegtl::until< pegtl::eolf >
    > {};


  struct var:
  	pegtl::seq<
  		pegtl::one<'%'>,
  		name
  	> {};

  struct cmp:
  	pegtl::sor<
      TAOCPP_PEGTL_STRING( "<=" ),
      TAOCPP_PEGTL_STRING( ">=" ),
  		pegtl::one<'<'>,
  		pegtl::one<'>'>,
  		pegtl::one<'='>
  	> {};

  struct op:
  	pegtl::sor<
	  	TAOCPP_PEGTL_STRING( "<<" ),
	  	TAOCPP_PEGTL_STRING( ">>" ),
  		pegtl::one<'+'>,
  		pegtl::one<'-'>,
  		pegtl::one<'*'>,
  		pegtl::one<'&'>
  	> {};

  struct u:
  	pegtl::sor<
  		var,
  		label
  	> {};

  struct t:
  	pegtl::sor<
  		var,
  		number
  	> {};

  struct s:
  	pegtl::sor<
  		t,
  		label
  	> {};

	struct seps:
	  pegtl::star<
	    pegtl::sor<
	      pegtl::ascii::space,
	      comment
	    >
	  > {};

 	struct args:
 		pegtl::sor<
 			pegtl::seq<
 				seps,
 				t,
 				seps,
 				pegtl::star<
 					seps,
 					pegtl::one<','>,
 					seps,
 					t,
 					seps>
 			>,
 			pegtl::seq<
 		  	seps,
 		  	t,
 		  	seps
 			>,
 			seps
 		> {};

  struct vars:
 		pegtl::sor<
 			pegtl::seq<
 				seps,
 				var,
 				seps,
 				pegtl::star<
 					seps,
 					pegtl::one<','>,
 					seps,
 					var,
 					seps
 				>
 			>,
 			pegtl::seq<
 		  	seps,
 		  	var,
 		  	seps
 			>,
 			seps
 		> {};

 	struct runtime_f:
 		pegtl::sor<
 			TAOCPP_PEGTL_STRING( "print" ),
	  	TAOCPP_PEGTL_STRING( "allocate" ),
	  	TAOCPP_PEGTL_STRING( "array-error" )
 		>{};

  struct callee:
  	pegtl::sor<
  		u,
	  	runtime_f
  	> {};

  struct arrow: 
    TAOCPP_PEGTL_STRING("<-")
    {};

  /**
  *
  *
  Instructions:
	*
	*
  **/
  struct instruction_assign:
  	pegtl::seq<
  		var,
  		seps,
  		arrow,
  		seps,
  		s
  	> {};

  struct instruction_op:
  	pegtl::seq<
  		var,
  		seps,
  		arrow,
  		seps,
  		t,
  		seps,
  		op,
  		seps,
  		t
  	> {};

  struct instruction_cmp:
  	pegtl::seq<
  		var,
  		seps,
  		arrow,
  		seps,
  		t,
  		seps,
  		cmp,
  		seps,
  		t
  	> {};

  struct instruction_load:
  	pegtl::seq<
  		var,
  		seps,
  		arrow,
  		seps,
  		TAOCPP_PEGTL_STRING("load"),
  		seps,
  		var
  	> {};

  struct instruction_store:
  	pegtl::seq<
  		TAOCPP_PEGTL_STRING("store"),
  		seps,
  		var,
  		seps,
  		arrow,
  		seps,
  		s
  	> {};

  struct instruction_ret:
  	TAOCPP_PEGTL_STRING("return") {};

  struct instruction_ret_val:
  	pegtl::seq<
  	TAOCPP_PEGTL_STRING("return"),
  	seps,
  	t
  	> {};

  struct instruction_label:
  	label {};

  struct instruction_br:
  	pegtl::seq<
  	TAOCPP_PEGTL_STRING("br"),
  	seps,
  	label
  	> {};

  struct instruction_br_var:
  	pegtl::seq<
  	TAOCPP_PEGTL_STRING("br"),
  	seps,
  	var,
  	seps,
  	label
  	> {};

  struct instruction_call:
  	pegtl::seq<
  	TAOCPP_PEGTL_STRING("call"),
  	seps,
  	callee,
  	seps,
  	pegtl::one<'('>,
  	seps,
  	args,
  	seps,
  	pegtl::one<')'>
  	>{};


  struct instruction_call_var:
  	pegtl::seq<
  	var,
  	seps,
  	arrow,
  	seps,
  	TAOCPP_PEGTL_STRING("call"),
  	seps,
  	callee,
  	seps,
  	pegtl::one<'('>,
  	seps,
  	args,
  	seps,
  	pegtl::one<')'>
  	>{};

  struct instruction:
   	pegtl::sor<
  		pegtl::seq<pegtl::at<instruction_ret_val>, instruction_ret_val>,
  		pegtl::seq<pegtl::at<instruction_call_var>, instruction_call_var>,
  		pegtl::seq<pegtl::at<instruction_call>, instruction_call>,
  		pegtl::seq<pegtl::at<instruction_br_var>, instruction_br_var>,
  		pegtl::seq<pegtl::at<instruction_br>, instruction_br>,
  		pegtl::seq<pegtl::at<instruction_label>, instruction_label>,
  		pegtl::seq<pegtl::at<instruction_ret>, instruction_ret>,
  		pegtl::seq<pegtl::at<instruction_store>, instruction_store>,
  		pegtl::seq<pegtl::at<instruction_load>, instruction_load>,
  		pegtl::seq<pegtl::at<instruction_cmp>, instruction_cmp>,
  		pegtl::seq<pegtl::at<instruction_op>, instruction_op>,
  		pegtl::seq<pegtl::at<instruction_assign>, instruction_assign>
  >{};

  struct instructions:
    pegtl::plus<
      pegtl::seq<
        seps,
        instruction,
        seps
      >
    > {};



  struct function:
  	pegtl::seq<
  		TAOCPP_PEGTL_STRING("define"),
  		seps,
  		function_name,
  		seps,
  		pegtl::one<'('>,
  		seps,
  		vars,
  		seps,
  		pegtl::one<')'>,
  		seps,
  		pegtl::one<'{'>,
  		seps,
  		instructions,
  		seps,
  		pegtl::one<'}'>
  	>{};

  struct functions:
    pegtl::plus<
      seps,
      function,
      seps
    > {};


  struct grammar:
    pegtl::must<
      functions
    > {};

  /*
   * Actions attached to grammar rules.
   */

  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < function_name > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto newF = new Function(in.string());
      p.functions.push_back(newF);
    }
  };

  template<> struct action < label > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto i = new I_label(in.string());
      parsed_registers.push_back(i);
      if (in.string().length()>p.longest_label.length()){
      	p.longest_label = in.string();
      }
    }
  };

  template<> struct action < number > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto i = new I_num(in.string());
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < var > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      for (const auto &v : p.functions.back()->vars_used) {
      	if (v->data == in.string()) {
      		parsed_registers.push_back(v);
      		return;
      	}
      }
      auto i = new I_var(in.string());
      parsed_registers.push_back(i);
      p.functions.back()->vars_used.push_back(i);
    }
  };

  template<> struct action < cmp > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto i = new I_cmp(in.string());
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < op > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto i = new I_op(in.string());
      parsed_registers.push_back(i);
    }
  };

 //  template<> struct action < args > {
 //    template< typename Input >
	// static void apply( const Input & in, Program & p){
	// 	for (auto it = parsed_registers.rbegin(); it != parsed_registers.rend(); ++it) {
	// 		parsed_args.push_back(*it);
	// 	}
 //    parsed_registers = std::vector<Item*>();
 //  }
 //  };

  template<> struct action < vars > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
		std::vector<I_var*> parsed_vars;
		for (auto it = parsed_registers.begin(); it != parsed_registers.end(); ++it) {
			parsed_vars.push_back((I_var*)*it);
		}
    parsed_registers = std::vector<Item*>();
    p.functions.back()->args = parsed_vars;
  }
  };

  template<> struct action < runtime_f > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto i = new I_runtime(in.string());
      parsed_registers.push_back(i);  
    }
  };

  template<> struct action < instruction_assign > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
			auto src = parsed_registers.back();
			parsed_registers.pop_back();
			auto dst = (I_var*) parsed_registers.back();
			parsed_registers.pop_back();
      auto i = new Instruction_assign(dst, src);
      p.functions.back()->instructions.push_back(i);
    }
  };

  template<> struct action < instruction_ret_val > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
			auto t = parsed_registers.back();
			parsed_registers.pop_back();
      auto i = new Instruction_ret_val(t);
      p.functions.back()->instructions.push_back(i);
    }
  };

  template<> struct action < instruction_op > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
			auto t2 = parsed_registers.back();
			parsed_registers.pop_back();
			auto op = (I_op*) parsed_registers.back();
			parsed_registers.pop_back();			
			auto t1 = parsed_registers.back();
			parsed_registers.pop_back();
			auto dst = (I_var*) parsed_registers.back();
			parsed_registers.pop_back();      
			auto i = new Instruction_op(dst, t1, op, t2);
      p.functions.back()->instructions.push_back(i);
    }
  };

  template<> struct action < instruction_cmp > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
			auto t2 = parsed_registers.back();
			parsed_registers.pop_back();
			auto cmp = (I_cmp*) parsed_registers.back();
			parsed_registers.pop_back();			
			auto t1 = parsed_registers.back();
			parsed_registers.pop_back();
			auto dst = (I_var*) parsed_registers.back();
			parsed_registers.pop_back();      
			auto i = new Instruction_cmp(dst, t1, cmp, t2);
      p.functions.back()->instructions.push_back(i);
    }
  };

  template<> struct action < instruction_load > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
			auto src = (I_var*) parsed_registers.back();
			parsed_registers.pop_back();
			auto dst = (I_var*) parsed_registers.back();
			parsed_registers.pop_back();
      auto i = new Instruction_load(dst, src);
      p.functions.back()->instructions.push_back(i);
    }
  };

  template<> struct action < instruction_store > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
			auto src = parsed_registers.back();
			parsed_registers.pop_back();
			auto dst = (I_var*) parsed_registers.back();
			parsed_registers.pop_back();
      auto i = new Instruction_store(dst, src);
      p.functions.back()->instructions.push_back(i);
    }
  };

  template<> struct action < instruction_ret > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
		auto i = new Instruction_ret();
		p.functions.back()->instructions.push_back(i);
	}
  };

  template<> struct action < instruction_label > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
		auto label = new I_label(in.string());
		auto i = new Instruction_label(label);
		p.functions.back()->instructions.push_back(i);
	}
  };

  template<> struct action < instruction_br > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
		auto label = (I_label*) parsed_registers.back();
		parsed_registers.pop_back();
		auto i = new Instruction_br(label);
		p.functions.back()->instructions.push_back(i);
	}
  };

  template<> struct action < instruction_br_var > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
		auto label = (I_label*) parsed_registers.back();
		parsed_registers.pop_back();
		auto var = (I_var*) parsed_registers.back();
		parsed_registers.pop_back();		
		auto i = new Instruction_br_var(var, label);
		p.functions.back()->instructions.push_back(i);
	}
  };

  template<> struct action < instruction_call > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
		auto callee = parsed_registers.at(0);
		std::vector<Item*> parsed_args;
		for (auto it = parsed_registers.begin()+1; it != parsed_registers.end(); ++it) {
			parsed_args.push_back(*it);
		}
		parsed_registers = std::vector<Item*>();
		auto i = new Instruction_call(callee, parsed_args);
		p.functions.back()->instructions.push_back(i);
	}
  };

  template<> struct action < instruction_call_var > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
		auto callee = parsed_registers.at(1);
		auto dst = (I_var*)parsed_registers.at(0);
		std::vector<Item*> parsed_args;
		for (auto it = parsed_registers.begin()+2; it != parsed_registers.end(); ++it) {
			parsed_args.push_back(*it);
		}
		parsed_registers = std::vector<Item*>();		
		auto i = new Instruction_call_var(dst, callee, parsed_args);
		p.functions.back()->instructions.push_back(i);
	}
  };

  Program parse_file (char *fileName){

    /*
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< grammar >();

    /*
     * Parse.
     */
    auto i = I_var("abc");
    file_input< > fileInput(fileName);
    Program p;
    parse< grammar, action >(fileInput, p);

    return p;
  }
}






