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
  CPRelVar v0_;
  // Voice 1
  CPRelVar v1_;
  // Resultant score
  CPRelVar S_;
  CPRelVar offset_;
  //CPRelVar pp;
  GRelation cons_;

  //  CPRelVar debug_;
  /**
   * \brief Given two variables \a v and \a f posts the constraint that
   * f is a follower voice of v by an onset offset \a o
   */
  void followVoice(CPRelVar v, CPRelVar f, CPRelVar o) {
    // offeset x times : <track,duration,pitch,onset> x <dt>
    //                -> <track,duration,pitch,onset,dt>
    CPRelVar times_offset(*this,GRelation(5),GRelation::create_full(5));
    join(*this,v,0,o,times_offset);
    
    // plus: <x,y,z> : x + y = z
    GRelation plus = makeSumRelation();
    CPRelVar plusVar(*this,plus,plus);

    //CPRelVar tmp(*this,GRelation(4),GRelation::create_full(4));
    follow(*this,times_offset,2,plusVar,f);
  }
  /**
   * \brief The score \a s must be harminc with respect to \a cons
   */
  void consonant(CPRelVar s, GRelation cons) {
    // From S: <track,duration,pitch,onset>, get <pitch,onset>
    CPRelVar pitch_onset(*this,GRelation(2),GRelation::create_full(2));
    projection(*this, 2, s, pitch_onset);

    // From pitch_onset: <pitch,onset> get <onset,pitch>
    CPRelVar onset_pitch(*this,GRelation(2),GRelation::create_full(2));
    PermDescriptor perm_pitch_onset;
    perm_pitch_onset.permute(0, 1);
    permutation(*this, pitch_onset, onset_pitch, perm_pitch_onset);

    // Take all the pitches that occur at the same onset in a relation: <pitch,pitch> 
    // pp : <pitch,pitch>      
    //CPRelVar 
    CPRelVar pp(*this,GRelation(2),GRelation::create_full(2));
    follow(*this, pitch_onset, 1, onset_pitch, pp);

    // There is a big difference between the voices being consonants and
    // the voices only containing consonant notes
    //CPRelVar nonConsonantV(*this,consonant.complement(),consonant.complement());
    CPRelVar consonantV(*this,cons,cons);
    //disjoint(*this,pp,nonConsonantV);
    subset(*this,pp,consonantV);
    // <track,duration,pitch,onset> -- <onset,pitch> = <track,duration,pitch,pitch>
  }
  /// Create a consonant relation from the pitches of score \a s
  static GRelation isConsonant(GRelation s) {
    GRelation s_pitch_onset = s.project(2);
    
    PermDescriptor perm_pitch_onset;
    perm_pitch_onset.permute(0, 1);

    GRelation s_onset_pitch = s_pitch_onset.permute(perm_pitch_onset);
    return s_pitch_onset.follow(1, s_onset_pitch);
  }
public:
  Canon(void) = delete;
  /// Constructor from an input score
  Canon(const char* inputScore, GRelation c) 
  : cons_(c) {
    {
      // The lower bound is the relation representing the input score
      MidiAsRelation reader(inputScore);
      reader.transform();
      GRelation lb = reader.getRelation();
      // The upper bound is the full relation
      GRelation ub = GRelation::create_full(4);

      // V0: <track,duration,pitch,onset>
      v0_ = CPRelVar(*this,lb,ub);
    }
    {
      GRelation lb(1); lb.add(Tuple(90));
      GRelation ub(1); 
      // Offset: <dt>
      offset_ = CPRelVar(*this,lb,ub.Union(lb));
    }
    {
      GRelation lb(4);
      GRelation ub = GRelation::create_full(4);
      // V1: <track,duration,pitch,onset>
      v1_ = CPRelVar(*this,lb,ub);
    }
    // v1 follows v0 by offset
    followVoice(v0_, v1_, offset_);

    // S: <track,duration,pitch,onset>
    S_ = CPRelVar(*this,GRelation(4),GRelation::create_full(4));
          
    {
      // Both voices must be part of the final score
      subset(*this,v0_,S_);
      subset(*this,v1_,S_);
    }
    // we consider the main voice to be consonant
    GRelation least_consonant = isConsonant(v0_.glb());
    // this is not really needed
    consonant(v0_, least_consonant);

    // the resulting score is harmonic
    consonant(S_,least_consonant);

    // Find suitable offsets for the problem
    branch(*this,offset_);
  }
  virtual void constrain(const Gecode::Space&) {

  }
  void print(std::ostream& os) const {
    os << std::endl << std::endl 
       << "Relation: " << std::endl
       //<< v0_.glb().project(2) << std::endl
       //<< v1_.glb().project(2) << std::endl
       << S_.glb()
    << std::endl;
  }
  Canon(bool share, Canon& s)
    : Gecode::Space(share,s), cons_(s.cons_)
  {
    v0_.update(*this, share, s.v0_);
    v1_.update(*this, share, s.v1_);
    S_.update(*this, share, s.S_);
    offset_.update(*this,share,s.offset_);
    //pp.update(*this,share,s.pp);
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
    delete s;
  }
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
