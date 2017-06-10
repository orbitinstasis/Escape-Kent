// copyright © Ben Kazemi 2017
// copyright © Escape Kent 2017

// Pirate Puzzle

#include <Bounce.h>
#include <Encoder.h>

#define ENCODER_FULL_REV                  80
#define ENCODER_HALF_REV                  ENCODER_FULL_REV/2
#define GAME_CLOCKWISE                    true
#define GAME_COUNTER_CLOCKWISE            false

// CHANGE BELOW ONLY ///////////////////////////////////////////////////////////////////////////////
/*
 *  please note that the directions for each step need to alternate, i.e. don't set two steps in the same direction
 */
#define GAME_UNLOCK_DURATION_SECS         10
#define STEP_ONE_DISTANCE                 ENCODER_HALF_REV
#define STEP_ONE_DIRECTION                GAME_CLOCKWISE
#define STEP_TWO_DISTANCE                 ENCODER_HALF_REV  // example of how you can add the distances
#define STEP_TWO_DIRECTION                GAME_COUNTER_CLOCKWISE
#define STEP_THREE_DISTANCE               ENCODER_HALF_REV
#define STEP_THREE_DIRECTION              GAME_CLOCKWISE

// CHANGE ABOVE ONLY ///////////////////////////////////////////////////////////////////////////////

#define BOARD_LED_STATE                   true
#define BOARD_LED                         13
#define ENCODER_RESET                     0
#define POSITION_CHECK_ERR_TOLERANCE      10
#define FIRST_MOVE_ERR_TOLERANCE          6
#define GAME_LEVELS                       3
#define GAME_WAIT_TIME_SUCCESS            700
#define GAME_LEVEL_COUNT_DISTANCE         0
#define GAME_LEVEL_DIRECTION              1
#define GAME_UNLOCK_DURATION              GAME_UNLOCK_DURATION_SECS * 1000
#define GAME_DOOR_LOCKED                  true
#define GAME_DOOR_UNLOCKED                false
#define ROTARY_BUTTON                     20
#define FORCE_UNLOCK_BUTTON               9
#define DOOR_GATE                         8

Bounce bouncer = Bounce( FORCE_UNLOCK_BUTTON, 50 );
Encoder myEnc(11, 10);
int currentgameLevel = 0;
boolean isFromBoot = true;
boolean debounceSwitch = false;
boolean resetGameState = true;
boolean isDoorLocked = true;
boolean haveLockedAlready = true; // init to true for flow
boolean isClockwise = false;
boolean isFirstLoop = true;
boolean isGameFinished = false;                     //probably don't need
boolean haveRegistedWaitCheck = false;
boolean moving = false;                             //probably don't need
long oldPosition  = -999;
long newPosition = 0;
int gameLevel [GAME_LEVELS] [2] = { // [countDistance][goingClockwise?]
  {STEP_ONE_DISTANCE, STEP_ONE_DIRECTION},                     // full clockwise
  {STEP_TWO_DISTANCE, STEP_TWO_DIRECTION}, // 1.5 counter clockwise
  {STEP_THREE_DISTANCE, STEP_THREE_DIRECTION}                      // half clockwise
};
int hints [GAME_LEVELS] = {21, 16, 17};
elapsedMillis sinceRead;

void resetGame(boolean isFullReset) {   // if param true, reset all, assert locked,
  noInterrupts();
  //  if (isFullReset) {                  // trigger this when the door locks DECIDE IF NECESSARY
  //    isDoorLocked = true;
  //  }
  Serial.println("    resetGame CALLED");
  delayMicroseconds(100000);
  isFirstLoop = true;
  myEnc.write(0);
  interrupts();
}

void setHints(int hintNum, boolean state) {
  state ? digitalWrite(hints[hintNum], HIGH) : digitalWrite(hints[hintNum], LOW);
}

void lockDoor (void) {
  Serial.println("lockDoor");
  digitalWrite(DOOR_GATE, GAME_DOOR_LOCKED);
  isGameFinished = false;
  isDoorLocked = true;
  resetGameState = true;

}

