/*This file is part of the Maslow Control Software.

The Maslow Control Software is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Maslow Control Software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the Maslow Control Software.  If not, see <http://www.gnu.org/licenses/>.

Copyright 2014-2017 Bar Smith*/

// This file contains the machine settings that are saved to eeprom

#include "Maslow.h"

void settingsInit(){
    // Do we have any error handling of this?
    settingsLoadFromEEprom();
    settingsLoadStepsFromEEprom();
}

void settingsLoadFromEEprom(){
    /* 
    Loads data from EEPROM if EEPROM data is valid, only called on startup
    
    Settings are stored starting at address 40 all the way up.
    */
    settingsVersion_t settingsVersionStruct;
    
    settingsReset(); // Load default values first
    EEPROM.get(0, settingsVersionStruct);
    if (settingsVersionStruct.settingsVersion == SETTINGSVERSION &&
        settingsVersionStruct.eepromValidData == EEPROMVALIDDATA){
        // This is a valid data
        EEPROM.get(40, sysSettings);
    }
}


void settingsReset() {
    /* 
    Loads default data into settings, many of these values are approximations
    from the an ideal stock frame.  Other values are just the recommended
    value.  Ideally we want these defaults to match the defaults in GroundControl
    so that if a value is not changed by a user or is not used, it doesn't 
    need to be updated here.
    */
    sysSettings = {
        2438.4, // float machineWidth;
        1219.2, // float machineHeight;
        2978.4, // float distBetweenMotors;
        463.0,  // float motorOffsetY;
        310.0,  // float sledWidth;
        139.0,  // float sledHeight;
        79.0,   // float sledCG;
        1,      // byte kinematicsType;
        100.0,  // float rotationDiskRadius;
        2000,   // int axisHoldTime;
        200,    // int kinematicsMaxGuess;
        1650,   // int originalChainLength;
        8113.7, // float encoderSteps;
        10,     // byte gearTeeth;
        6.35,   // float chainPitch;
        1000,   // int maxFeed;
        true,   // zAxisAttached;
        false,  // bool zAxisAuto;
        12.60,  // float maxZRPM;
        3.17,   // float zDistPerRot;
        7560.0, // float zEncoderSteps;
        1300.0, // float KpPos;
        0.0,    // float KiPos;
        34.0,   // float KdPos;
        1.0,    // float propWeightPos;
        7.0,    // float KpV;
        0.0,    // float KiV;
        0.28,   // float KdV;
        1.0,    // float propWeightV;
        1300.0, // float zKpPos;
        0.0,    // float zKiPos;
        34.0,   // float zKdPos;
        1.0,    // float zPropWeightPos;
        7.0,    // float zKpV;
        0.0,    // float zKiV;
        0.28,   // float zKdV;
        1.0,    // float zPropWeightV;
        EEPROMVALIDDATA // byte eepromValidData;
    };
}

void settingsSaveToEEprom(){
    /* 
    Saves settings to EEPROM, only called when settings change
    
    Settings are stored starting at address 40 all the way up.
    */
    settingsVersion_t settingsVersionStruct = {SETTINGSVERSION, EEPROMVALIDDATA};
    EEPROM.put(0, settingsVersionStruct);
    EEPROM.put(40, sysSettings);
}

void settingsSaveStepstoEEprom(){
    /* 
    Saves position to EEPROM, is called frequently by execSystemRealtime
    
    Steps are saved in address 10 -> 39.  Room for expansion for additional
    axes in the future.
    */
    settingsVersion_t settingsVersionStruct = {SETTINGSVERSION, EEPROMVALIDDATA};
    EEPROM.put(0, settingsVersionStruct);
    EEPROM.put(10, sysSteps);
}

void settingsLoadStepsFromEEprom(){
    /* 
    Saves position to EEPROM, is on startup.  This struct should never change
    so it doesn't check the settings version
    
    Steps are saved in address 10 -> 39.  Room for expansion for additional
    axes in the future.
    */
    settingsStepsV1_t tempStepsV1;
    settingsVersion_t settingsVersionStruct;
    
    settingsReset(); // Load default values first
    EEPROM.get(0, settingsVersionStruct);
    if (settingsVersionStruct.settingsVersion == SETTINGSVERSION &&
        settingsVersionStruct.eepromValidData == EEPROMVALIDDATA){
        EEPROM.get(10, tempStepsV1);
        if (tempStepsV1.eepromValidData == EEPROMVALIDDATA){
            sysSteps = tempStepsV1;
        }
        else {
            systemRtExecAlarm |= ALARM_POSITION_LOST;
        }
    }// We can add additional elseif statements here to check for old settings 
    // versions and upgrade them without a loss of data.
    else if (EEPROM.read(5) == EEPROMVALIDDATA &&
        EEPROM.read(105) == EEPROMVALIDDATA &&
        EEPROM.read(205) == EEPROMVALIDDATA){
        // Try and load position from pre settings days
        float l, r , z;
        EEPROM.get(9, l);
        EEPROM.get(109, r);
        EEPROM.get(209, z);
        // Old method stored position as a float of rotations
        // TODO figure out when this runs how to convert to steps. 
        // perhaps just set axis to position and let conversion happen
        // naturally there
        // There is a small bug in the old way of doing it as the number
        // of rotations is calculated using either the encoderSteps per 
        // rotation or distance per rotation, changing of either of these
        // values requires a recalibration of the machine
        //position = {l,r,z, EEPROMVALIDDATA}
    }
    else {
        systemRtExecAlarm |= ALARM_POSITION_LOST;  // if this same global is touched by ISR then need to make atomic somehow
                                                   // also need to consider if need difference between flag with bits and
                                                   // error message as a byte.
    }
}

