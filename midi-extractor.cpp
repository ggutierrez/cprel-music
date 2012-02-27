
#include <iostream>
#include <jdksmidi/world.h>
#include <jdksmidi/multitrack.h>
#include <jdksmidi/filereadmultitrack.h>
#include <jdksmidi/fileread.h>

using namespace std;

/**
 * \brief Class to encapsulate a midi file and react on the information it
 * contains.
 *
 */
class MidiTransformer {
private:
  /// Source of the midi
  const char *fname_;
  /// Information contained in the midi file
  jdksmidi::MIDIMultiTrack tracks_;
public:
  /// Avoid default construction
  MidiTransformer(void) = delete;
  /// Create from a midi source
  MidiTransformer(const char *fname) : fname_(fname), tracks_(1) {
    jdksmidi::MIDIFileReadStreamFile midiStream(fname_);
    jdksmidi::MIDIFileReadMultiTrack trackLoader(&tracks_);
    jdksmidi::MIDIFileRead reader(&midiStream,&trackLoader);
    tracks_.ClearAndResize(reader.ReadNumTracks());
    if (!reader.Parse()) {
      std::cerr << "Error parsing the midi file" << std::endl;
    }
  }
private:
  void trackInfo(int trackNum) {
    jdksmidi::MIDITrack *track = tracks_.GetTrack(trackNum);
    if (!track->EventsOrderOK())
      track->SortEventsOrder();
    // Iterate on every message in the track
    jdksmidi::MIDITimedBigMessage message;
    //char buff[64];
    for (int i = 0; i < track->GetNumEvents(); ++i) {
      track->GetEvent(i, &message);
      // We only consider channel events and just note on/off events.
      if (message.IsChannelEvent() && message.IsNote()) {
        processChannelMessage(trackNum, message.GetTime(),
                              message.GetChannel() + 1,
                              message.IsNoteOn(), message.GetNote(),
                              message.GetVelocity());
        // message for human.
        // cout << "Message: " << message.MsgToText(buff) << endl;
      }
    }
  }
public:
  /// Method to trigger the transformer
  void transform(void) {
    for (int i = 0; i < tracks_.GetNumTracks(); ++i)
      trackInfo(i);
  }
  /// Handler for every message in the file
  virtual
  void processChannelMessage(int track, jdksmidi::MIDIClockTime time, int channel,
                             bool noteOn, int note, int velocity) = 0;
};

class MidiAsRelation : public MidiTransformer {
public:
  MidiAsRelation(const char* fname) : MidiTransformer(fname) {}
  virtual
  void processChannelMessage(int track, jdksmidi::MIDIClockTime time, int channel,
                             bool noteOn, int note, int velocity) override {

  }
};
/*
void processChannelMessage(jdksmidi::MIDIClockTime time, int channel, bool noteOn,
                           int note, int velocity) {
  cout << "Processed message: " << time << ";" << channel << ";" << noteOn
       << ";" << note << ";" << velocity << endl;
}

void midiStats(jdksmidi::MIDIFileRead& reader) {
  cout << "Number of tracks: " << reader.ReadNumTracks() << endl;
}

void trackInfo(jdksmidi::MIDITrack *track) {
  cout << "There are " << track->GetNumEvents() << " events in the track"
    << endl;
  // Test if the events on the track are ordered and order them if not.
  if (!track->EventsOrderOK())
    track->SortEventsOrder();
  // Iterate on every message in the track
  jdksmidi::MIDITimedBigMessage message;
  char buff[64];
  for (int i = 0; i < track->GetNumEvents(); ++i) {
    track->GetEvent(i, &message);
    // We only consider channel events and just note on/off events.
    if (message.IsChannelEvent() && message.IsNote()) {
      processChannelMessage(message.GetTime(), message.GetChannel() + 1,
                            message.IsNoteOn(), message.GetNote(),
                            message.GetVelocity());
      // message for human.
      cout << "Message: " << message.MsgToText(buff) << endl;
    }
  }
}
*/
int main ( int argc, char **argv ) {
  if (argc < 2) {
    cout << "Midi extractor options: " << endl
      << argv[0] << " input.midi " << endl;
    return 0;
  }

  MidiAsRelation transformer(argv[1]);
  transformer.transform();
/*
  const char *inputFile = argv[1];
  jdksmidi::MIDIFileReadStreamFile midiStream(inputFile);
  cout << "Starting midi extraction on: " << inputFile << endl;

  // Container for all the tracks
  jdksmidi::MIDIMultiTrack tracks(1); // initally set to 1 but resized later
  // Loader of tracks in the tracks object
  jdksmidi::MIDIFileReadMultiTrack trackLoader(&tracks);
  // The actual reader
  jdksmidi::MIDIFileRead reader(&midiStream,&trackLoader);
  tracks.ClearAndResize(reader.ReadNumTracks());
  // Output some statistics on the midi file
  midiStats(reader);
  // Load midi file in the multi track object
  if (!reader.Parse()) {
    cerr << "Error parsing the file: " << inputFile << endl;
    return 1;
  }
  // At this point the tracks object is a representation of the midi file.
  trackInfo(tracks.GetTrack(0));
  */
  return 0;
}
