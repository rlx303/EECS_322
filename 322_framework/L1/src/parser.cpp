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
  std::vector<Item> parsed_registers;

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

  /*
   * Keywords.
   */

  struct sx : TAOCPP_PEGTL_STRING( "rcx" ) {};

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

  struct dst_w :
    w {};

  struct x :
    pegtl::sor<
      w,
      TAOCPP_PEGTL_STRING( "rsp" )
    > {};

  struct t :
    pegtl::sor<
      x,
      number
    > {};

  struct s :
    pegtl::sor<
      t,
      label
    >{};

  struct u :
    pegtl::sor<
      w,
      label
    > {};

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

  struct E :
    pegtl::sor<
      pegtl::one< '0' >,
      pegtl::one< '2' >,
      pegtl::one< '4' >,
      pegtl::one< '8' >
    > {};

  struct M:
    number {};

  struct seps:
    pegtl::star<
      pegtl::sor<
        pegtl::ascii::space,
        comment
      >
    > {};

  struct arrow: 
    TAOCPP_PEGTL_STRING("<-")
    {};

  struct memxM:
    pegtl::seq<
      TAOCPP_PEGTL_STRING("mem"),
      seps,
      x,
      seps,
      M
    >{};

  struct dst_memxM:
    memxM {};

  struct str_return : TAOCPP_PEGTL_STRING( "return" ) {};

  struct Label_rule:
    label {};

  struct Instruction_return_rule:
    pegtl::seq<
      str_return
    > { };

  struct s2w_assign_rule:
    pegtl::seq<
      dst_w,
      seps,
      arrow,
      seps,
      s
    >{};

  struct mem2w_assign_rule:
    pegtl::seq<
      dst_w,
      seps,
      arrow,
      seps,
      memxM
    >{};

  struct s2mem_assign_rule:
    pegtl::seq<
      dst_memxM,
      seps,
      arrow,
      seps,
      s
    >{};

  struct t2w_aop_rule:
    pegtl::seq<
      dst_w,
      seps,
      aop,
      seps,
      t
    >{};

  struct sx2w_sop_rule:
    pegtl::seq<
      dst_w,
      seps,
      sop,
      seps,
      sx
    >{};

  struct N2w_sop_rule:
    pegtl::seq<
      dst_w,
      seps,
      sop,
      seps,
      number
    >{};

  struct t2mem_aop_rule:
    pegtl::seq<
      dst_memxM,
      seps,
      plus_minus,
      seps,
      t
    >{};

  struct mem2w_aop_rule:
    pegtl::seq<
      dst_w,
      seps,
      plus_minus,
      seps,
      memxM
    >{};

  struct assign_rules: 
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
      pegtl::seq<pegtl::at<assign_rules>, assign_rules>
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

  template<> struct action < M > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      if (std::stoi(in.string()) % 8 != 0) {
        abort();
      }
    }
  };

  template<> struct action < arrow > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->op = in.string();
    }
  };

  template<> struct action < aop > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->op = in.string();
    }
  };

  template<> struct action < plus_minus > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->op = in.string();
    }
  };

  template<> struct action < sop > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->op = in.string();
    }
  };



  template<> struct action < s > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->src = in.string();
    }
  };

  // template<> struct action < w > {
  //   template< typename Input >
  // static void apply( const Input & in, Program & p){
  //     auto currentF = p.functions.back();
  //     currentF->w = in.string();
  //   }
  // };

  template<> struct action < dst_w > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->dst = in.string();
    }
  };

  template<> struct action < dst_memxM > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->dst = in.string();
    }
  };

  template<> struct action < t > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->src = in.string();
    }
  };

  template<> struct action < memxM > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->src = in.string();
    }
  };

  template<> struct action < sx > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->src = in.string();
    }
  };

  template<> struct action < number > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->src = in.string();
    }
  };

  template<> struct action < label > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      if (p.entryPointLabel.empty()){
        p.entryPointLabel = in.string();
      } else {
      }
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

  // template<> struct action < s2w_assign_rule > {
  //   template< typename Input >
  // static void apply( const Input & in, Program & p){
  //     auto currentF = p.functions.back();
  //     auto i = new Instruction_assign();
  //     i->dst = currentF->dst;
  //     i->src = currentF->s;
  //     i->op = currentF->op;
  //     currentF->instructions.push_back(i);
  //   }
  // };

  // template<> struct action < mem2w_assign_rule > {
  //   template< typename Input >
  // static void apply( const Input & in, Program & p){
  //     auto currentF = p.functions.back();
  //     auto i = new Instruction_assign();
  //     i->dst = currentF->dst;
  //     i->src = currentF->memxM;
  //     i->op = currentF->op;
  //     currentF->instructions.push_back(i);
  //   }
  // };

  // template<> struct action < s2mem_assign_rule > {
  //   template< typename Input >
  // static void apply( const Input & in, Program & p){
  //     auto currentF = p.functions.back();
  //     auto i = new Instruction_assign();
  //     i->dst = currentF->dst;
  //     i->src = currentF->s;
  //     i->op = currentF->op;
  //     currentF->instructions.push_back(i);
  //   }
  // };

  // template<> struct action < t2w_aop_rule > {
  //   template< typename Input >
  // static void apply( const Input & in, Program & p){
  //     auto currentF = p.functions.back();
  //     auto i = new Instruction_assign();
  //     i->dst = currentF->dst;
  //     i->src = currentF->t;
  //     i->op = currentF->op;
  //     currentF->instructions.push_back(i);
  //   }
  // };


  template<> struct action < assign_rules > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_assign();
      i->dst = currentF->dst;
      i->src = currentF->src;
      i->op = currentF->op;
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Label_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      Item i;
      i.labelName = in.string();
      parsed_registers.push_back(i);
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
