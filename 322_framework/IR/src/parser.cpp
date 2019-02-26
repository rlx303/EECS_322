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

#include <IR.h>
#include <parser.h>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace IR {
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

  struct type:
    pegtl::sor<
      pegtl::seq<
        TAOCPP_PEGTL_STRING("int64"),
        pegtl::star<
          TAOCPP_PEGTL_STRING("[]")
        >
      >,
      TAOCPP_PEGTL_STRING("tuple"),
      TAOCPP_PEGTL_STRING("code")
    > {};

  struct T:
    pegtl::sor<
      type,
      TAOCPP_PEGTL_STRING("void")
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

  struct array_accessor:
    pegtl::seq<
      seps,
      pegtl::one<'['>,
      seps,
      t,
      seps,
      pegtl::one<']'>,
      seps
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

  struct type_var:
    pegtl::seq<
      type,
      seps,
      var
    >{};

  struct type_vars:
    pegtl::sor<
      pegtl::seq<
        type_var,
        seps,
        pegtl::star<
          seps,
          pegtl::one<','>,
          seps,
          type_var,
          seps>
      >,
      pegtl::seq<
        seps,
        type_var,
        seps
      >, 
      seps
    >{};

  /**
  *
  *
  Instructions:
	*
	*
  **/

  struct instruction_read_array:
    pegtl::seq<
      var,
      seps,
      arrow,
      seps,
      var,
      seps,
      pegtl::plus<
        array_accessor
      >
    > {};

  struct instruction_write_array:
    pegtl::seq<
      var,
      pegtl::plus<
        array_accessor
      >,
      seps,
      arrow,
      seps,
      s
    > {};

  struct instruction_length:
    pegtl::seq<
      var,
      seps,
      arrow,
      seps,
      TAOCPP_PEGTL_STRING("length"),
      seps,
      var,
      seps,
      t
    > {};

  struct instruction_def:
    pegtl::seq<
      type_var
    > {};


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
  	label,
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

  struct instruction_new_array:
    pegtl::seq<
      var,
      seps,
      arrow,
      seps,
      TAOCPP_PEGTL_STRING("new"),
      seps,
      TAOCPP_PEGTL_STRING("Array"),
      seps,
      pegtl::one<'('>,
      seps,
      args,
      seps,
      pegtl::one<')'>
    >{};

  struct instruction_new_tuple:
    pegtl::seq<
      var,
      seps,
      arrow,
      seps,
      TAOCPP_PEGTL_STRING("new"),
      seps,
      TAOCPP_PEGTL_STRING("Tuple"),
      seps,
      pegtl::one<'('>,
      seps,
      t,
      seps,
      pegtl::one<')'>
    >{};

  struct instruction:
   	pegtl::sor<
      pegtl::seq<pegtl::at<instruction_read_array>, instruction_read_array>,
      pegtl::seq<pegtl::at<instruction_label>, instruction_label>,
  		pegtl::seq<pegtl::at<instruction_call_var>, instruction_call_var>,
  		pegtl::seq<pegtl::at<instruction_call>, instruction_call>,
  		pegtl::seq<pegtl::at<instruction_cmp>, instruction_cmp>,
  		pegtl::seq<pegtl::at<instruction_op>, instruction_op>,
  		pegtl::seq<pegtl::at<instruction_assign>, instruction_assign>,
      pegtl::seq<pegtl::at<instruction_write_array>, instruction_write_array>,
      pegtl::seq<pegtl::at<instruction_length>, instruction_length>,
      pegtl::seq<pegtl::at<instruction_new_array>, instruction_new_array>,
      pegtl::seq<pegtl::at<instruction_new_tuple>, instruction_new_tuple>,
      pegtl::seq<pegtl::at<instruction_def>, instruction_def>
  >{};

  struct instructions:
    pegtl::star<
      pegtl::seq<
        seps,
        instruction,
        seps
      >
    > {};

  struct te: 
    pegtl::sor<
      instruction_br_var,
      instruction_br,
      instruction_ret_val,
      instruction_ret
    > {};

  struct bb:
    pegtl::seq<
      instruction_label,
      seps,
      instructions,
      seps,
      te
    > {};

  struct bbs:
    pegtl::plus<
      seps,
      bb,
      seps
    > {};

  struct function:
  	pegtl::seq<
  		TAOCPP_PEGTL_STRING("define"),
  		seps,
  		T,
  		seps,
  		function_name,
      seps,
      pegtl::one<'('>,
      seps,
      type_vars,
      seps,
      pegtl::one<')'>,
      seps,
      pegtl::one<'{'>,
      seps,
      bbs,
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

  template<> struct action < type_var > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto input = in.string();
      if (input.substr(0, input.find(' ')) == "tuple") {
        parsed_registers.back()->be_tuple();
      }
    }
  };

  template<> struct action < type_vars > {
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

  template<> struct action < runtime_f > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto i = new I_runtime(in.string());
      parsed_registers.push_back(i);  
    }
  };

  template<> struct action < instruction_def > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      parsed_registers.pop_back();
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
		auto label2 = (I_label*) parsed_registers.back();
		parsed_registers.pop_back();
    auto label1 = (I_label*) parsed_registers.back();
    parsed_registers.pop_back();		
    auto var = (I_var*) parsed_registers.back();
		parsed_registers.pop_back();		
		auto i = new Instruction_br_var(var, label1, label2);
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

  template<> struct action < instruction_read_array > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto dst = (I_var*)parsed_registers.at(0);
      auto array = (I_var*)parsed_registers.at(1);
      std::vector<Item*> parsed_inds;
      for (auto it = parsed_registers.begin()+2; it != parsed_registers.end(); ++it) {
        parsed_inds.push_back(*it);
      }
      parsed_registers = std::vector<Item*>();    
      auto i = new Instruction_read_array(dst, array, parsed_inds);
      p.functions.back()->instructions.push_back(i);
    }
  };

  template<> struct action < instruction_write_array > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto dst = (I_var*)parsed_registers.at(0);
      auto src = parsed_registers.back();
      parsed_registers.pop_back();
      std::vector<Item*> parsed_inds;
      for (auto it = parsed_registers.begin()+1; it != parsed_registers.end(); ++it) {
        parsed_inds.push_back(*it);
      }
      parsed_registers = std::vector<Item*>();    
      auto i = new Instruction_write_array(dst, src, parsed_inds);
      p.functions.back()->instructions.push_back(i);
    }
  };

  template<> struct action < instruction_length > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto t = parsed_registers.back();
      parsed_registers.pop_back();
      auto array = (I_var*) parsed_registers.back();
      parsed_registers.pop_back();
      auto dst = (I_var*)parsed_registers.back();
      parsed_registers.pop_back(); 
      auto i = new Instruction_length(dst, array, t);
      p.functions.back()->instructions.push_back(i);
    }
  };

  template<> struct action < instruction_new_array > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto dst = (I_var*)parsed_registers.at(0);
      std::vector<Item*> parsed_args;
      for (auto it = parsed_registers.begin()+1; it != parsed_registers.end(); ++it) {
        parsed_args.push_back(*it);
      }
      parsed_registers = std::vector<Item*>();
      auto i = new Instruction_new_array(dst, parsed_args);
      p.functions.back()->instructions.push_back(i);
    }
  };

  template<> struct action < instruction_new_tuple > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto t = parsed_registers.back();
      parsed_registers.pop_back();
      auto dst = (I_var*)parsed_registers.back();
      parsed_registers.pop_back();
      auto i = new Instruction_new_tuple(dst, t);
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
    file_input< > fileInput(fileName);
    Program p;
    parse< grammar, action >(fileInput, p);

    return p;
  }
}






