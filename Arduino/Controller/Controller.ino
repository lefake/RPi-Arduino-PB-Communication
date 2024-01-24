// ======================================== INCLUDES ========================================
#include <pb_common.h>
#include <pb.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "floatarray.pb.h"
// #include "int32.pb.h"

#include "PBUtils.h"

// ======================================== CONDITIONNAL INCLUDES ========================================

// ======================================== VARIABLES ========================================

// ==================== TIMERS ====================
long lastTime = 0;
long delayInterval = 500;

// ==================== TOPICS ====================
// Out
FloatArray sensorsMsg = FloatArray_init_zero;

// In
FloatArray cmdMsg = FloatArray_init_zero;

const Topic topics[] = {
      // Out
      {SENSORS, FloatArray_fields, &sensorsMsg},
      // In
      {CMD, FloatArray_fields, &cmdMsg},
    };

// ==================== SERIAL COMMUNICATION ====================
const String START_DELIMITER = "<{";
const String END_DELIMITER = ">}";

bool recvInProgress = false;
int inCmdIndex = 0;
char inCmd[MAX_MSG_LEN] = { "\0" };
bool inCmdComplete = false;
int inCmdType = -1;
int nbsNewMsgs = 0;
int newMsgsIds[MAX_NBS_MSG];
bool msgDiscardedLength = false;

PBUtils pbUtils(topics);

// ==================== Safety=======================
bool lastEstopState = false;
bool estopState = false;
bool deadmanActive = false;

// ==================== DEVICES ====================

float test_data = 0.0;

// ======================================== MAIN ========================================
void setup()
{
  Serial.begin(115200);
  delay(10);
}

void loop()
{
  if (inCmdComplete)
  {
    inCmdComplete = false;
    bool status = pbUtils.decodePb(inCmd, newMsgsIds, nbsNewMsgs);
    
    if (status && nbsNewMsgs > 0)
    {
      for (int i = 0; i < nbsNewMsgs; ++i)
      {
        switch (newMsgsIds[i])
        {
          case CMD:
            cmdCallback();
            break;
            
          default:
            break;
        }
      }
    }
    else
    inCmdType = -1;
  }
  delay(10);

  if(millis() - lastTime > delayInterval){
    sensorsCallback();
    lastTime = millis();
  }
}

// ======================================== FUNCTIONS ========================================

// ======================================== CALLBACKS ========================================

void cmdCallback()
{
  test_data = cmdMsg.data[0];
}

void sensorsCallback()
{
  sensorsMsg.data_count = 1;
  sensorsMsg.data[0] = test_data;
  pbUtils.pbSend(1, SENSORS);
}


// ======================================== SERIAL ========================================
void serialEvent()
{
  while (Serial.available() > 0)
  {
    char inChar = Serial.read();
    int startDelimIndex = START_DELIMITER.indexOf(inChar);

    if (recvInProgress)
    {
      int endDelimIndex = END_DELIMITER.indexOf(inChar); // If start and end index is diff, will always return end
      if (endDelimIndex == -1)
      {
        inCmd[inCmdIndex++] = inChar;

        if (inCmdIndex >= MAX_MSG_LEN-1) // If the last char isn't the end delimiter the message is not going to fit
        {
          msgDiscardedLength = true;
          inCmd[inCmdIndex] = '\0';
          recvInProgress = false;
          inCmdIndex = 0;
        }
      }
      else
      {
        inCmd[inCmdIndex] = '\0';
        recvInProgress = false;
        inCmdIndex = 0;
        inCmdType = endDelimIndex;
        inCmdComplete = true;
      }
    }
    else if (startDelimIndex != -1)
      recvInProgress = true;
  }
}
