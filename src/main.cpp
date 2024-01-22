#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <myWaveForm.h>

AudioSynthWaveform waveform;
AudioOutputI2S           i2s1;           //xy=360,98
AudioOutputAnalogStereo  dacs1;          //xy=372,173
AudioMixer4              mixer1;         //xy=445,386
AudioConnection          patchCord1(waveform, 0, mixer1, 0);
AudioConnection          patchCord2(mixer1, 0, i2s1, 0);
AudioConnection          patchCord3(mixer1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=239,232

// harmony test
AudioSynthWaveform waveform2;
AudioSynthWaveform waveform3;
AudioConnection          patchCord4(waveform2, 0, mixer1, 1);
AudioConnection          patchCord5(waveform3, 0, mixer1, 2);

// vibroto
AudioSynthWaveform waveform4;
AudioConnection          patchCord6(waveform2, 0, mixer1, 3);

#define FORCE_SENSOR_PIN 41
#define SOFTPOT_PIN 40
#define PITCH_SHIFT 14
#define FORCE_READING_CAP 100

void setup() {
  Serial.begin(9600);
  pinMode(FORCE_SENSOR_PIN, INPUT);
  pinMode(SOFTPOT_PIN, INPUT);
  pinMode(PITCH_SHIFT, INPUT);
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(10);
  Serial.println("enabling sgt something");
  // Comment these out if not using the audio adaptor board.
  // This may wait forever if the SDA & SCL pins lack
  // pullup resistors
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.4);
  
  waveform.arbitraryWaveform(myWaveform, 172.0);
  waveform.frequency(400);
  waveform.amplitude(1);
  waveform.begin(WAVEFORM_SINE);

  // harmonics
  waveform2.arbitraryWaveform(myWaveform, 172.0);
  waveform2.begin(WAVEFORM_SINE);
  waveform3.arbitraryWaveform(myWaveform, 172.0);
  waveform3.begin(WAVEFORM_SINE);

  waveform2.frequency(800);
  waveform2.amplitude(1);
  waveform3.frequency(1200);
  waveform3.amplitude(1);

  // vibroto
  waveform4.arbitraryWaveform(myWaveform, 172.0);
  waveform4.frequency(400);
  waveform4.amplitude(1);
  waveform4.begin(WAVEFORM_SINE);
}

void loop() {
  int forceReading = analogRead(FORCE_SENSOR_PIN);
  int softpotReading = analogRead(SOFTPOT_PIN);
  int pitchShiftReading = analogRead(PITCH_SHIFT);
  Serial.print("pitch shift reading = ");
  Serial.println(pitchShiftReading);
  // Serial.print("force sensor reading: ");
  // Serial.println(forceReading);
  if (forceReading > FORCE_READING_CAP)  // forceReading is capped for a consistent max volume
  {
    forceReading = FORCE_READING_CAP;
  }

  if (forceReading > 30)
  {
    int mappedForceVal = map(forceReading, 30, FORCE_READING_CAP, 0, 1000);
    float volume = (float)mappedForceVal / 1000.0;
    float freqMultiplier = (float)softpotReading / 1023.0;
    int pitchShiftFreq = map(pitchShiftReading, 1, 1023, 0, 100);
    // Serial.print("freq multiplier = ");
    // Serial.println(freqMultiplier);
    int baseFreq = (100.0 + freqMultiplier * 900.0) + pitchShiftFreq;

    // CHANGING THE FREQUENCY
    AudioNoInterrupts();
    waveform.frequency(baseFreq);
    waveform2.frequency(baseFreq * 2); // high harmonic
    waveform3.frequency(baseFreq * 0.5); // low harmonic

    waveform4.frequency(baseFreq + 10); // attempt at vibroto lmfao
    mixer1.gain(0,volume*0.5);
    mixer1.gain(1,volume*0.3);
    mixer1.gain(2,volume*0.2);
    mixer1.gain(3,volume*0.0);
    AudioInterrupts();
  }
  else
  {
    AudioNoInterrupts();
    mixer1.gain(0, 0);
    mixer1.gain(1, 0);
    mixer1.gain(2, 0);
    mixer1.gain(3, 0);
    AudioInterrupts();
  }
}