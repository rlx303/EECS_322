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
      pegtl::one< '1' >,
      pegtl::one< '2' >,
      pegtl::one< '4' >,
      pegtl::one< '8' >
    > {};

  struct M:
    number {};

  struct sx : TAOCPP_PEGTL_STRING( "rcx" ) {};

  struct sx_rule : 
    sx {};

  struct sx_shift : 
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
      w_rule,
      Label_rule
    > {};

  struct arrow: 
    TAOCPP_PEGTL_STRING("<-")
    {};

  struct plus_minus :
    pegtl::sor<
      TAOCPP_PEGTL_STRING( "+=" ),
      TAOCPP_PEGTL_STRING( "-=" )
    > {};

  struct plus_minus_rule :
    plus_minus {};

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
      TAOCPP_PEGTL_STRING( "<=" ),
      pegtl::one< '<' >,
      pegtl::one< '=' >
    > {};

  struct inc_dec:
    pegtl::sor<
      TAOCPP_PEGTL_STRING( "++" ),
      TAOCPP_PEGTL_STRING( "--" )
    >{};

  struct seps:
    pegtl::star<
      pegtl::sor<
        pegtl::ascii::space,
        comment
      >
    > {};

  struct space:
    pegtl::star<
      internal::one< 
        internal::result_on_found::SUCCESS, 
        internal::peek_char, 
        ' ', 
        '\t' 
      >
    >{};


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

  struct instruction_label_rule:
    label {};

  struct instruction_return_rule:
    pegtl::seq<
      str_return
    > {};

  struct instruction_goto_rule:
    pegtl::seq<
      TAOCPP_PEGTL_STRING("goto"),
      seps, 
      Label_rule
    > {};

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
      sx_shift
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
      plus_minus_rule,
      seps,
      t
    >{};

  struct mem2w_aop_rule:
    pegtl::seq<
      w_rule,
      seps,
      plus_minus_rule,
      seps,
      memxM
    >{};

  struct inc_dec_rule:
    pegtl::seq<
      w_rule,
      seps,
      inc_dec
    >{};

  struct two_operand_rule: 
    pegtl::sor<
      pegtl::seq<pegtl::at<t2w_aop_rule>, t2w_aop_rule>,
      pegtl::seq<pegtl::at<s2w_assign_rule>, s2w_assign_rule>,
      pegtl::seq<pegtl::at<mem2w_assign_rule>, mem2w_assign_rule>,
      pegtl::seq<pegtl::at<s2mem_assign_rule>, s2mem_assign_rule>,
      pegtl::seq<pegtl::at<sx2w_sop_rule>, sx2w_sop_rule>,
      pegtl::seq<pegtl::at<N2w_sop_rule>, N2w_sop_rule>,
      pegtl::seq<pegtl::at<t2mem_aop_rule>, t2mem_aop_rule>,
      pegtl::seq<pegtl::at<mem2w_aop_rule>, mem2w_aop_rule>
    > {};

  struct cmp_rule:
    pegtl::seq<
      w_rule,
      seps,
      arrow,
      seps,
      t,
      seps,
      cmp,
      seps,
      t
    >{};

  struct two_label_jump:
    pegtl::seq<
      TAOCPP_PEGTL_STRING("cjump"),
      seps,
      t,
      seps,
      cmp,
      seps,
      t,
      seps,
      Label_rule,
      space,
      Label_rule
    >{};

  struct one_label_jump:
    pegtl::seq<
      TAOCPP_PEGTL_STRING("cjump"),
      seps,
      t,
      seps,
      cmp,
      seps,
      t,
      seps,
      Label_rule
    >{};

  struct wwe_rule:
    pegtl::seq<
      w_rule,
      seps,
      pegtl::one<'@'>,
      seps,
      w_rule,
      seps,
      w_rule,
      seps,
      E
    >{};

  struct call_rule:
    pegtl::seq<
      TAOCPP_PEGTL_STRING("call"), 
      seps, 
      u, 
      seps,
      number_rule
    >{};

  struct call_print:
    pegtl::seq<
      TAOCPP_PEGTL_STRING("call"), 
      seps, 
      TAOCPP_PEGTL_STRING("print"), 
      seps,
      pegtl::one<'1'>
    >{};

  struct call_allocate:
    pegtl::seq<
      TAOCPP_PEGTL_STRING("call"), 
      seps, 
      TAOCPP_PEGTL_STRING("allocate"), 
      seps,
      pegtl::one<'2'>
    >{};

  struct call_array_error:
    pegtl::seq<
      TAOCPP_PEGTL_STRING("call"), 
      seps, 
      TAOCPP_PEGTL_STRING("array-error"), 
      seps,
      pegtl::one<'2'>
    >{};

  struct call_instructions:
    pegtl::sor<
      pegtl::seq<pegtl::at<call_rule>, call_rule>,
      pegtl::seq<pegtl::at<call_print>, call_print>,
      pegtl::seq<pegtl::at<call_allocate>, call_allocate>,
      pegtl::seq<pegtl::at<call_array_error>, call_array_error>
    >{};

  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq<pegtl::at<call_instructions>, call_instructions>,
      pegtl::seq<pegtl::at<two_label_jump>, two_label_jump>,
      pegtl::seq<pegtl::at<one_label_jump>, one_label_jump>,
      pegtl::seq<pegtl::at<cmp_rule>, cmp_rule>,
      pegtl::seq<pegtl::at<instruction_return_rule>, instruction_return_rule>,
      pegtl::seq<pegtl::at<two_operand_rule>, two_operand_rule>,
      pegtl::seq<pegtl::at<inc_dec_rule>, inc_dec_rule>,
      pegtl::seq<pegtl::at<instruction_label_rule>, instruction_label_rule>,
      pegtl::seq<pegtl::at<instruction_goto_rule>, instruction_goto_rule>,
      pegtl::seq<pegtl::at<wwe_rule>, wwe_rule>
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

  template<> struct action < plus_minus_rule > {
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

  template<> struct action < inc_dec > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new I_inc_dec(in.string());
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < cmp > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new I_cmp(in.string());
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

  template<> struct action < sx_shift > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new I_sx(in.string());
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

  template<> struct action < E > {
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

  template<> struct action < instruction_return_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_ret();
      i->arguments = currentF->arguments;
      i->locals = currentF->locals;
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < instruction_label_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_label();
      i->label = new I_label(in.string());
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < instruction_goto_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_goto();
      i->label = (I_label *)parsed_registers.back();
      parsed_registers.pop_back();
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < two_operand_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto ins = new Instruction_assign();
      ins->src = parsed_registers.back();
      parsed_registers.pop_back();
      ins->op = parsed_registers.back();
      parsed_registers.pop_back();
      ins->dst = parsed_registers.back();
      parsed_registers.pop_back();
      currentF->instructions.push_back(ins);
    }
  };

  template<> struct action < inc_dec_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto ins = new Instruction_inc_dec();
      ins->op = parsed_registers.back();
      parsed_registers.pop_back();
      ins->dst = parsed_registers.back();
      parsed_registers.pop_back();
      currentF->instructions.push_back(ins);
    }
  };

  template<> struct action < cmp_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto ins = new Instruction_cmp();
      ins->t2 = parsed_registers.back();
      parsed_registers.pop_back();
      ins->cmp = parsed_registers.back();
      parsed_registers.pop_back();
      ins->t1 = parsed_registers.back();
      parsed_registers.pop_back();
      ins->op = parsed_registers.back();
      parsed_registers.pop_back();
      ins->dst = parsed_registers.back();
      parsed_registers.pop_back();
      currentF->instructions.push_back(ins);
    }
  };

  template<> struct action < two_label_jump > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto ins = new Instruction_two_label_jump();
      ins->label2 = parsed_registers.back();
      parsed_registers.pop_back();
      ins->label1 = parsed_registers.back();
      parsed_registers.pop_back();
      ins->t2 = parsed_registers.back();
      parsed_registers.pop_back();
      ins->cmp = parsed_registers.back();
      parsed_registers.pop_back();
      ins->t1 = parsed_registers.back();
      parsed_registers.pop_back();
      currentF->instructions.push_back(ins);
    }
  };

  template<> struct action < one_label_jump > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto ins = new Instruction_one_label_jump();
      ins->label1 = parsed_registers.back();
      parsed_registers.pop_back();
      ins->t2 = parsed_registers.back();
      parsed_registers.pop_back();
      ins->cmp = parsed_registers.back();
      parsed_registers.pop_back();
      ins->t1 = parsed_registers.back();
      parsed_registers.pop_back();
      currentF->instructions.push_back(ins);
    }
  };

  template<> struct action < wwe_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto ins = new Instruction_wwe();
      ins->E = parsed_registers.back();
      parsed_registers.pop_back();
      ins->w2 = parsed_registers.back();
      parsed_registers.pop_back();
      ins->w1 = parsed_registers.back();
      parsed_registers.pop_back();
      ins->dst = parsed_registers.back();
      parsed_registers.pop_back();
      currentF->instructions.push_back(ins);
    }
  };

  template<> struct action < call_print > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto ins = new Instruction_runtime();
      ins->name = "print";
      ins->arg_num = "1";
      currentF->instructions.push_back(ins);
    }
  };

  template<> struct action < call_allocate > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto ins = new Instruction_runtime();
      ins->name = "allocate";
      ins->arg_num = "2";
      currentF->instructions.push_back(ins);
    }
  };

  template<> struct action < call_array_error > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto ins = new Instruction_runtime();
      ins->name = "array-error";
      ins->arg_num = "2";
      currentF->instructions.push_back(ins);
    }
  };

  template<> struct action < call_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto ins = new Instruction_call();
      ins->arg_num = parsed_registers.back();
      parsed_registers.pop_back();
      ins->label = parsed_registers.back();
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
