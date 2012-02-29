#include <iostream>
#include <transformer.hh>
#include <bdddomain/tuple.hh>
#include <bdddomain/grelation.hh>


class MidiAsRelation : public MidiTransformer {
private:
  MPG::GRelation midiRel_; 
public:
  MidiAsRelation(const char* fname)
  : MidiTransformer(fname), midiRel_(6)
  {}
  virtual
  void processChannelMessage(int track, unsigned long time, int channel,
                             bool noteOn, int note, int velocity) override {
    /*
    cout << "Processed message: " << time << ";" << channel << ";" << noteOn
    << ";" << note << ";" << velocity << endl;
    */
    int tm = time;
    int b = noteOn ? 1 : 0;
    cout << "Processed message: " << track+1 << ";" << tm << ";" << channel << ";" << noteOn
    << ";" << note << ";" << velocity << endl;
    MPG::Tuple t(track,tm,channel,b,note,velocity); 
    midiRel_.add(t);
  }
  MPG::GRelation getRelation(void) {
    return midiRel_;
  }
};

// TODO: I have to fix the visit method to remove this hack.
MidiWriter w(1);
void functor(const std::vector<int>& v) {
  //std::vector<int> x(v.size());
  std::vector<int> x(v);
  std::reverse(begin(x), end(x));
/*  
  for (int c : x)
    cout << c << " ";
  cout << endl;
*/
 if (x[3] == 1)
    w.processChannelMessage(x[0], x[2], x[4], x[5], 499, x[1]);
  
//  cout << x.at(3) << endl;
  //if (x.at(3)) {
    //x[3] = 499;
    //w.processChannelMessage(x);
  //}
}

int main ( int argc, char **argv ) {
  if (argc < 2) {
    cout << "Midi extractor options: " << endl
    << argv[0] << " input.midi " << endl;
    return 0;
  }

  MidiAsRelation transformer(argv[1]);
  transformer.transform();
  MPG::GRelation rel = transformer.getRelation();
  cout << "Cardinality " << rel.cardinality() << endl;

  rel.visit(functor);
  w.write("out2.midi");
  return 0;
}
