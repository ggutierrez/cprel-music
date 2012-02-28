#include <jdksmidi/world.h>
#include <jdksmidi/multitrack.h>

// implementation ->
#include <jdksmidi/filewritemultitrack.h>

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
  /// Create from a midi source \a fname
  MidiTransformer(const char *fname);
private:
  /// Transform the information in \a trackNum
  void trackInfo(int trackNum);
public:
  /// Method to trigger the transformer
  void transform(void);
  /// Handler for every message in the file
  virtual
  void processChannelMessage(int track, unsigned long time, int channel,
                             bool noteOn, int note, int velocity) = 0;
};

class MidiWriter {
private:
  /// Information to write in the midi file
  jdksmidi::MIDIMultiTrack tracks_;
public:
  /// Avoid default construction
  MidiWriter(int tracks) : tracks_(tracks + 1) {
    tracks_.SetClksPerBeat(100);
    // track 0 -> custom things...
    jdksmidi::MIDITimedBigMessage m;
    m.SetTime(0);
    m.SetTimeSig( 4, 2 ); // measure 4/4 (default values for time signature)
    tracks_.GetTrack(0)->PutEvent(m);

    m.SetTempo(1000000);
    tracks_.GetTrack(0)->PutEvent(m);
    tracks_.GetTrack(0)->PutTextEvent(0, jdksmidi::META_TRACK_NAME, "CPRel");
  }
  /// Add a message to the midi file
  void processChannelMessage(const std::vector<int>& info) {
    // Assumes:
    // info[0] : track
    // info[1] : time
    // info[2] : channel
    // info[3] : duration
    // info[4] : note
    // info[5] : velocity
    jdksmidi::MIDITimedBigMessage m;
    m.SetTimeSig( 4, 2 ); // measure 4/4 (default values for time signature)
    m.SetTempo(1000000);

    m.SetTime(info.at(1));
    m.SetNoteOn(info.at(2),info.at(4),info.at(5));
    // the noteOn message
    tracks_.GetTrack(info.at(0)+1)->PutEvent(m);
    
    m.SetTime(info.at(1) + info.at(3));
    m.SetNoteOff(info.at(2),info.at(4),info.at(5));
    // the noteOff message
    tracks_.GetTrack(info.at(0)+1)->PutEvent(m);
  }
  /// Write the midi file
  void write(const char* filename) {
    // Order messages before writing them?
    jdksmidi::MIDIFileWriteStreamFileName out_stream(filename);
    if( out_stream.IsValid() ) {
      jdksmidi::MIDIFileWriteMultiTrack writer( &tracks_, &out_stream);
      if ( writer.Write(2)) {
        cout << "File saved: " << filename << endl;
      }  else {
        cerr << "\nError writing file " << filename << endl;
      }
    } else {
      cerr << "\nError opening file " << filename << endl;
    }
  }
};