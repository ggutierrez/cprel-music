#ifndef __CPRELTOOLS_CSVREADER_HH__
#define __CPRELTOOLS_CSVREADER_HH__

#include <fstream>
#include <bdddomain/rel-impl.hh>

namespace CPRelTools {
  /**
   * \brief Takes the information in \a is and creates a ground relation out of it. 
   */
  MPG::VarImpl::RelationImpl 
  createFromCSV(std::istream& is, int columns);
}

#endif
