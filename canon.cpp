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
  for (int i = 0; i < 200; ++i)
    for (int j = 0; j < 200; ++j)
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
  CPRelVar pp;
  GRelation consonant;

  CPRelVar debug;
public:
  Canon(void) = delete;
  /// Constructor from an input score
  Canon(const char* inputScore, GRelation c) 
  : consonant(c) {
    {
      // The lower bound is the relation representing the input score
      MidiAsRelation reader(inputScore);
      reader.transform();
      GRelation lb = reader.getRelation();
      // The upper bound is the full relation
      GRelation ub = GRelation::create_full(4);

      // V0: <track,duration,pitch,onset>
      v0 = CPRelVar(*this,lb,ub);
    }
    {
      GRelation lb(1);
      lb.add(Tuple(50));

      GRelation ub(1); //= GRelation::create_full(1);
      //ub.add(Tuple(0));
      //ub.add(Tuple(500));
      // Offset: <dt>
      offset = CPRelVar(*this,lb,ub.Union(lb));
    }
    {
      GRelation lb(4);
      GRelation ub = GRelation::create_full(4);
      // V1: <track,duration,pitch,onset>
      v1 = CPRelVar(*this,lb,ub);
    }
    {
      // S: <track,duration,pitch,onset>
      S = CPRelVar(*this,GRelation(4),GRelation::create_full(4));
    }
    // plus: <x,y,z> : x + y = z
    GRelation plus = makeSumRelation();
    {
      //GRelation x = v0.glb().join(0,offset.glb()).follow(2,plus);
      //cout << x << endl;
    }
    {
      // This block of constraints enforces that v1 is a follower voice of v0
      // offeset x times : <track,duration,pitch,onset> x <dt>
      //                -> <track,duration,pitch,onset,dt>
      CPRelVar times(*this,GRelation(5),GRelation::create_full(5));
      join(*this,v0,0,offset,times);
      CPRelVar plusVar(*this,plus,plus);

      CPRelVar tmp(*this,GRelation(4),GRelation::create_full(4));
      follow(*this,times,2,plusVar,tmp);
      subset(*this, v1, tmp);
    }
    {
      // Both voices must be part of the final score
      //Union(*this,v0,v1,S);
      equal(*this,v1,S);
    }
    {
      // The final score must be consonant

      // From S: <track,duration,pitch,onset>, get <pitch,onset>
      CPRelVar pitch_onset(*this,GRelation(2),GRelation::create_full(2));
      projection(*this, 2, S, pitch_onset);

      // From pitch_onset: <pitch,onset> get <onset,pitch>
      CPRelVar onset_pitch(*this,GRelation(2),GRelation::create_full(2));
      PermDescriptor perm_pitch_onset;
      perm_pitch_onset.permute(0, 1);
      permutation(*this, pitch_onset, onset_pitch, perm_pitch_onset);

      // Take all the pitches that occur at the same onset in a relation: <pitch,pitch> 
      // pp : <pitch,pitch>      
      //CPRelVar 
      pp = CPRelVar(*this,GRelation(2),GRelation::create_full(2));
      follow(*this, pitch_onset, 1, onset_pitch, pp);

      // There is a big difference between the voices being consonants and
      // the voices only containing consonant notes
      //CPRelVar nonConsonantV(*this,consonant.complement(),consonant.complement());
      CPRelVar consonantV(*this,consonant,consonant);
      //disjoint(*this,pp,nonConsonantV);
      //subset(*this,pp,consonantV);
      // <track,duration,pitch,onset> -- <onset,pitch> = <track,duration,pitch,pitch>

    }
    //CPRelVar pp(*this,GRelation(5),GRelation::create_full(5));
    branch(*this,offset);
    //branch(*this,v0);
  }
  virtual void constrain(const Gecode::Space&) {

  }
  void print(std::ostream& os) const {
    os << std::endl << std::endl 
       << "Relation: " << std::endl
       << pp.lub() //.intersect(consonant) 
      // << S.lub()
       //<< S.lub().intersect(v0.glb())
       //<< S << std::endl;
    // some other test
    << std::endl;
  }
  Canon(bool share, Canon& s)
    : Gecode::Space(share,s), consonant(s.consonant)
  {
    v0.update(*this, share, s.v0);
    v1.update(*this, share, s.v1);
    S.update(*this, share, s.S);
    offset.update(*this,share,s.offset);
    pp.update(*this,share,s.pp);
    //debug.update(*this,share,s.debug);
  }
  virtual Space* copy(bool share) {
    return new Canon(share,*this);
  }
};

int main(int, char** argv) {
  // create a relation with consonant pitches
  
  GRelation consonant(2);
  consonant.add(Tuple(71,64));
  consonant.add(Tuple(48,56));
  //consonant.add(Tuple(70,52));
  
  //GRelation consonant = GRelation::create_full(2);
  Canon* g = new Canon(argv[1],consonant);

  Gecode::DFS<Canon> e(g);
  Gecode::Space* s = e.next();
  if (s) {
    cout << "A solution!!!" << endl;
    static_cast<Canon*>(s)->print(std::cout);
  }
  delete s;
  delete g;
  
/*  
  Gist::Print<Canon> p("Print solution");
  Gist::Options o;
  o.inspect.click(&p);
  Gist::dfs(g,o);
  //delete g;
  */
  std::cout << "Finishing " << std::endl;
  return 0;
}
