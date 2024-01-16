// ======================================== INCLUDES ========================================
#include <pb_common.h>
#include <pb.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "floatarray.pb.h"
#include "int32.pb.h"

#include "Configuration.h"
#include "Constants.h"
#include "StatusMessage.h"
#include "PBUtils.h"
#include "Pins.h"

// ======================================== CONDITIONNAL INCLUDES ========================================

#ifdef HAS_LIGHTTOWER
#include "LightTower.h"
#endif

// ======================================== FUNCTIONS ========================================
void propCallback();
void chuteCallback();
void soufflanteCmdCallback();
void deadmanCallback();
void estopCallback();
void lightCallback();
void pidCstCallback();

void loopSonars();
void loopSafety();
void loopController();

// ======================================== VARIABLES ========================================

// ==================== TIMERS ====================
long lastTime = 0;
long delayInterval = 100;

long lastTimeSonar = 0;
long delayIntervalSonar = 1500;

long lastTimeImu = 0;
long delayIntervalImu = 10;

long lastTimeGps = 0;
long delayIntervalGps = 200;

long lastDebounceTime = 0;
long delayDebounceInterval = 50;

// ==================== TOPICS ====================
// Out
FloatArray debugArduinoMsg = FloatArray_init_zero;
FloatArray encMsg = FloatArray_init_zero;
FloatArray imuMsg = FloatArray_init_zero;
FloatArray gpsMsg = FloatArray_init_zero;
FloatArray sonarPairsMsg = FloatArray_init_zero;
Int32 soufflanteHeightMsg = Int32_init_zero;
Int32 estopStateMsg = Int32_init_zero;
FloatArray debugMotMsg = FloatArray_init_zero;

// In
FloatArray propMsg = FloatArray_init_zero;
FloatArray chuteMsg = FloatArray_init_zero;
Int32 soufflanteCmdMsg = Int32_init_zero;
Int32 deadmanMsg = Int32_init_zero;
Int32 estopMsg = Int32_init_zero;
FloatArray lightMsg = FloatArray_init_zero;
FloatArray pidCstMsg = FloatArray_init_zero;

const Topic topics[] = {
      // Out
      {DEBUG_ARDUINO, FloatArray_fields, &debugArduinoMsg},
      {ENC, FloatArray_fields, &encMsg},
      {IMU, FloatArray_fields, &imuMsg},
      {GPS, FloatArray_fields, &gpsMsg},
      {SONAR_PAIRS, FloatArray_fields, &sonarPairsMsg},
      {SOUFFLANTE_HEIGHT, Int32_fields, &soufflanteHeightMsg},      
      {ESTOP_STATE, Int32_fields, &estopStateMsg},
      {DEBUG_MOT, FloatArray_fields, &debugMotMsg},

      // In
      {PROP, FloatArray_fields, &propMsg},
      {CHUTE, FloatArray_fields, &chuteMsg},
      {SOUFFLANTE_CMD, Int32_fields, &soufflanteCmdMsg},
      {DEADMAN, Int32_fields, &deadmanMsg},
      {ESTOP, Int32_fields, &estopMsg},
      {LIGHT, FloatArray_fields, &lightMsg},
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

#ifdef HAS_LIGHTTOWER
LightTower lightTower;
#endif

float test_data = 0.0;

// ======================================== MAIN ========================================
void setup()
{
  Serial.begin(115200);


// ==== Controller ====
    #ifdef HAS_LIGHTTOWER
    lightTower.init(lightPins);
    #endif
  
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
          case LIGHT:
            lightCallback();
            testCallback();
            break;
            
          default:
            sendStatusWithMessage(WARNING, OTHER, "Unsupported topic:" + String(newMsgsIds[i]));
            break;
        }
      }
    }
    else
    inCmdType = -1;
  }
  delay(10);
}

// ======================================== CALLBACKS ========================================

void lightCallback()
{
#ifdef HAS_LIGHTTOWER
  test_data = lightMsg.data[0];
#endif
}

void testCallback()
{
  debugMotMsg.data_count = 5;
  debugMotMsg.data[0] = test_data;
  debugMotMsg.data[1] = 1.0;   // Vitesse
  debugMotMsg.data[2] = 1.2;
  debugMotMsg.data[3] = 3.2;
  debugMotMsg.data[4] = 4.3;
  pbUtils.pbSend(1, DEBUG_MOT);
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
