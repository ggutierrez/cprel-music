#include <iostream>
#include <fstream>
#include <tools/csvreader.hh>

int main(int argc, char* argv[]) {
  std::cout << "Executing main " <<  std::endl;
  // assumes the midi file name is given as the first argument in the
  // command line
  if (argc == 2) {
    std::ifstream is(argv[1]);
    auto rel = CPRelTools::createFromCSV(is,6);
   
    for (int i = 0; i < 6; i++) {
      auto qrel = rel.unique(i);
      //std::cout << "RelationText " << qrel << std::endl;
    }
    /*
    //rel.remove(qrel);
    std::cout << "Rel " << std::endl << qrel.cardinality()  << std::endl;
    std::cout << "Rel  " << qrel << std::endl;
    */ 
 } else {
    std::cout << "Not called properly " <<  std::endl;
  }
  return 0;
}
