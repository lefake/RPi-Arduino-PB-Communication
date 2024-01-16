#ifndef _CONSTANTS_H
#define _CONSTANTS_H

// ======================= Sonars ===================================
#define NBS_SONARS                          14
#define MIN_DIST_DETECTION_M                0.02
#define MAX_DIST_DETECTION_M                2.0
#define TEMPERATURE                         20.0

// ======================= IMU ======================================
#define IMU_ADDRESS                         0x68
#define IMU_MAG_DECLINATION                 14.926      //https://www.geomag.nrcan.gc.ca/calc/mdcal-en.php

// ======================= GPS ======================================
#ifndef DEGTORAD
#define DEGTORAD                            0.0174532925199432957f
#define RADTODEG                            57.295779513082320876f
#endif

// ======================= ENCODERS ==================================
#define NBS_ENCODERS                        2
#define NOP_A5                              0x00
#define RD_POS                              0x10
#define WAIT                                0xA5
#define NBS_TICK_PER_REV                    4096.0
#define TICK_SAFTY_RATIO                    1.15
#define MAX_SPEED_RPM                       130.0

// ======================= MOTORS ===================================

#define RADIUS 0.24/2
#ifndef M_PI
#define M_PI 3.14159
#endif

// ======================= Servos ===================================
#define NBR_SERVOS                          2
#define INIT_ELEVATION                      45 // En degree
#define INIT_ROTATION                       0  // En degree


// ======================= SERIAL COMMUNICATION =====================
#define ID_BROADCAST_DELAY                  500
#define DATA_MSGS                           0
#define STATUS_MSGS                         1

// ======================= EEPROM  ==================================
#define ID_EEPROM_ADDRESS                   0x00
#define IMU_EEPROM_ADDRESS                  0x10

// ======================= ROBOT  ===================================
#define TRACK_WIDTH                         0.546


#endif // _CONSTANTS_H
