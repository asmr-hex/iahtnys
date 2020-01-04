brew install rt-audio
brew install rt-midi

g++ --std=c++17 -lrtaudio -lrtmidi -I/usr/local/Cellar/rt-audio/5.1.0/include/rtaudio -o synth main.cpp
