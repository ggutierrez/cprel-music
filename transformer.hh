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

/*
 * \brief Class to write to a midi file
 */
class MidiWriter {
private:
  /// Information to write in the midi file
  jdksmidi::MIDIMultiTrack tracks_;
public:
  /// Avoid default construction
  MidiWriter(int tracks);
  /// Add a message to the midi file
  //void processChannelMessage(const std::vector<int>& info) {
  void processChannelMessage(int track, int channel, int note, int velocity,
                             int duration, jdksmidi::MIDIClockTime time);
  /// Write the midi file
  void write(const char* filename);
};