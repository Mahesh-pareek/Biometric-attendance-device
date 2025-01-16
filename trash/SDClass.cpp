#include "SDClass.h"

time_t createTime(int year, int month, int day, int hour, int minute, int second) {
    tmElements_t tm;
    tm.Year = year - 1970; // Year offset from 1970
    tm.Month = month;
    tm.Day = day;
    tm.Hour = hour;
    tm.Minute = minute;
    tm.Second = second;
    return makeTime(tm);
}

genfile::genfile(String truePath, String Company) {
  this->truePath = truePath;
  this->Company = Company;

  this->path = "/TestCompany/" + Company + truePath;
}

genfile::genfile(fs::FS &fs, String truePath, String Company) {

      this->truePath = truePath;
      this->Company = Company;

      this->path = "/TestCompany/" + Company + truePath;

      this->createDir(fs, "/TestCompany/" + Company);
      

      if (!this->checkFile(fs)) {
        

        if(this->writeFile(fs, "")) {
          Serial.println("genfile constructor called, successfull in creating a new file");

          this->present = true;
          
        } else {

        
        }
      } else {
        Serial.println("genfile constructor called, file already existed");
        this->present = true;
      }
      
}

bool genfile::appendFile(fs::FS &fs, String toAppend){
  
    File file = fs.open(this->path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return false;
    }
    if(file.print(toAppend)){
        
    } else {
        Serial.println("Append failed");
    }
    file.close();
    return true;
}

bool genfile::checkFile(fs::FS &fs){
      File file = fs.open(this->path);
      if(file){
          file.close();
          Serial.println("checkFile called, file is found");
          return true;
      } else {
          Serial.println("checkFile called, file is not found");
          return false;
      }
}

String genfile::readFile(fs::FS &fs) {
        Serial.println("Reading file: "+this->path);

        File file = fs.open(path);
        if (!file) {
          return "error";
        } 

        String file_txt = "";


        while (file.available()) {
          file_txt += char(file.read());
        }
        
        file.close();
        Serial.println("File contents: \n" + file_txt);
        return file_txt;
}


