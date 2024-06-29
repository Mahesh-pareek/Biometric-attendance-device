void handleMarkOut() {
  
  if (server.hasArg("plain") == false) {
    server.send(400, "text/plain", "Invalid Request");
    return;
  }

  String body = server.arg("plain");
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, body);
  deserializeJson(doc, body);
  String fingerID_ = doc["id"];
  String outDate = doc["out_date"];  //dd/mm/yyyy
  String outTime = doc["out_time"];  //hh:mm (24 hour)
  int id = (String(fingerID_[3])+String(fingerID_[4])+String(fingerID_[5])).toInt();


  // write function here
  TotalAttendance tempObject(SD, fingerID_, CompanyName);
  char dateMark[11] = "";
  char timeMark[6] = "";

  outDate.toCharArray(dateMark, sizeof(dateMark));
  outTime.toCharArray(timeMark, sizeof(timeMark));
  bool inorout = tempObject.newMarked(SD, dateMark, timeMark); 
  updateAttendance(id);

  String response = "Marked out employee" + fingerID_ + " at " + outTime;
  server.send(200, "text/plain", response);
  
}

void handleEndShift() {
  
  if (server.hasArg("plain") == false) {
    server.send(400, "text/plain", "Invalid Request");
    return;
  }

  String body = server.arg("plain");
  DynamicJsonDocument doc(256);
  deserializeJson(doc, body);
  String fingerID_ = doc["id"];  // it's finger id
  Serial.println("fingerID " + fingerID_);

  int id = (String(fingerID_[3])+String(fingerID_[4])+String(fingerID_[5])).toInt();

  // here
  
  TotalAttendance tempObject(SD, fingerID_, CompanyName);

  String currentIO = tempObject.current_attend.readFile(SD);
  
  const char * cIO = currentIO.c_str();

  char dateIN[11];
  char timeIN[6];

  strncpy(dateIN, cIO+2, 10); dateIN[10] = '\0';
  strncpy(timeIN, cIO + 13, 5); timeIN[5] = '\0';

  Serial.print("DateIN "); Serial.println(dateIN);
  Serial.print("timeIN "); Serial.println(timeIN);

  time_t shiftEnd = tempObject.addxHours(dateIN, timeIN, 8);

  tmElements_t tm;
  breakTime(shiftEnd, tm);

  uint32_t yearOUT = tm.Year + 1970;
  uint8_t monthOUT = tm.Month;
  uint8_t dayOUT = tm.Day;
  uint8_t hourOUT = tm.Hour;
  uint8_t minuteOUT = tm.Minute;

  Serial.println(String(yearOUT) + " " + String(monthOUT) + " " + String(dayOUT) + " " + String(hourOUT) + " " + String(minuteOUT));

  char dateOUT[11];
  char timeOUT[6];

  dateOUT[4] = '/';
  dateOUT[7] = '/';

  timeOUT[2] = ':';

  dateOUT[0] = '0' + yearOUT/1000;
  dateOUT[1] = '0' + (yearOUT%1000)/100;
  dateOUT[2] = '0' + (yearOUT%100)/10;
  dateOUT[3] = '0' + (yearOUT%10);

  dateOUT[5] = '0' + monthOUT/10;
  dateOUT[6] = '0' + monthOUT%10;

  dateOUT[8] = '0' + dayOUT/10;
  dateOUT[9] = '0' + dayOUT%10;

  dateOUT[10] = '\0';

  Serial.print("DateIN "); Serial.println(dateIN);
  Serial.print("timeIN "); Serial.println(timeIN);

  timeOUT[0] = '0' + hourOUT/10;
  timeOUT[1] = '0' + hourOUT%10;

  timeOUT[3] = '0' + minuteOUT/10;
  timeOUT[4] = '0' + minuteOUT%10;

  timeOUT[5] = '\0';

  Serial.print("DateIN "); Serial.println(dateIN);
  Serial.print("timeIN "); Serial.println(timeIN);
  Serial.print("dateOUT : "); Serial.println(dateOUT);
  Serial.print("TimeOUT ");Serial.println(timeOUT);

  tempObject.newMarked(SD, dateOUT, timeOUT);
  updateAttendance(id);

  String response = "Ended shift for employee" + fingerID_;
  server.send(200, "text/plain", response);
  
}



