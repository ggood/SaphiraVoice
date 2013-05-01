#include <Bounce.h>
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <SFEMP3Shield.h>

SdFat sd;
SFEMP3Shield MP3player;

const byte button_pins[] = {A0, A1, A2, A3, A4, A5};
const byte momentary_sounds[1] = {1};  // Starts at one, not zero
Bounce bounce[sizeof(button_pins)] = {
  Bounce(button_pins[0], 5),
  Bounce(button_pins[1], 5),
  Bounce(button_pins[2], 5),
  Bounce(button_pins[3], 5),
  Bounce(button_pins[4], 5),
  Bounce(button_pins[5], 5),
};


void setup() {
  uint8_t result; //result code from some function as to be tested at later time.

  Serial.begin(115200);
  
  for (byte i = 0; i < sizeof(button_pins); i++) {
    pinMode(button_pins[i], INPUT_PULLUP); 
  }
  
  // Initialize the SD card
  if (!sd.begin(SD_SEL, SPI_HALF_SPEED)) {
    sd.initErrorHalt();
  }
  if (!sd.chdir("/")) {
    sd.errorHalt("sd.chdir");
  }
  
  // Initialize the SparkFun MP3 Shield
  if ((result = MP3player.begin()) != 0) {
    Serial.print("MP3 shield init failed: ");
    Serial.println(result);
  }
}

int button_pressed = -1;
int current_sound = -1;

int get_button() {
  for (byte i = 0; i < 6; i++) {
    bounce[i].update();
    if (!bounce[i].read()) {
      return i + 1;
    }
  }
  return -1;
}

void fade_out(unsigned int time) {
  // Fade to silence over the course of <time> milliseconds
  const unsigned int fade_quantum = 5; // change volume every 5 ms
  unsigned int fade_cycles = time / fade_quantum;
  unsigned int vol = 0;
  unsigned int fade_per_cycle = 0xFE / fade_cycles;
  for (unsigned int i = 0; vol <= 0xFE; i++) {
    if (get_button() != -1) {
      // Button pressed before fade complete. Exit.
      break;
    }
    MP3player.setVolume(vol, vol);
    vol += fade_per_cycle;
  }
  MP3player.stopTrack();
  Serial.print("Stopped");
}


void loop() {
  char filename[16];
  
  button_pressed = get_button();
  
  if (button_pressed == -1) {
    if (MP3player.isPlaying()) {
      // Is this a momentary sound, or should it keep going?
      Serial.print("is playing, checking ");
      Serial.print(sizeof(momentary_sounds));
      Serial.print(" buttons, current sound = ");
      Serial.println(current_sound);
      for (byte i = 0; i < sizeof(momentary_sounds); i++) {
        if (momentary_sounds[i] == current_sound) {
          // Yes, turn it off
          Serial.println("Turning off track");
          fade_out(100);
          current_sound = -1;
        }
      }
    } else {
      // Playback has finished
      current_sound = -1;
    }
  } else if (button_pressed != current_sound) {
    // Play a sound
    sprintf(filename, "track%03d.mp3", button_pressed);
    Serial.print("Playing file ");
    Serial.println(filename);
    MP3player.setVolume(0, 0);
    MP3player.playMP3(filename);
    Serial.print("Done playing");
    current_sound = button_pressed;
  }
}