void genfile::createDir(fs::FS &fs, String path){
    Serial.println("Creating Dir:"+ path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}



bool genfile::writeFile (fs::FS &fs, String message) {
      Serial.println("Writing file: "+this->path);

      File file = fs.open(this->path, FILE_WRITE);
      if(!file){
          Serial.println("Failed to open file for writing");
          return false;
      }
      if(file.print(message)){
          Serial.println("The File has been written with the following message "+message);
          file.close();
          return true;
      } else {
          Serial.println("Write failed");
          file.close();
          return false;
      }
}

MasterList::MasterList(fs::FS &fs, String path, String Company) : genfile(path, Company) {

  this->createDir(fs, "/TestCompany/" + Company);

  this->employeeID[128][7];
  this->Name[128][31];

  for (int i = 1; i <= 127; i++) {
    for (int j = 0; j <= 31; j++) {
      *(Name[i]+j) = '\0';
    } 
    *(employeeID[i]) = 'T';
    *(employeeID[i]+1) = 'W';
    *(employeeID[i]+2) = 'B';
    for (int j = 3; j<=6; j++) {
      *(employeeID[i]+j) = '\0';
    }
  }
    
  if(!genfile::checkFile(fs)) {
    this->resetMaster(fs);
    this->localToFile(fs);
  } else {
    this->fileToLocal(fs);
  }

}

void MasterList::localToFile(fs::FS &fs) {
  genfile::writeFile(fs, "");
  for (int i = 1; i <= 127; i++) {

    String toAppend = String(i)+","+String(employeeID[i])+","+String(Name[i])+"\n";
    genfile::appendFile(fs, toAppend);
  }
}

void MasterList::resetMaster(fs::FS &fs) {
  for (int i = 1; i <= 127; i++) {
    for (int j = 0; j <= 31; j++) {
      *(Name[i]+j) = '\0';
    } 
    *(employeeID[i]) = 'T';
    *(employeeID[i]+1) = 'W';
    *(employeeID[i]+2) = 'B';
    for (int j = 3; j<=6; j++) {
      *(employeeID[i]+j) = '\0';
    }
  }
  this->localToFile(fs);
}

void MasterList::fileToLocal(fs::FS &fs) {

  File file = fs.open(path);
  int lineCount = 1;
  int horizontalCount = 0;
  String temp = "";
  char curr = '\0';
  while(file.available()) {
    
    curr = char(file.read());
    if (curr==','){
      if (horizontalCount==1) {
       
        for (int j = 3; j < (6 > temp.length() ? temp.length() : 6); j++) {
          *(this->employeeID[lineCount]+j)=temp[j];
          Serial.print(employeeID[lineCount]);
          Serial.print(" | ");
          Serial.println(lineCount);
        }
        horizontalCount = 0;
      }
      else {
        horizontalCount = 1;
      }
      temp = "";
    }
    else if (curr=='\n') {
    
      for (int j = 0; j < temp.length(); j++) {
        *(this->Name[lineCount]+j) = temp[j];
        Serial.print(Name[lineCount]);
        Serial.print(" | ");
        Serial.println(lineCount);
      }
      temp = "";
      lineCount++;
    }
    else {
      temp += curr;
    }
    
  }
}

String MasterList::returnName(fs::FS &fs, uint8_t fingerID) {
  return Name[fingerID];
}

String MasterList::returnEmpID(fs::FS &fs, uint8_t fingerID) {
  return employeeID[fingerID];
}

void MasterList::addName(fs::FS &fs, uint8_t fingerID, String empID, String empName) {
  for (int i = 0; i < (30 < empName.length() ? 30 : empName.length()); i++) {
    *(this->Name[fingerID]+i) = empName[i];
  }
  for (int i = 0; i < 6; i++) {
    *(this->employeeID[fingerID]+i) = empID[i];
  }
  localToFile(fs);
}

void MasterList::removeName(fs::FS &fs, uint8_t fingerID) {
  for (int i = 0; i < 30; i++) {
    *(this->Name[fingerID]+i) = '\0'; 
  }
  for (int i = 3; i < 6; i++) {
    *(this->employeeID[fingerID]+i) = '\0';
  }
  localToFile(fs);
}

TotalAttendance::TotalAttendance(fs::FS &fs, String empID, String Company) : genfile("/"+empID+"/Attendance.txt", Company) , ToUpload("/"+empID+"/Upload.txt", Company), 
  current_attend("/"+empID+"/Current.txt", Company) {
  String pardir = "/TestCompany/" + Company;
  String dir = "/TestCompany/"+Company+"/"+empID;
  genfile::createDir(fs, pardir);
  genfile::createDir(fs, dir);

  if (!genfile::checkFile(fs)) {
    writeFile(SD, "");
    ToUpload.writeFile(fs, "0");
    current_attend.writeFile(fs, "0");
  }
}

void TotalAttendance::appendAttendance(fs::FS &fs, const char *dateIN, const char *timeIN, const char *dateOUT, const char *timeOUT, uint32_t timediff) {
  char time_diff[6] = "00:00";

  time_diff[0] = String((uint8_t)((timediff/3600)/10))[0];
  time_diff[1] = String((uint8_t)(timediff/3600)%10)[0];
  time_diff[3] = String((uint8_t)((timediff%3600)/60)/10)[0];
  time_diff[4] = String((uint8_t)((timediff%3600)/60)%10)[0];


  genfile::appendFile(SD, dateIN); genfile::appendFile(SD, " "); genfile::appendFile(SD, timeIN); genfile::appendFile(SD, " ");
  genfile::appendFile(SD, dateOUT); genfile::appendFile(SD, " "); genfile::appendFile(SD, timeOUT); genfile::appendFile(SD, " ");
  genfile::appendFile(SD, time_diff); genfile::appendFile(SD, "\n");

  genfile::readFile(SD);

  int8_t toMark = (this->ToUpload.readFile(fs)).toInt();
  toMark++;
  this->ToUpload.writeFile(fs, String(toMark));
}


uint32_t TotalAttendance::differenceTimes(const char* dateIN, const char* timeIN, const char* dateOUT, const char* timeOUT) {
  char cyearIN[5];
  char cmonthIN[3];
  char cdayIN[3];

  char cyearOUT[5];
  char cmonthOUT[3];
  char cdayOUT[3];

  strncpy(cyearIN, dateIN, 4); cyearIN[4] = '\0';
  strncpy(cyearOUT, dateOUT, 4); cyearOUT[4] = '\0';

  strncpy(cmonthIN, dateIN+5, 2); cmonthIN[2] = '\0';
  strncpy(cmonthOUT, dateOUT+5, 2); cmonthOUT[2] = '\0';

  strncpy(cdayIN, dateIN+8, 2); cdayIN[2] = '\0';
  strncpy(cdayOUT, dateOUT+8, 2); cdayOUT[2] = '\0';

  char chourIN[3];
  char cminIN[3];

  char chourOUT[3];
  char cminOUT[3];

  strncpy(chourIN, timeIN, 2); chourIN[2] = '\0';
  strncpy(chourOUT, timeOUT, 2); chourOUT[2] = '\0';

  strncpy(cminIN, timeIN + 3, 2); cminIN[2] = '\0';
  strncpy(cminOUT, timeOUT + 3, 2); cminOUT[2] = '\0';

  uint32_t yearIN = atoi(cyearIN); uint32_t yearOUT = atoi(cyearOUT);
  uint8_t monthIN = atoi(cmonthIN); uint8_t monthOUT = atoi(cmonthOUT);
  uint8_t dayIN = atoi(cdayIN); uint8_t dayOUT = atoi(cdayOUT);

  uint8_t hourIN = atoi(chourIN); uint8_t hourOUT = atoi(chourOUT);
  uint8_t minIN = atoi(cminIN); uint8_t minOUT = atoi(cminOUT);

  time_t startTime = createTime(yearIN, monthIN, dayIN, hourIN, minIN, 0);
  time_t endTime = createTime(yearOUT, monthOUT, dayOUT, hourOUT, minOUT, 0);

  return (endTime-startTime);
}

time_t TotalAttendance::addTenHours(const char * dateIN, const char* timeIN) {
  char cyearIN[5];
  char cmonthIN[3];
  char cdayIN[3];

  strncpy(cyearIN, dateIN, 4); cyearIN[4] = '\0';
  strncpy(cmonthIN, dateIN+5, 2); cmonthIN[2] = '\0';
  strncpy(cdayIN, dateIN+8, 2); cdayIN[2] = '\0';

  char chourIN[3];
  char cminIN[3];
  
  strncpy(chourIN, timeIN, 2); chourIN[2] = '\0';
  strncpy(cminIN, timeIN + 3, 2); cminIN[2] = '\0';

  uint32_t yearIN = atoi(cyearIN); 
  uint8_t monthIN = atoi(cmonthIN); 
  uint8_t dayIN = atoi(cdayIN); 

  uint8_t hourIN = atoi(chourIN); 
  uint8_t minIN = atoi(cminIN); 



  time_t startTime = createTime(yearIN, monthIN, dayIN, hourIN, minIN, 0);

  time_t futureTime = startTime + 10*3600;

  return futureTime;
}

bool TotalAttendance::newMarked(fs::FS &fs, const char *dateMark, const char *timeMark) {
  String currentIO = (this->current_attend).genfile::readFile(fs);
  if (currentIO[0] == '1') {
    const char * cIO = currentIO.c_str();

    char dateIN[11];
    char timeIN[6];

    strncpy(dateIN, cIO+2, 10); dateIN[10] = '\0';
    strncpy(timeIN, cIO + 13, 5); timeIN[5] = '\0';

    Serial.print("DateIN "); Serial.println(dateIN);
    Serial.print("timeIN "); Serial.println(timeIN);


    uint32_t timediff;
    timediff = differenceTimes(dateIN, timeIN, dateMark, timeMark);

    if (timediff>12*3600) {

  

      time_t futureTime = addTenHours(dateIN, timeIN);

          Serial.print("DateIN "); Serial.println(dateIN);
    Serial.print("hhhhhhhhhh timeIN "); Serial.println(timeIN);

  

      tmElements_t tm;
      breakTime(futureTime, tm);

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

      appendAttendance(fs, dateIN, timeIN, dateIN, timeIN, 0);
      this->current_attend.genfile::writeFile(fs, "1,");
      this->current_attend.genfile::appendFile(fs, dateMark);
      this->current_attend.genfile::appendFile(fs, ",");
      this->current_attend.genfile::appendFile(fs, timeMark);

      return true;
    }
    else{
      appendAttendance(fs, dateIN, timeIN, dateMark, timeMark, timediff);
      
      this->current_attend.genfile::writeFile(fs, "0");

      return false;
    }

  } 
  else {
    this->current_attend.genfile::writeFile(fs, "1,");
    this->current_attend.genfile::appendFile(fs, dateMark);
    this->current_attend.genfile::appendFile(fs, ",");
    this->current_attend.genfile::appendFile(fs, timeMark);

    return true;
  }
}

AttendanceSystem::AttendanceSystem(fs::FS &fs, String Company) : genfile("", Company), latestID("/LatestID.txt", Company) {

  createDir(fs, this->path);

  if (!latestID.checkFile(fs)) {
    latestID.writeFile(fs, "1");
  } else {
    Serial.print("The Current ID to be used is " + latestID.readFile(fs));
  }

}

uint8_t AttendanceSystem::getLatestID(fs::FS &fs) {
  return latestID.readFile(fs).toInt();
}

void AttendanceSystem::increaseID(fs::FS &fs) {
  latestID.writeFile(fs, String(latestID.readFile(fs).toInt()+1));
}

String AttendanceSystem::returnState(fs::FS &fs, String empID) {
  TotalAttendance tempObject(fs, empID, Company);
  String currentAttend = tempObject.current_attend.readFile(fs);
  if (currentAttend[0]=='1') {
    String toReturn = empID + ","+String("                    ")+ ",";
    toReturn += currentAttend.substring(2)+",";
    return toReturn;
  } else {
    return "";
  }
}

String AttendanceSystem::returnCurrentIn(fs::FS &fs) {
  int latestID = getLatestID(fs);
  String returnString = "";
  for (int i = 1; i < latestID; i++) {
    returnString += returnState(fs, "TWB"+String(i/100)+String((i%100)/10) + String(i%10));
  }
  return returnString;
}





