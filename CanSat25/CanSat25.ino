// ATTENTION! If (CLOSED) is present next to a function it means that section is finished and doesn't need further work.

// NOTE: Every Serial.println() command should be replaced with Xbee.write once the Xbee is implemented.

String inputString = "";  // Holds incoming data sent via serial communication.
int SIMcounter = 0;
int CX_on = 0;
int CAMERA_MOTOR_on = 0;
bool stringComplete = false;  // Indicates whether the string is complete.

// Define all the data variables (e.g., pressure, position, etc.)
String telemetryMessage = "";
int PACKET_COUNT = 0;
String MODE = "F"; //can be either F (flight mode) or S (simulation mode). When the Cansat starts. it is in flight mode by default
String STATE = "";
float altitude_at_CAL = 0;
float altitude = 0;
float TRUE_ALTITUDE = 0;
float TEMPERATURE = 0;
int sim_pressure = 0;
float kPRESSURE = 0; //pressure in kPascal. This is needed only in telemetry
float VOLTAGE = 0;
float GYRO_R = 0; // R = roll
float GYRO_P = 0; // P = pitch
float GYRO_Y = 0; // Y = yaw
float ACCEL_R = 0;
float ACCEL_P = 0;
float ACCEL_Y = 0;
float MAG_R = 0;
float MAG_P = 0;
float MAG_Y = 0;
float AUTO_GYRO_ROTATION_RATE = 0;
String GPS_TIME = "";
float GPS_ALTITUDE = 0;
float GPS_LATITUDE = 0;
float GPS_LONGITUDE = 0;
int GPS_SATS = 0;
String CMD_ECHO = "";

void restoreFunc() {
  // Empties the inputString variable to allow a new command to be processed.
  if (stringComplete) {
    inputString = "";  // Possibly redundant, to be checked.
    stringComplete = false;
  }
}

void (*resetFunc)(void) = 0; // Resets the Teensy microcontroller.

void CX(StrFIRSTOPTION, CX_on) {
  if (StrFIRSTOPTION == "ON") {
    if (CX_on == 0) {
      CX_on = 1; //telemetry is started
    } else {
      Serial.println("Telemetry is already active.");
    }
  } else if (StrFIRSTOPTION == "OFF") {
    if (CX_on == 1) {
      CX_on = 0; //telemetry is stopped
    } else {
      Serial.println("Telemetry is already inactive.");
    }
  } else {
    Serial.println("Command is incomplete.");
  }
}

void ST(StrFIRSTOPTION) { // Sets the mission time
  if (StrFIRSTOPTION == "GPS") {
    // Retrieve current time from the GPS sensor and write it to telemetry
  } else {
    int indexOfFirstTD = StrFIRSTOPTION.indexOf(":");
    int indexOfSecondTD = StrFIRSTOPTION.indexOf(":", indexOfFirstTD + 1);
    if (indexOfFirstTD == -1 || indexOfSecondTD == -1) {
      Serial.println("Time format is incorrect. Retry.");
      return;
    }
    // Ensure at least one character between each pair of colons
    if (indexOfFirstTD <= 1 || indexOfSecondTD - indexOfFirstTD <= 1 || StrFIRSTOPTION.length() - indexOfSecondTD <= 1) {
      Serial.println("Time format is incorrect. Retry.");
      return;
    }
    String StrHour = StrFIRSTOPTION.substring(0, indexOfFirstTD);
    String StrMinutes = StrFIRSTOPTION.substring(indexOfFirstTD + 1, indexOfSecondTD);
    String StrSeconds = StrFIRSTOPTION.substring(indexOfSecondTD + 1);
  }
}

void SIM(StrFIRSTOPTION, SIMcounter, MODE) { // (CLOSED) Manages simulated mode activation.
  if (StrFIRSTOPTION == "ENABLE" && SIMcounter == 0) {
    SIMcounter = 1;
  } else if (StrFIRSTOPTION == "ACTIVATE" && SIMcounter == 1 && MODE == "F") {
    // SIMactive = 1 enables SIMP function
    MODE = "S";
  } else if (StrFIRSTOPTION == "ACTIVATE" && SIMcounter == 1 && MODE == "S") {
    Serial.println("SIM mode is already active");
    return;
  } else if (StrFIRSTOPTION == "DISABLE" && MODE == "S") {
    SIMcounter = 0;
    MODE = "F";
    // Stop simulation mode
  } else if (StrFIRSTOPTION == "DISABLE" && MODE == "F") {
    Serial.println("SIM mode is already disabled.");
    return;
  } else {
    Serial.println("SIM option unrecognized. Retry.");
  }
}

void SIMP(StrFIRSTOPTION, MODE) { // Requires MODE == S (simulation mode active).
  if (MODE == "S") {
    if (StrFIRSTOPTION == "") {
      Serial.println("No pressure data entered.");
      return;
    }
    sim_pressure = StrFIRSTOPTION.toInt();
    Serial.print("Pressure set to: ");
    Serial.println(sim_pressure);  // pressure is an integer type measured in Pascal ("resolution of one Pascal")
    Serial.println("Resend if the value is incorrect."); // Ensures a valid float conversion.
  } else {
    Serial.println("Simulated mode is not active.");
  }
}

void CAL(altitude) { // Sets ground altitude to 0 at launch pad installation.
  altitude_at_CAL = altitude;
  // Telemetry should use TRUE_ALTITUDE = altitude - altitude_at_CAL
  Serial.print("Ground altitude set to: ");
  Serial.println(TRUE_ALTITUDE);
  packet_count = 0; //this resets the number of packets sent via the Xbee
}

