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

/*
 * Midi Writer
 */

MidiWriter::MidiWriter(int tracks) : tracks_(tracks+1) {
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

void MidiWriter::processChannelMessage(int track, int channel, int note, 
                                       int velocity, int duration,
                                       jdksmidi::MIDIClockTime time) {

  // we keep track 0 for midi information
  track++;
  jdksmidi::MIDITimedBigMessage m;
  //m.SetTimeSig( 4, 2 ); // measure 4/4 (default values for time signature)
    //m.SetTempo(1000000);

  m.SetTime(time);
  m.SetNoteOn(channel,note,velocity);
  // the noteOn message
  tracks_.GetTrack(track)->PutEvent(m);
  
  m.SetTime(time+100);
  m.SetNoteOff(channel,note,velocity);
  // the noteOff message
  tracks_.GetTrack(track)->PutEvent(m);
}

void MidiWriter::write(const char *filename) {
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



