#include <transformer.hh>
#include <jdksmidi/filereadmultitrack.h>
#include <jdksmidi/fileread.h>

MidiTransformer::MidiTransformer(const char *fname) : fname_(fname), tracks_(1) {
  jdksmidi::MIDIFileReadStreamFile midiStream(fname_);
  jdksmidi::MIDIFileReadMultiTrack trackLoader(&tracks_);
  jdksmidi::MIDIFileRead reader(&midiStream,&trackLoader);
  tracks_.ClearAndResize(reader.ReadNumTracks());
  if (!reader.Parse()) {
    /// \todo Rise exception
    std::cerr << "Error parsing the midi file" << std::endl;
  }
}

void MidiTransformer::trackInfo(int trackNum) {
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

void MidiTransformer::transform(void) {
  for (int i = 0; i < tracks_.GetNumTracks(); ++i)
    trackInfo(i);
}