void MEC(StrFIRSTOPTION, StrSECONDOPTION, CAMERA_MOTOR_on) {
  if (StrFIRSTOPTION == "camera") {
    if (StrSECONDOPTION == "ON") {
      if (CAMERA_MOTOR_on == 0){
        // Activate motor to move camera positioned at 45°
      } else {
        Serial.println("Camera motor is already on.");
        return;
      }
    } else if (StrSECONDOPTION == "OFF") {
      if (CAMERA_MOTOR_on == 1) {
        // Shut off camera motor
      } else {
        Serial.println("Camera motor is already off.");
        return;
      }
    } else {
      Serial.println("Second option is invalid.");
      return;
    }
  } else {
    Serial.println("Device not recognized. Retry.");
    return;
  }
}

void telemetry(MISSION_TIME, PACKET_COUNT, MODE, STATE, TRUE_ALTITUDE, TEMPERATURE, kPRESSURE, VOLTAGE, GYRO_R, GYRO_P, GYRO_Y, ACCEL_R, ACCEL_P, ACCEL_Y, MAG_R, MAG_P, MAG_Y, AUTO_GYRO_ROTATION_RATE, GPS_TIME, GPS_ALTITUDE, GPS_LATITUDE, GPS_LONGITUDE, GPS_SATS, CMD_ECHO) {
  // (CLOSED)
  telemetryMessage = "3132," + String(MISSION_TIME) + "," + String(PACKET_COUNT) + "," + MODE + "," + STATE + "," + String(TRUE_ALTITUDE, 1) + "," + String(TEMPERATURE,1) + ",";
  telemetryMessage = telemetryMessage + String(kPRESSURE, 1) + "," + String(VOLTAGE, 1) + "," + String(GYRO_R) + "," + String(GYRO_P) + "," + String(GYRO_Y) + ",";
  telemetryMessage = telemetryMessage + String(ACCEL_R) + "," + String(ACCEL_P) + "," + String(ACCEL_Y) + "," + String(MAG_R) + "," + String(MAG_P) + "," + String(MAG_Y) + ",";
  telemetryMessage = telemetryMessage + String(AUTO_GYRO_ROTATION_RATE) + "," + GPS_TIME + "," + String(GPS_ALTITUDE,1) + "," + String(GPS_LATITUDE,4) + "," + String(GPS_LONGITUDE,4) + ",";
  telemetryMessage = telemetryMessage + String(GPS_SATS) + "," + CMD_ECHO;
  Serial.println(telemetryMessage);
  PACKET_COUNT = PACKET_COUNT + 1; //we update the number of packets sent at a rate of 1 Hz
}

void control(inputString) { // Validates string sent from ground station.
  inputString.trim(); // Removes whitespace
  int indexOfFirstComma = inputString.indexOf(",");
  int indexOfSecondComma = inputString.indexOf(",", indexOfFirstComma + 1);
  int indexOfThirdComma = inputString.indexOf(",", indexOfSecondComma + 1);
  int indexOfFourthComma = inputString.indexOf(",", indexOfThirdComma + 1);
  // Validate comma positions
  if (indexOfFirstComma == -1 || indexOfSecondComma == -1 || indexOfThirdComma == -1 || indexOfFourthComma == -1) {
    Serial.println("Incomplete command. Retry.");
    return;
  }
  String StrCMD = inputString.substring(0, indexOfFirstComma);
  String StrTEAMID = inputString.substring(indexOfFirstComma + 1, indexOfSecondComma);
  String StrCOMMANDID = inputString.substring(indexOfSecondComma + 1, indexOfThirdComma);
  String StrFIRSTOPTION = inputString.substring(indexOfThirdComma + 1, indexOfFourthComma);
  String StrSECONDOPTION = inputString.substring(indexOfFourthComma + 1);
  if (StrCMD == "CMD" && StrTEAMID == "3132") {
    if (StrCOMMANDID == "CX") {
      CX(StrFIRSTOPTION, CX_on);
    } else if (StrCOMMANDID == "ST") {
      ST(StrFIRSTOPTION);
    } else if (StrCOMMANDID == "SIM") {
      SIM(StrFIRSTOPTION, SIMcounter, SIMactive);
    } else if (StrCOMMANDID == "SIMP") {
      SIMP(StrFIRSTOPTION, MODE);
    } else if (StrCOMMANDID == "CAL") {
      CAL(altitude);
    } else if (StrCOMMANDID == "MEC") {
      MEC(StrFIRSTOPTION, StrSECONDOPTION, CAMERA_MOTOR_on);
    } else {
      Serial.println("Unknown command. Retry.");
    }
  }
  CMD_ECHO = StrCOMMANDID + StrFIRSTOPTION + StrSECONDOPTION;
}

void setup() {
  delay(1);
  Serial.begin(9600);
  while (Serial.available()) {  // Clears serial buffer post-reset.
    Serial.read();
  }
}

void loop() {
  while (Serial.available()) {
    inputString = Serial.readString();  // Stores input as a string
    stringComplete = true;              // Runs control function once per command
  }
  if (stringComplete) {  // Processes command if saved completely.
    control(inputString);
    restoreFunc();
  }
  if (CX_on == 1) {
    telemetry(MISSION_TIME, PACKET_COUNT, MODE, STATE, TRUE_ALTITUDE, TEMPERATURE, kPRESSURE, VOLTAGE, GYRO_R, GYRO_P, GYRO_Y, ACCEL_R, ACCEL_P, ACCEL_Y, MAG_R, MAG_P, MAG_Y, AUTO_GYRO_ROTATION_RATE, GPS_TIME, GPS_ALTITUDE, GPS_LATITUDE, GPS_LONGITUDE, GPS_SATS, CMD_ECHO);
  }
}
