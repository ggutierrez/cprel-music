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
    cout << "Processed message: " << time << ";" << channel << ";" << noteOn
    << ";" << note << ";" << velocity << endl;
    int tm = time;
    int b = noteOn ? 1 : 0;
    MPG::Tuple t(track,tm,channel,b,note,velocity); 
    midiRel_.add(t);
  }
  MPG::GRelation getRelation(void) {
    return midiRel_;
  }
};

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
  return 0;
}