byte settingsStoreGlobalSetting(const byte parameter,const float value){
    /*
    Alters individual settings which are then stored to EEPROM.  Returns a 
    status message byte value 
    */
    
    // We can add whatever sanity checks we want here and error out if we like
    switch(parameter) {
        case 0: case 1: case 2: case 3: case 4: case 5:
            switch(parameter) {
                case 0:
                      sysSettings.machineWidth = value;
                      break;
                case 1: 
                      sysSettings.machineHeight = value;
                      break;
                case 2: 
                      sysSettings.distBetweenMotors = value;
                      break;
                case 3: 
                      sysSettings.motorOffsetY = value;
                      break;
                case 4: 
                      sysSettings.sledWidth = value;
                      break;
                case 5: 
                      sysSettings.sledHeight = value;
                      break;
            }
            // TODO not sure how to handle kinematics calc now. or when we 
            // should finalize.  What happens if not fully complete and 
            // location can't be calculated
            sys.rcvdKinematicSettings = 1;
            finalizeMachineSettings();
            kinematics.recomputeGeometry();
            break;
        case 6: 
              sysSettings.sledCG = value;
              break;
        case 7: 
              sysSettings.kinematicsType = value;
              break;
        case 8: 
              sysSettings.rotationDiskRadius = value;
              break;
        case 9: 
              sysSettings.axisHoldTime = value;
              break;
        case 10: 
              sysSettings.kinematicsMaxGuess = value;
              break;
        case 11: 
              sysSettings.originalChainLength = value;
              break;
        case 12: 
              sysSettings.encoderSteps = value;
              leftAxis.changeEncoderResolution(sysSettings.encoderSteps);
              rightAxis.changeEncoderResolution(sysSettings.encoderSteps);
              sys.encoderStepsChanged = true;
              break;
        case 13: 
              sysSettings.gearTeeth = value;
              leftAxis.changePitch(sysSettings.gearTeeth*sysSettings.chainPitch);
              rightAxis.changePitch(sysSettings.gearTeeth*sysSettings.chainPitch);
              kinematics.R = (sysSettings.gearTeeth*sysSettings.chainPitch)/(2.0 * 3.14159);
              break;
        case 14: 
              sysSettings.chainPitch = value;
              leftAxis.changePitch(sysSettings.gearTeeth*sysSettings.chainPitch);
              rightAxis.changePitch(sysSettings.gearTeeth*sysSettings.chainPitch);
              kinematics.R = (sysSettings.gearTeeth*sysSettings.chainPitch)/(2.0 * 3.14159);
              break;
        case 15: 
              sysSettings.maxFeed = value;
              break;
        case 16:
              sysSettings.zAxisAttached = value;
              break;
        case 17: 
              sysSettings.zAxisAuto = value;
              break;
        case 18: 
              sysSettings.maxZRPM = value;
              break;
        case 19: 
              sysSettings.zDistPerRot = value;
              zAxis.changePitch(sysSettings.zDistPerRot);
              break;
        case 20: 
              sysSettings.zEncoderSteps = value;
              zAxis.changeEncoderResolution(sysSettings.zEncoderSteps);
              sys.zEncoderStepsChanged = true;
              break;
        case 21: case 22: case 23: case 24: case 25: case 26: case 27: case 28:
            switch(parameter) {
                case 21:
                      sysSettings.KpPos = value;
                      break;
                case 22: 
                      sysSettings.KiPos = value;
                      break;
                case 23: 
                      sysSettings.KdPos = value;
                      break;
                case 24: 
                      sysSettings.propWeightPos = value;
                      break;
                case 25: 
                      sysSettings.KpV = value;
                      break;
                case 26: 
                      sysSettings.KiV = value;
                      break;
                case 27: 
                      sysSettings.KdV = value;
                      break;
                case 28: 
                      sysSettings.propWeightV = value;
                      break;
                }
                leftAxis.setPIDValues(sysSettings.KpPos, sysSettings.KiPos, sysSettings.KdPos, sysSettings.propWeightPos, sysSettings.KpV, sysSettings.KiV, sysSettings.KdV, sysSettings.propWeightV);
                rightAxis.setPIDValues(sysSettings.KpPos, sysSettings.KiPos, sysSettings.KdPos, sysSettings.propWeightPos, sysSettings.KpV, sysSettings.KiV, sysSettings.KdV, sysSettings.propWeightV);
                break;
        case 29: case 30: case 31: case 32: case 33: case 34: case 35: case 36:
            switch(parameter) {
                case 29: 
                      sysSettings.zKpPos = value;
                      break;
                case 30: 
                      sysSettings.zKiPos = value;
                      break;
                case 31: 
                      sysSettings.zKdPos = value;
                      break;
                case 32: 
                      sysSettings.zPropWeightPos = value;
                      break;
                case 33: 
                      sysSettings.zKpV = value;
                      break;
                case 34: 
                      sysSettings.zKiV = value;
                      break;
                case 35: 
                      sysSettings.zKdV = value;
                      break;
                case 36: 
                      sysSettings.zPropWeightV = value;
                      break;
            }
            zAxis.setPIDValues(sysSettings.zKpPos, sysSettings.zKiPos, sysSettings.zKdPos, sysSettings.zPropWeightPos, sysSettings.zKpV, sysSettings.zKiV, sysSettings.zKdV, sysSettings.zPropWeightV);
            break;
        default:
              return(STATUS_INVALID_STATEMENT);
    }
    settingsSaveToEEprom();
    return(STATUS_OK);
}