void handleMarkAttendance() {
  if (server.hasArg("plain") == false) {
    server.send(400, "text/plain", "Invalid Request");
    return;
  }

  String body = server.arg("plain");
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, body);

  if (error) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  int id = doc["id"];
  String inDate = doc["in_date"];
  String inTime = doc["in_time"];
  String outTime = doc["out_time"]; 
   // Optional
  
  String empID = "TWB" + String(id/100) + String((id%100)/10) + String(id%10);
  TotalAttendance tempObject(SD, empID, CompanyName);

  

  




  String logEntry = "ID: " + String(id) + ", In Date: " + inDate + ", In Time: " + inTime;
  if (outTime != "") {
    logEntry += ", Out Time: " + outTime;

    int hours = outTime.toInt();

    char dateIN[11];
    char timeIN[6];

    for (int i = 0; i < 10; i++) dateIN[i] = inDate[i];
    dateIN[10] = '\0';

    for (int j = 0; j < 5; j++) timeIN[j] = inTime[j];
    timeIN[5] = '\0';

    Serial.print("DateIN "); Serial.println(dateIN);
    Serial.print("timeIN "); Serial.println(timeIN);

    time_t shiftEnd = tempObject.addxHours(dateIN, timeIN, hours);

    tmElements_t tm;
    breakTime(shiftEnd, tm);

    uint32_t yearOUT = tm.Year + 1970;
    uint8_t monthOUT = tm.Month;
    uint8_t dayOUT = tm.Day;
    uint8_t hourOUT = tm.Hour;
    uint8_t minuteOUT = tm.Minute;

    Serial.println(String(yearOUT) + " " + String(monthOUT) + " " + String(dayOUT) + " " + String(hourOUT) + " " + String(minuteOUT));

    char dateOUT[11];
    char timeOUT[6];

    dateOUT[4] = '/';
    dateOUT[7] = '/';

    timeOUT[2] = ':';

    dateOUT[0] = '0' + yearOUT/1000;
    dateOUT[1] = '0' + (yearOUT%1000)/100;
    dateOUT[2] = '0' + (yearOUT%100)/10;
    dateOUT[3] = '0' + (yearOUT%10);

    dateOUT[5] = '0' + monthOUT/10;
    dateOUT[6] = '0' + monthOUT%10;

    dateOUT[8] = '0' + dayOUT/10;
    dateOUT[9] = '0' + dayOUT%10;

    dateOUT[10] = '\0';

    Serial.print("DateIN "); Serial.println(dateIN);
    Serial.print("timeIN "); Serial.println(timeIN);

    timeOUT[0] = '0' + hourOUT/10;
    timeOUT[1] = '0' + hourOUT%10;

    timeOUT[3] = '0' + minuteOUT/10;
    timeOUT[4] = '0' + minuteOUT%10;

    timeOUT[5] = '\0';

    Serial.print("DateIN "); Serial.println(dateIN);
    Serial.print("timeIN "); Serial.println(timeIN);
    Serial.print("dateOUT : "); Serial.println(dateOUT);
    Serial.print("TimeOUT ");Serial.println(timeOUT);

    tempObject.appendAttendance(SD, dateIN, timeIN, dateOUT, timeOUT, hours*3600);
    updateAttendance(id);
  } else {
    int hours = 8;

    char dateIN[11];
    char timeIN[6];

        for (int i = 0; i < 10; i++) dateIN[i] = inDate[i];
    dateIN[10] = '\0';

    for (int j = 0; j < 5; j++) timeIN[j] = inTime[j];
    timeIN[5] = '\0';

    Serial.print("DateIN "); Serial.println(dateIN);
    Serial.print("timeIN "); Serial.println(timeIN);

    time_t shiftEnd = tempObject.addxHours(dateIN, timeIN, hours);

    tmElements_t tm;
    breakTime(shiftEnd, tm);

    uint32_t yearOUT = tm.Year + 1970;
    uint8_t monthOUT = tm.Month;
    uint8_t dayOUT = tm.Day;
    uint8_t hourOUT = tm.Hour;
    uint8_t minuteOUT = tm.Minute;

    Serial.println(String(yearOUT) + " " + String(monthOUT) + " " + String(dayOUT) + " " + String(hourOUT) + " " + String(minuteOUT));

    char dateOUT[11];
    char timeOUT[6];

    dateOUT[4] = '/';
    dateOUT[7] = '/';

    timeOUT[2] = ':';

    dateOUT[0] = '0' + yearOUT/1000;
    dateOUT[1] = '0' + (yearOUT%1000)/100;
    dateOUT[2] = '0' + (yearOUT%100)/10;
    dateOUT[3] = '0' + (yearOUT%10);

    dateOUT[5] = '0' + monthOUT/10;
    dateOUT[6] = '0' + monthOUT%10;

    dateOUT[8] = '0' + dayOUT/10;
    dateOUT[9] = '0' + dayOUT%10;

    dateOUT[10] = '\0';

    Serial.print("DateIN "); Serial.println(dateIN);
    Serial.print("timeIN "); Serial.println(timeIN);

    timeOUT[0] = '0' + hourOUT/10;
    timeOUT[1] = '0' + hourOUT%10;

    timeOUT[3] = '0' + minuteOUT/10;
    timeOUT[4] = '0' + minuteOUT%10;

    timeOUT[5] = '\0';

    Serial.print("DateIN "); Serial.println(dateIN);
    Serial.print("timeIN "); Serial.println(timeIN);
    Serial.print("dateOUT : "); Serial.println(dateOUT);
    Serial.print("TimeOUT ");Serial.println(timeOUT);

    tempObject.appendAttendance(SD, dateIN, timeIN, dateOUT, timeOUT, hours*3600);
    updateAttendance(id);
  }
  Serial.println(logEntry);

  server.send(200, "text/plain", "Attendance marked successfully");
}
