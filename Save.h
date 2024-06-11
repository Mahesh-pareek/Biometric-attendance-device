#ifndef CLASSES_H
#define CLASSES_H

#include <Arduino.h>
#include <FS.h> 
#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>

time_t createTime(int year, int month, int day, int hour, int minute, int second);

class genfile {
  public:
    String path;
    String Company;
    String truePath;

    bool present;

    genfile(fs::FS &fs, String path, String Company);
    genfile(String path, String Company);


    void createDir(fs::FS &fs, String dirpath);
    bool checkFile(fs::FS &fs);
    String readFile(fs::FS &fs);
    bool writeFile(fs::FS &fs, String message);
    bool appendFile(fs::FS &fs, String toAppend);

};

class MasterList : public genfile{
  public:
    MasterList(fs::FS &fs, String path, String Company); //path = "/MasterEmployee.txt" Company = "CompanyTest2" --> truePath = "/TestCompany/CompanyTest2/Mas--"

    void localToFile(fs::FS &fs);
    void fileToLocal(fs::FS &fs);
    String returnName(fs::FS &fs, uint8_t fingerID);
    String returnEmpID(fs::FS &fs, uint8_t fingerID);
    void addName(fs::FS &fs, uint8_t fingerID, String empID, String empName);
    void removeName(fs::FS &fs, uint8_t fingerID);
  
    void resetMaster(fs::FS &fs);

    char employeeID[128][7];
    char Name[128][31];

};



class TotalAttendance : public genfile {
  public:
    genfile ToUpload;

    genfile current_attend;

    TotalAttendance(fs::FS &fs, String empID, String Company);

    void newMarked(fs::FS &fs, const char* dateMark, const char* timeMark);

    time_t addTenHours(const char * dateIN, const char * timeIN);

    void appendAttendance(fs::FS &fs, const char* dateIN, const char* timeIN, const char* dateOUT, const char* timeOUT);

    uint32_t differenceTimes(const char* dateIN, const char* timeIN, const char* dateOUT, const char* timeOUT);
};



class AttendanceSystem : public genfile{
  public:
    AttendanceSystem(fs::FS &fs, String Company);

    genfile latestID;

    uint8_t getLatestID(fs::FS &fs);
    void increaseID(fs::FS &fs);
 };

#endif
