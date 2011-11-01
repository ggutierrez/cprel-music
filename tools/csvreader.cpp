#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <tools/csvreader.hh>

namespace CPRelTools {
  
  MPG::VarImpl::RelationImpl
  createFromCSV(std::istream& is, int columns) {
    MPG::VarImpl::RelationImpl r(columns);
    std::string line;
    std::vector<int> tuple(columns,-1);
    while (is.good()) {
      std::getline(is,line);
      std::stringstream ss(line);
      int i = 0;
      while (ss.good()) {
	ss >> tuple[i];
	i++;
      }
      MPG::Tuple t(tuple);
      r.add(t);
    }
    return r;
  }
}

