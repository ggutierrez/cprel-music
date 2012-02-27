#include <jdksmidi/world.h>
#include <jdksmidi/multitrack.h>

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