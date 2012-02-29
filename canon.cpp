#include <vector>
#include <map>
#include <gecode/search.hh>
#include <gecode/gist.hh>
#include <cprel/cprel.hh>
#include <transformer.hh>

using namespace std;
using namespace Gecode;
using namespace MPG;
using namespace MPG::CPRel;


class MidiAsRelation : public MidiTransformer {
private:
  MPG::GRelation midiRel_;
  // maps pitch -> startTime
  std::map<int,int> startTime;
public:
  MidiAsRelation(const char* fname)
  : MidiTransformer(fname), midiRel_(4)
  {}
  virtual
  void processChannelMessage(int track, unsigned long time, int /*channel*/,
                             bool noteOn, int note, int /*velocity*/) override {
    int tm = time;

/*
    cout << "Processed message: " << time << ";" << channel << ";" << noteOn
    << ";" << note << ";" << velocity << endl;
    */
    if (noteOn) {
      assert(startTime.count(note) == 0);
      startTime.insert(std::make_tuple(note,tm));
    } else {
      assert(startTime.count(note) == 1);
      int start = startTime.at(note);
      // the relation is <pitch,onset,duration,track>
      //MPG::Tuple t(note,start % 499,tm - start,track);
      // the relation is <track,duration,pitch,onset>
      MPG::Tuple t(track,tm - start,note,start % 499);

      midiRel_.add(t);
      startTime.erase(note);
    }
  }
  MPG::GRelation getRelation(void) {
    return midiRel_;
  }
};
GRelation makeSumRelation(void) {
  GRelation sum(3);
  for (int i = 0; i < 84; ++i)
    for (int j = 0; j < 84; ++j)
      sum.add(Tuple(i,j,i+j));
  return sum;
}

class Canon : public Gecode::Space {
protected:
  // Initial score
  CPRelVar v0;
  // Voice 1
  CPRelVar v1;
  // Resultant score
  CPRelVar S;
  CPRelVar offset;
public:
  Canon(void) = delete;
  /// Constructor from an input score
  Canon(const char* inputScore)  {
    {
      // The lower bound is the relation representing the input score
      MidiAsRelation reader(inputScore);
      reader.transform();
      GRelation lb = reader.getRelation();
      // The upper bound is the full relation
      GRelation ub = GRelation::create_full(4);

      // V0
      v0 = CPRelVar(*this,lb,ub);
    }
    {
      GRelation lb(1);
      lb.add(Tuple(2));
      GRelation ub = GRelation::create_full(1);
      // Offset
      offset = CPRelVar(*this,lb,ub);
    }
    {
      GRelation lb(4);
      GRelation ub = GRelation::create_full(4);
      // V1
      v1 = CPRelVar(*this,lb,ub);
    }
    {
      S = CPRelVar(*this,GRelation(4),GRelation::create_full(4));
    }
    GRelation plus = makeSumRelation();
    {
      //GRelation x = v0.glb().join(0,offset.glb()).follow(2,plus);
      //cout << x << endl;
    }
    CPRelVar times(*this,GRelation(5),GRelation::create_full(5));
    join(*this,v0,0,offset,times);
    CPRelVar plusVar(*this,plus,plus);
    follow(*this,times,2,plusVar,v1);
    Union(*this,v0,v1,S);
    //CPRelVar pp(*this,GRelation(5),GRelation::create_full(5));
    branch(*this,v0);
  }
  virtual void constrain(const Gecode::Space&) {

  }
  void print(std::ostream& os) const {
    os << std::endl << std::endl 
       << "Relation: " << std::endl
       << S.glb() << std::endl;
    // some other test
    
  }
  Canon(bool share, Canon& s)
    : Gecode::Space(share,s) {
    v0.update(*this, share, s.v0);
    v1.update(*this, share, s.v1);
    S.update(*this, share, s.S);
    offset.update(*this,share,s.offset);
  }
  virtual Space* copy(bool share) {
    return new Canon(share,*this);
  }
};

int main(int, char** argv) {
  Canon* g = new Canon(argv[1]);
  g->status();
  g->print(cout);
  delete g;
/*
  Gecode::DFS<Canon> e(g);
  
  std::cout << "Search will start" << std::endl;
  while (Gecode::Space* s = e.next()) {
    static_cast<Canon*>(s)->print(std::cout);
    delete s;
  }
*/
  /*
  Gist::Print<MinimalTest> p("Print solution");
  Gist::Options o;
  o.inspect.click(&p);
  Gist::dfs(g,o);
  delete g;
  */
  std::cout << "Finishing " << std::endl;
  return 0;
}
