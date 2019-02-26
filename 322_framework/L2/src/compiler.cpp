#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <iostream>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
#include <iostream>

#include <parser.h>
#include <code_generator.h>
#include <ig.h>
#include <L2.h>
#include <utils.h>

using namespace std;

void print_help (char *progName){
  std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] [-s] [-l 1|2] [-i] SOURCE" << std::endl;
  return ;
}

int main(
  int argc, 
  char **argv
  ){
  auto enable_code_generator = true;
  auto spill_only = false;
  auto interference_only = false;
  auto verbose = false;
  int32_t liveness_only = 0;
  int32_t optLevel = 0;

  /* 
   * Check the compiler arguments.
   */
  if( argc < 2 ) {
    print_help(argv[0]);
    return 1;
  }
  int32_t opt;
  while ((opt = getopt(argc, argv, "vg:O:sl:i")) != -1) {
    switch (opt){

      case 'l':
        liveness_only = strtoul(optarg, NULL, 0);
        break ;

      case 'i':
        interference_only = true;
        break ;

      case 's':
        spill_only = true;
        break ;

      case 'O':
        optLevel = strtoul(optarg, NULL, 0);
        break ;

      case 'g':
        enable_code_generator = (strtoul(optarg, NULL, 0) == 0) ? false : true ;
        break ;

      case 'v':
        verbose = true;
        break ;

      default:
        print_help(argv[0]);
        return 1;
    }
  }



  /*
   * Parse the input file.
   */
  if (spill_only){

    /* 
     * Parse an L2 function and the spill arguments.
     */
     //TODO
 
  } else if (liveness_only){

    /*
     * Parse an L2 function.
     */
    //TODO

  } else if (interference_only){

    /*
     * Parse an L2 function.
     */
     //TODO

  } else {

    /* 
     * Parse the L2 program.
     */
    //TODO
  }

  /*
   * Special cases.
   */
  if (spill_only){
    auto p = L2::parse_spill_test(argv[optind]);
    auto f = (L2::Spill_function*) p.functions.back();
    spill(f, f->var, f->prefix);
    cout << '(' << f->name << endl;
    cout << '\t' << f->arguments << ' ' << f->locals << endl;

    for (auto& i : f->instructions) {
      cout << '\t' << i->print_L2() << endl;
    }
    cout << ')' << endl;
    return 0;
  }

  /*
   * Liveness test.
   */
  if (liveness_only){
    auto p = L2::parse_function(argv[optind]);
    auto f = p.functions.back();
    L2::generate_in_out(f);
    cout << "(" << endl;
    cout << "(in" << endl;
    for (auto& i : f->instructions) {
      cout << "(";
      for (auto& s : i->in) {
        cout << L2::remove_percentage(s) << " ";
      }
      cout << ")\n";
    }
    cout << ")\n\n";
    cout << "(out" << endl;
    for (auto& i : f->instructions) {
      cout << "(";
      for (auto& s : i->out) {
        cout << L2::remove_percentage(s) << " ";
      }
      cout << ")\n";
    }
    cout << ")\n\n)\n";
  }

  /*
   * Interference graph test.
   */
  if (interference_only){
    auto p = L2::parse_function(argv[optind]);
    auto f = p.functions.back();
    L2::generate_in_out(f);
    auto ig = L2::generate_interference_graph(f);
    ig.print();
    //testing only/////
    L2::color_graph(f, ig);
    ///////////////////
    return 0;
  }

  /*
   * Generate the target code.
   */
  if (enable_code_generator){
    auto p = L2::parse_file(argv[optind]);
    generate_code(p);
  }
  return 0;
}
