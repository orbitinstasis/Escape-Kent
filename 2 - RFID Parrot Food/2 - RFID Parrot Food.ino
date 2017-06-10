/*****************************

Each of the four tags has a number written on it 1 to 4.

These will associate to the wav tracks (must be wav) on the SD card with similar names. 

If you buy new tags, then increase TOTAL_TAGS_AVAILABLE to the new total, 
  and add the tag id (it will show you when you scan it) to 'knownTags'. 

FILENAMES MUST BE CAPATILISED and MUST BE 16 BIT 44.1 KHz

******************************/

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14
#define TOTAL_TAGS_AVAILABLE 5

AudioPlaySdWav           playSdWav1;
AudioOutputI2S           i2s1;
AudioConnection          patchCord1(playSdWav1, 0, i2s1, 0);
AudioConnection          patchCord2(playSdWav1, 1, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;

const int tagLen = 16;
const int idLen = 13;
const int kTags = TOTAL_TAGS_AVAILABLE;

// Put your known tags here!
char knownTags[kTags][idLen] = {
             "7F001B48725E", // tag 1 
             "7F001B54D5E5", // tag 2 
             "7F001B231354", // tag 3  
             "7F001B413D18", // tag 4 
             "7F001AFD72EA"  // tag 5
};

char newTag[idLen];

void setup() {
  // Starts the hardware and software serial ports
  Serial.begin(9600);
  Serial.println("RFID Parrot by Ben Kazemi for Escape Kent\n\n");
  Serial1.begin(9600);
  AudioMemory(8);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  delay(1000);
}

void loop() {

  int i = 0;

  int readByte;

  boolean tag = false;

  if (Serial1.available() == tagLen) {
    tag = true;
  }

  if (tag == true) {
    while (Serial1.available()) {
      // Take each byte out of the serial buffer, one at a time
      readByte = Serial1.read();

      if (readByte != 2 && readByte!= 13 && readByte != 10 && readByte != 3) {
        newTag[i] = readByte;
        i++;
      }

      if (readByte == 3) {
        tag = false;
      }

    }
  }

  if (strlen(newTag)== 0) {
    return;
  }

  else {
    int total = 0;
    boolean flag = true;
    int ct = 0;
    for (ct=1; ct < kTags + 1 && flag; ct++){
        total += checkTag(newTag, knownTags[ct - 1]);
        if (total == 1)
          flag = false;
    }

    if (total > 0 && playSdWav1.isPlaying() == false) {

// UN MUTE 
      
      Serial.print("Tag ");
      Serial.print(ct - 1);
      Serial.print(": ");
      String temp; 
      switch (ct - 1)
      {
        case 1:
            playFile("ONE.WAV");
            break;
        case 2:
            playFile("TWO.WAV");
            break;
        case 3:
            playFile("THREE.WAV");
            break;
        case 4:        
            playFile("FOUR.WAV");
            break;
        case 5:        
            playFile("FIVE.WAV");
            break;
      }

      // MUTE THIS SHIT AND FUCKING MUTE AT SETUP TOO 
    }

    else if (total == 1 && playSdWav1.isPlaying() == false){
        Serial.print("Add ID to the list KNOWN TAGS and increment TOTAL_TAGS_AVAILABLE if you want it in the game.\n");
        Serial.print(newTag);
        Serial.println();
    }
  }

  for (int c=0; c < idLen; c++) {
    newTag[c] = 0;
  }
}

int checkTag(char nTag[], char oTag[]) {
    for (int i = 0; i < idLen; i++) {
      if (nTag[i] != oTag[i]) {
        return 0;
      }
    }
  return 1;
}

void playFile(const char *filename)
{
  Serial.print("Playing file ");
  Serial.println(filename);
  playSdWav1.play(filename);
  delay(10);
}

