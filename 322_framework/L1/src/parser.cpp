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

#include <L1.h>
#include <parser.h>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace L1 {

  /*
   * Data required to parse
   */
  std::vector<Item*> parsed_registers;

  /*
   * Grammar rules from now on.
   */
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

  struct number_rule:
    number {};

  struct function_name:
    label {};

  struct argument_number:
    number {};

  struct local_number:
    number {} ;

  struct comment:
    pegtl::disable<
      TAOCPP_PEGTL_STRING( "//" ),
      pegtl::until< pegtl::eolf >
    > {};

  struct Label_rule:
    label {};

  /*
   * Keywords.
   */


  struct E :
    pegtl::sor<
      pegtl::one< '0' >,
      pegtl::one< '2' >,
      pegtl::one< '4' >,
      pegtl::one< '8' >
    > {};

  struct M:
    number {};

  struct sx : TAOCPP_PEGTL_STRING( "rcx" ) {};

  struct sx_rule : 
    sx {};

  struct a :
    pegtl::sor<
      TAOCPP_PEGTL_STRING( "rdi" ),
      TAOCPP_PEGTL_STRING( "rsi" ),
      TAOCPP_PEGTL_STRING( "rdx" ),
      sx,
      TAOCPP_PEGTL_STRING( "r8" ),
      TAOCPP_PEGTL_STRING( "r9" )
    > {};

  struct w :
    pegtl::sor<
      a,
      TAOCPP_PEGTL_STRING( "rax" ),
      TAOCPP_PEGTL_STRING( "rbx" ),
      TAOCPP_PEGTL_STRING( "rbp" ),
      TAOCPP_PEGTL_STRING( "r10" ),
      TAOCPP_PEGTL_STRING( "r11" ),
      TAOCPP_PEGTL_STRING( "r12" ),
      TAOCPP_PEGTL_STRING( "r13" ),
      TAOCPP_PEGTL_STRING( "r14" ),
      TAOCPP_PEGTL_STRING( "r15" )
    > {};

  struct w_rule :
    w {};

  struct x :
    pegtl::sor<
      w,
      TAOCPP_PEGTL_STRING( "rsp" )
    > {};


  struct x_rule :
    x {};

  struct t :
    pegtl::sor<
      x_rule,
      number_rule
    > {};

  struct t_rule :
    t {};

  struct s :
    pegtl::sor<
      t,
      Label_rule
    >{};

  struct s_rule :
    s {};

  struct u :
    pegtl::sor<
      w,
      Label_rule
    > {};

  struct u_rule :
    u {};


  struct arrow: 
    TAOCPP_PEGTL_STRING("<-")
    {};

  struct plus_minus :
    pegtl::sor<
      TAOCPP_PEGTL_STRING( "+=" ),
      TAOCPP_PEGTL_STRING( "-=" )
    > {};

  struct aop :
    pegtl::sor<
      plus_minus,
      TAOCPP_PEGTL_STRING( "*=" ),
      TAOCPP_PEGTL_STRING( "&=" )
    > {};

  struct aop_rule :
    aop {};

  struct sop :
    pegtl::sor<
      TAOCPP_PEGTL_STRING( "<<=" ),
      TAOCPP_PEGTL_STRING( ">>=" )
    > {};

  struct cmp :
    pegtl::sor<
      pegtl::one< '<' >,
      TAOCPP_PEGTL_STRING( "<=" ),
      pegtl::one< '=' >
    > {};

  struct seps:
    pegtl::star<
      pegtl::sor<
        pegtl::ascii::space,
        comment
      >
    > {};

  struct memxM:
    pegtl::seq<
      TAOCPP_PEGTL_STRING("mem"),
      seps,
      x_rule,
      seps,
      M
    >{};

