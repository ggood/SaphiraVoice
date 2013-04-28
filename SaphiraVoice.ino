#include <Bounce.h>
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <SFEMP3Shield.h>

SdFat sd;
SFEMP3Shield MP3player;

byte button_pins[] = {A0, A1, A2, A3, A4, A5};
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

void loop() {
  char filename[16];
  
  button_pressed = get_button();
  if (!MP3player.isPlaying()) {
    current_sound = -1;
  }
  if (button_pressed > -1 && button_pressed != current_sound) {
    // Play a sound
    sprintf(filename, "track%03d.mp3", button_pressed);
    Serial.print("Playing file ");
    Serial.println(filename);
    MP3player.playMP3(filename);
    current_sound = button_pressed;
  }
}
