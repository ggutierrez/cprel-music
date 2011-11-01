
#include <cassert>
#include <vector>
#include <jdksmidi/world.h>
#include <jdksmidi/fileread.h>
#include <jdksmidi/fileshow.h>
#include <jdksmidi/multitrack.h>
#include <jdksmidi/filereadmultitrack.h>
#include <jdksmidi/filewritemultitrack.h>
#include <jdksmidi/utils.h>

using namespace jdksmidi;


#include <iostream>
using namespace std;

void readTrack(const MIDITrack& track) {
  int numEvents = track.GetNumEvents();
  vector<vector<unsigned long> > activeNotesPerChannel(16);
  for (int i = 0; i < 16; i++)
    activeNotesPerChannel[i].resize(200,0);

  std::cout << "There are " << numEvents << " events " << std::endl;
    
  for (int i = 0; i < numEvents; i++) {
    const MIDITimedBigMessage *event = track.GetEvent(i);
    //std::cout << EventAsText(*event) << std::endl; 
    if (event->IsNoteOn()) {
      int channel = (int)event->GetChannel();
      int note = (int)event->GetNote();
      MIDIClockTime startTime = event->GetTime();
      //std::cerr << "Note On " << channel << " " << note << std::endl;
      //assert(activeNotesPerChannel.at(channel).at(note) == 0);
      activeNotesPerChannel[channel][note] = startTime;
    }
   
    if (event->IsNoteOff()) {
      //std::cout << EventAsText(*event) << std::endl; 
      int channel = (int)event->GetChannel();
      int note = (int)event->GetNote();
      MIDIClockTime stopTime = event->GetTime();
      //assert(activeNotesPerChannel.at(channel).at(note) != 0);
      unsigned long onset = activeNotesPerChannel.at(channel).at(note);
      unsigned long duration = stopTime - onset;
      std::cout << "This should be a triple: " << note << " " << onset << " " << duration << std::endl; 
      // do something here and then restore to zero
      activeNotesPerChannel[channel][note] = 0;
    }
  }
}
void readTracks(const MIDIMultiTrack& tracks) {
  int numTracks = tracks.GetNumTracks();
  //std::cout << "There are " << numTracks << std::endl; 
  for (int i = 0; i < numTracks; i++)
    readTrack(*(tracks.GetTrack(i)));
}

int main ( int argc, char **argv ) {
  if ( argc > 2 ) {
    const char *infile_name = argv[1];
    const char* outfile_name = argv[2];
    
    MIDIFileReadStreamFile rs ( infile_name );
    if ( !rs.IsValid() ) {
      cerr << "\nError opening file " << infile_name << endl;
      return -1;
    }

    // the multitrack object which will hold all the tracks
    MIDIMultiTrack tracks( 1 ); // only 1 track in multitrack object - not enough for midifile format 1
    
    // the object which loads the tracks into the tracks object
    MIDIFileReadMultiTrack track_loader ( &tracks );
    
    // the object which parses the midifile and gives it to the multitrack loader
    MIDIFileRead reader ( &rs, &track_loader );
    
    // make amount of of tracks equal to midifile_num_tracks
    int midifile_num_tracks = reader.ReadNumTracks();
    tracks.ClearAndResize( midifile_num_tracks );


    /*
    // if true print META_SEQUENCER_SPECIFIC events as text string
    bool sqspecific_as_text = true;

    MIDIFileShow shower ( stdout, sqspecific_as_text );
    MIDIFileRead reader ( &rs, &shower );
    */
    if ( !reader.Parse() ) {
      cerr << "\nError parse file " << infile_name << endl;
      return -1;
    }

    readTracks(tracks);

    // writing
    // create the output stream
    MIDIFileWriteStreamFileName out_stream ( outfile_name );
    
    if ( out_stream.IsValid() ) {
      // the object which takes the midi tracks and writes the midifile to the output stream
      MIDIFileWriteMultiTrack writer ( &tracks, &out_stream );

      // uncomment this string for output midifile without running status usage
      // writer.UseRunningStatus( false );

      // extract the original multitrack division
      int division = reader.GetDivision();

      // get really number of track, used in reader.Parse() process
      int num_tracks = tracks.GetNumTracksWithEvents();
      
      // write the output midi file
      if ( writer.Write ( num_tracks, division ) ) {
	cout << "\nAll OK. Number of tracks with events " << num_tracks << endl;;
      } else {
	cerr << "\nError writing file " << outfile_name << endl;
      }
    } else {
      cerr << "\nError opening file " << outfile_name << endl;
    }
  }
  else {
    cerr << "usage:\n\tjdkmidi_test_show INFILE.mid\n";
    return -1;
  }

  return 0;
}