// ````````````````````````````````````INSTRUCTIONS:

  struct str_return : TAOCPP_PEGTL_STRING( "return" ) {};

  struct Instruction_return_rule:
    pegtl::seq<
      str_return
    > { };

  struct s2w_assign_rule:
    pegtl::seq<
      w_rule,
      seps,
      arrow,
      seps,
      s
    >{};

  struct mem2w_assign_rule:
    pegtl::seq<
      w_rule,
      seps,
      arrow,
      seps,
      memxM
    >{};

  struct s2mem_assign_rule:
    pegtl::seq<
      memxM,
      seps,
      arrow,
      seps,
      s
    >{};

  struct t2w_aop_rule:
    pegtl::seq<
      w_rule,
      seps,
      aop_rule,
      seps,
      t
    >{};

  struct sx2w_sop_rule:
    pegtl::seq<
      w_rule,
      seps,
      sop,
      seps,
      sx_rule
    >{};

  struct N2w_sop_rule:
    pegtl::seq<
      w_rule,
      seps,
      sop,
      seps,
      number_rule
    >{};

  struct t2mem_aop_rule:
    pegtl::seq<
      memxM,
      seps,
      plus_minus,
      seps,
      t
    >{};

  struct mem2w_aop_rule:
    pegtl::seq<
      w_rule,
      seps,
      plus_minus,
      seps,
      memxM
    >{};

  struct two_operand_rule: 
    pegtl::sor<
      pegtl::seq<pegtl::at<s2w_assign_rule>, s2w_assign_rule>,
      pegtl::seq<pegtl::at<mem2w_assign_rule>, mem2w_assign_rule>,
      pegtl::seq<pegtl::at<s2mem_assign_rule>, s2mem_assign_rule>,
      pegtl::seq<pegtl::at<t2w_aop_rule>, t2w_aop_rule>,
      pegtl::seq<pegtl::at<sx2w_sop_rule>, sx2w_sop_rule>,
      pegtl::seq<pegtl::at<N2w_sop_rule>, N2w_sop_rule>,
      pegtl::seq<pegtl::at<t2mem_aop_rule>, t2mem_aop_rule>,
      pegtl::seq<pegtl::at<mem2w_aop_rule>, mem2w_aop_rule>
    > {};

  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<Instruction_return_rule>, Instruction_return_rule>,
      pegtl::seq<pegtl::at<two_operand_rule>, two_operand_rule>
    > { };

  struct Instructions_rule:
    pegtl::plus<
      pegtl::seq<
        seps,
        Instruction_rule,
        seps
      >
    > { };

  struct Function_rule:
    pegtl::seq<
      pegtl::one< '(' >,
      seps,
      function_name,
      seps,
      argument_number,
      seps,
      local_number,
      seps,
      Instructions_rule,
      seps,
      pegtl::one< ')' >
    > {};

  struct Functions_rule:
    pegtl::plus<
      seps,
      Function_rule,
      seps
    > {};

  struct entry_point_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      seps,
      label,
      seps,
      Functions_rule,
      seps,
      pegtl::one< ')' >,
      seps
    > { };

  struct grammar :
    pegtl::must<
      entry_point_rule
    > {};

  /*
   * Actions attached to grammar rules.
   */

  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};


//`````````````````````````` op rules:

  template<> struct action < arrow > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new I_arrow(in.string());
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < aop_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new I_aop(in.string());
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < plus_minus > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new I_aop(in.string());
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < sop > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new I_sop(in.string());
      parsed_registers.push_back(i);
    }
  };



//``````````````````` items rules
  template<> struct action < M > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      if (std::stoi(in.string()) % 8 != 0) {
        abort();
      }
      auto i = new I_num(in.string());
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < x_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new I_reg(in.string());
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < w_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new I_reg(in.string());
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < sx_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new I_reg(in.string());
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < number_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new I_num(in.string());
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < label > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      if (p.entryPointLabel.empty()){
        p.entryPointLabel = in.string();
      } else {
        abort();
      }
    }
  };

  template<> struct action < Label_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new I_label(in.string());
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < memxM > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      std::string M = parsed_registers.back()->data;
      parsed_registers.pop_back();
      std::string x = parsed_registers.back()->data;
      parsed_registers.pop_back();
      auto i = new I_memxM(x, M);
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < function_name > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto newF = new Function();
      newF->name = in.string();
      p.functions.push_back(newF);
    }
  };

  template<> struct action < argument_number > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->arguments = std::stoll(in.string());
    }
  };

  template<> struct action < local_number > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->locals = std::stoll(in.string());
    }
  };

  template<> struct action < str_return > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_ret();
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < two_operand_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto ins = new Instruction_s2w_assign();
      ins->src = parsed_registers.back();
      parsed_registers.pop_back();
      ins->op = parsed_registers.back();
      parsed_registers.pop_back();
      ins->dst = parsed_registers.back();
      parsed_registers.pop_back();
      currentF->instructions.push_back(ins);
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
