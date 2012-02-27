#include <iostream>
#include <transformer.hh>


class MidiAsRelation : public MidiTransformer {
public:
  MidiAsRelation(const char* fname) : MidiTransformer(fname) {}
  virtual
  void processChannelMessage(int track, unsigned long time, int channel,
                             bool noteOn, int note, int velocity) override {
    cout << "Processed message: " << time << ";" << channel << ";" << noteOn
       << ";" << note << ";" << velocity << endl;

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
  return 0;
}
