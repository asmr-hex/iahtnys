#include <RtAudio.h>
#include <RtMidi.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <cmath>


struct some_data {
  double data[2];
  std::vector<unsigned char>* msg;
};

void midi_cb(double deltatime, std::vector<unsigned char> *msg, void *user_data) {
  some_data *d = (some_data*) user_data;
  d->msg = msg;
  int i = 0;
  for ( auto m : *msg ) {
    std::string s;
    switch (i) {
    case 0:
      s = "first byte";
      break;
    case 1:
      s = "second byte";
      break;
    case 2:
      s = "third byte";
      break;
    }
    std::cout << s << ": " << (int)m << "\n";
    i++;
  }
}

// Two-channel sawtooth wave generator.
int saw( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData ) {
  unsigned int i, j;
  double *buffer = (double *) outputBuffer;
  some_data *d = (some_data *) userData;
  std::vector<unsigned char> *msg = d->msg;
  double *lastValues = d->data;
  if ( status )
    std::cout << "Stream underflow detected!" << std::endl;

  float note = (float)(*msg)[1];
  note = pow(2, (note-30)/12);
  
  if ((int)(*msg)[0] == 145) {
    // Write interleaved audio data.
    for ( i=0; i<nBufferFrames; i++ ) {
      for ( j=0; j<2; j++ ) {
        *buffer++ = lastValues[j];
        lastValues[j] += note * (0.005 * (j*(1.1) + 1));
        if ( lastValues[j] >= 1.0 ) lastValues[j] -= 2.0;
      }
    }    
  }

  if ((int)(*msg)[0] == 129) {
    // Write interleaved audio data.
    for ( i=0; i<nBufferFrames; i++ ) {
      for ( j=0; j<2; j++ ) {
        *buffer++ = lastValues[j];
        lastValues[j] = 0;
        if ( lastValues[j] >= 1.0 ) lastValues[j] -= 2.0;
      }
    }    
  }

  return 0;
}


int main() {
  RtMidiIn *midi_in;
  
  try {
    midi_in = new RtMidiIn();
  } catch ( RtMidiError &error ) {
    error.printMessage();
    exit( EXIT_FAILURE );
  }

  for (int i = 0; i< midi_in->getPortCount(); i++) {
    std::cout << midi_in->getPortName(i) << "\n";
  }

  midi_in->openPort(2);
  
  some_data d;

  midi_in->setCallback(midi_cb, &d);
  
  RtAudio dac;
  if ( dac.getDeviceCount() < 1 ) {
    std::cout << "\nNo audio devices found!\n";
    exit( 0 );
  }
  RtAudio::StreamParameters parameters;
  parameters.deviceId = dac.getDefaultOutputDevice();
  parameters.nChannels = 2;
  parameters.firstChannel = 0;
  unsigned int sampleRate = 44100;
  unsigned int bufferFrames = 256; // 256 sample frames
  // double data[2];


  
  
  try {
    dac.openStream( &parameters, NULL, RTAUDIO_FLOAT64,
                    sampleRate, &bufferFrames, &saw, (void *)&d );
    dac.startStream();
  }
  catch ( RtAudioError& e ) {
    e.printMessage();
    exit( 0 );
  }
  
  char input;
  std::cout << "\nPlaying ... press <enter> to quit.\n";
  std::cin.get( input );
  try {
    // Stop the stream
    dac.stopStream();
  }
  catch (RtAudioError& e) {
    e.printMessage();
  }
  if ( dac.isStreamOpen() ) dac.closeStream();
  return 0;
}