void forceUnlock() {

    isDoorLocked = false;
    Serial.println("    YOU PUSHED THE BUTTON ");



  //  if (digitalRead(FORCE_UNLOCK_BUTTON) == true) {
  //    digitalWrite(DOOR_GATE, true);
  //    Serial.println("      FORCE LOCKED");
  //  } else {
  //    digitalWrite(DOOR_GATE, false);
  //    Serial.println("      FORCE UNLOCKED");
  //  }
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < GAME_LEVELS; i++) {
    pinMode(hints[i], OUTPUT);
    digitalWrite(hints[i], LOW);
  }
  pinMode(BOARD_LED, OUTPUT);
  digitalWrite(BOARD_LED, BOARD_LED_STATE);
  pinMode(ROTARY_BUTTON, INPUT);
  digitalWrite(ROTARY_BUTTON, LOW);
  pinMode(FORCE_UNLOCK_BUTTON, INPUT);
  pinMode(DOOR_GATE, OUTPUT);
  digitalWrite(DOOR_GATE, HIGH);
  //attachInterrupt(FORCE_UNLOCK_BUTTON, forceUnlock, RISING);
  delayMicroseconds(500000);
}


void loop() {

  if ( bouncer.update() && !isFromBoot) {
    if ( bouncer.read() == HIGH) {
      forceUnlock();
    }
  } else 
    isFromBoot = false;

  if (!isDoorLocked) {
    Serial.println("!isDoorLocked");
    digitalWrite(DOOR_GATE, GAME_DOOR_UNLOCKED);   // unlock
    elapsedMillis holdPeriod;
    while (holdPeriod < GAME_UNLOCK_DURATION) {  }
    lockDoor();
  }

  newPosition = myEnc.read();

  if (resetGameState) {
    resetGame(false);
    resetGameState = false;
  }

  if (sinceRead >= GAME_WAIT_TIME_SUCCESS && !haveRegistedWaitCheck) {
    moving = false;
    haveRegistedWaitCheck = true;
    if ((abs(gameLevel[currentgameLevel][GAME_LEVEL_COUNT_DISTANCE] - abs(newPosition)) <= POSITION_CHECK_ERR_TOLERANCE) && (gameLevel[currentgameLevel][GAME_LEVEL_DIRECTION] == isClockwise)) {
      Serial.print("    LEVEL ");
      Serial.print(currentgameLevel + 1);
      Serial.println(" PASSED");
      setHints(currentgameLevel, true);
      //      Serial.println(gameLevel[currentgameLevel][GAME_LEVEL_COUNT_DISTANCE]);
      //      Serial.println(newPosition);
      //      Serial.println(abs(gameLevel[currentgameLevel][GAME_LEVEL_COUNT_DISTANCE] - abs(newPosition)));
      currentgameLevel++;
      if (currentgameLevel >= GAME_LEVELS) {
        isDoorLocked = false;
        //          haveLockedAlready = false;
        Serial.println("      CONGRATS, GAME COMPLETE");
        isGameFinished = true;          // move this out to the point where you register a correct move
        // OPEN DOOR  AND WAIT UNTIL THE DOOR IS CLOSED TO RESET AND LOCK THE DOOR TO START OVER (reset hints)
      }
    } else {
      resetGameState = true;
      currentgameLevel = 0;
      for (int i = 0; i < GAME_LEVELS; i++) {
        setHints(i, false);
      }
    }
    //    Serial.println(gameLevel[currentgameLevel][GAME_LEVEL_COUNT_DISTANCE]);
    //    Serial.println(newPosition);
    //    Serial.println(gameLevel[currentgameLevel][GAME_LEVEL_COUNT_DISTANCE] - abs(newPosition));
  }


  if (newPosition != oldPosition) {         //only process when we're moving
    sinceRead = 0;
    moving = true;
    haveRegistedWaitCheck = false;
    if (isFirstLoop) {
      Serial.println("    IN FIRST LOOP");
      isFirstLoop = false;
      if (newPosition < oldPosition)
        isClockwise = false;
      else
        isClockwise = true;
      oldPosition = newPosition;
    }

    if (newPosition != 0) {
      if (newPosition < oldPosition) {        // counter clockwise
        if (isClockwise) {                    // check if we were already going counter clockwise
          isClockwise = false;                // if already counter clockwise then don't reset
          myEnc.write(ENCODER_RESET);
          oldPosition  = 0;
        }
      }

      if (newPosition > oldPosition) {        // clockwise
        if (!isClockwise) {                    // check if we were already going counter clockwise
          isClockwise = true;                // if already counter clockwise then don't reset
          myEnc.write(ENCODER_RESET);
          oldPosition  = 0;
        }
      }
    }

    Serial.print(isClockwise);
    Serial.print("    ");
    Serial.print(newPosition);
    Serial.print("    ");
    Serial.println(oldPosition);
    oldPosition = newPosition;

  } // end turning processing

} // end loop

