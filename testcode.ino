#include "SDClass.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <ArduinoJson.h>  // large ig :(
#include "RTClib.h"
#include <Adafruit_Fingerprint.h>    // 33KB
#include <Adafruit_LiquidCrystal.h>  // 21KB
#include <ESPmDNS.h>
#include <HTTPClient.h>         // 40KB
#include <LiquidCrystal_I2C.h>  // 12KB
#include <Update.h>
#include <WebServer.h>  // 36 KB
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Arduino.h>
#include <ESP_Google_Sheet_Client.h>  // 140KB
#include <time.h>


#define PROJECT_ID "biometric-attendance-device"
#define CLIENT_EMAIL "bad-001@biometric-attendance-device.iam.gserviceaccount.com"

#define SIZE_OF_DATA_ARRAY 62
#define SDA_ID 6  // location for delimiters (",")
#define SDA_NAME 27
#define SDA_IND 38
#define SDA_INT 44
#define SDA_OUTD 55
#define SDA_OUTT 61
#define MAX_FINGER_TRY 3

#define mySerial Serial2
#define FINGERPRINT_CAPACITY 127


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS1307 rtc;

MasterList *employeeList;
AttendanceSystem *OverallAttendance;



/*---KeyWords and Parameters*/
// Two Tabs "-   " Operation successful
// "Error:  " Task Failed

String ssid_;
String password_;
String wwwid_;
String wwwpass_;
String gsheetUrl_;
int companyid_;

String ssidPath = "/ssid.txt";
String passwordPath = "/password.txt";
String wwwidPath = "/wwwid.txt";
String wwwpassPath = "/wwwpass.txt";
String gsheetUrlPath = "/gsheetUrl.txt";
String companyidPath = "/companyid.txt";

const String DEFAULT_SSID = "ioT_weB";
const String DEFAULT_PASSWORD = "12341234";
const String DEFAULT_WWWID = "admin";
const String DEFAULT_WWWPASS = "esp32";
const String DEFAULT_GSHEETURL = "1kjVsjo8GPZHqmcbIkiuYRqXgevBSdq4YZchCUihwx14";
const int DEFAULT_COMPANYID = 20;

String CompanyName = "Company" + String(DEFAULT_COMPANYID);

const char *www_realm = "Custom Auth Realm";  // set realm of auth to Default:"Login Required"
String authFailResponse = "Authentication Failed";
bool foundStation = false;
bool ntpReadyToUpload = false;
bool softAPOn = false;
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDvgnF1lbw8R1MX\nynR/DKbFnsc0tlPei/3J4bWiYoerg5EG7+1Z5Um3APF6QNh1USoMiP7kerCLkaOs\npnF2XAbdTg9BGT0M8uuhIHJkl53VaLzEPuw2onAk1xGsbNr+Dbk8TSW697cLfJ+G\ng3i1Yzj1UUOzxzXaKjDXWDe6Th7ONLxBY+9wR1b7gzLKrJs03X+4sbY/vDA3WljL\nDlULNKIkkcE3GxtHLE1hliXcFp69YtZeIEBpBUnMwBvU8WXJuN594nBnJW/DWsSO\nk5yG4aVIU/v7oZM3Hb1IVthUZS8fYazmj6yX4nJbxid3RlzW66tvVyEYuVaUVUee\nEb7vWZRLAgMBAAECggEAMFMKX2Ypx6PAcTFpFeVQlNj3anxLKcDnIN+IjU4B/6oZ\nDeRE6HlpySHIlxkUU39f042kmCblDhQBiSlABViGDgrc6w6TkdRFlIn+aqtorhFm\n3NWxRUi7BcPbDgYcnVvZfwA116Z5aTjo2vQV28s8R8rwtQpBpFYU6q4i+FUgxAvd\n7apCZP69UwjfcaTZgaMf1u/Lf7H4SF5eML5FnV7EROUz3cygKDj0QGX6b1b0dTmr\ngPuIkmutgvSs6Kn9jt3B9RElBYGZCLJD3FUR+75YoL6f4hjtFuFk23ym1X+X9fKl\nkK5X9bhx9GXhiCLWAon9P6J+hKqUoIYHpg6SsVsvSQKBgQD67IFEX09jF2vp9azw\n4JMuJlqYoOOky5iV+Mj6vU5lzFMfXND0Ix9XwV+EWEBSK2i/7zRHoNnT7T2cVeET\nPXOWZXcbofy1/TaXnzmyWqJ3DuY9cYcdyZ1YqAxEX0BPoRAl3bJIJ0xog+teNVC9\nucMB/FZZK03kaW4yyBRjRhIlpQKBgQD0WtNLicEZW3tFTGghTsKr+4z8mhL+Vihb\n+lF3HoR+NvSQkGHZwtJO6ITyOLZDCqCSP5/CYXCO7qpJLzRxvXB/PJUIA4GeBm5G\njmqAFR/URmocnbgMEsA6jwgRxtN+QYj3FcyYOiMpSYWGcfwplxbksuCZ8Z50VXt+\nTwGLcZAPLwKBgQDI5sM48vak6APG9qTY13X/5UV0Hp0lPL2WlfKUjy1N5CJJYg8K\n6XQW1wSa+e3F3GuqAONi1i6Rt8k0eq4SsYjySZrrzO6A55PmId4YJukdWhiN6W+9\nMceBTEtGEt1y5AoBx1cO3igXJkg0zYsv3KN924NnMXzT/HX2wqtQMLIm4QKBgQDy\nChVJuYXALFGKbwimFXHXETCx9Zviukz37pdLOrtamezeVWXXYZcX+lHV7D79YUV0\nF/mTfRkVO2sJWuzDzTXlkoi8i1yUvyH0WZU8kbAZgDYTalan/trSJmOD2EIdyaR1\nuu4Ry2SQvPa6GaYBPFu85UJ8ukyORy4wCuooybA8mQKBgQDiAyv3f9VDppgI12LX\nJol//+3tzL56xCKnwGBPWJpbqUKEcFR+dwAfxmoN73eD655tjSE9WXAHISyCHIkJ\npnqMRXJ2w2Wl5bbC7Fta5Nh8xWiWY5UdOcGWzfKzId+P/ucJmzKc0ZIuBgCA1fNc\nxZ5uT3w9cMunPd0qJG214xM4Dg==\n-----END PRIVATE KEY-----\n";


WebServer server(80);

uint8_t tryNum = 0;
unsigned long lastUpTime = 0;
unsigned long timerDelay = 60000;  // change
unsigned long softapstartmilli = 0;
const unsigned long reconnectTimer = 30000;  // change

/*----Function Definitions----*/
void setupWiFi();
void setupOTA();
void initializeVar();
void initializeRTC();
void tokenStatusCallback(TokenInfo info);

void handleRoot();
void handleHome();

void handleConfigureDevice();
void handleSaveConfig();
void handleDeleteAllFingerprints();

void handleEnrollUser();
void handleDeleteUser();
void handleLogout();

void uploaddd(char *data);
void deleteFingerprint(int id);
uint8_t getFingerprintEnroll();

String dateToday();
String timeNow();


/*-----------------------------------------------------------------------*/
void initializeVar() {


  genfile ssidf(ssidPath, "Settings");
  ssidf.createDir(SD, "/TestCompany/Settings");
  if (!ssidf.checkFile(SD)) {
    ssidf.writeFile(SD, DEFAULT_SSID);
  }
  ssid_ = ssidf.readFile(SD);

  genfile passwordf(passwordPath, "Settings");
  if (!passwordf.checkFile(SD)) {
    passwordf.writeFile(SD, DEFAULT_PASSWORD);
  }
  password_ = passwordf.readFile(SD);

  genfile wwwidf(wwwidPath, "Settings");
  if (!wwwidf.checkFile(SD)) {
    wwwidf.writeFile(SD, DEFAULT_WWWID);
  }
  wwwid_ = wwwidf.readFile(SD);

  genfile wwwpassf(wwwpassPath, "Settings");
  if (!wwwpassf.checkFile(SD)) {
    wwwpassf.writeFile(SD, DEFAULT_WWWPASS);
  }
  wwwpass_ = wwwpassf.readFile(SD);

  genfile gsheetUrlf(gsheetUrlPath, "Settings");
  if (!gsheetUrlf.checkFile(SD)) {
    gsheetUrlf.writeFile(SD, DEFAULT_GSHEETURL);
  }
  gsheetUrl_ = gsheetUrlf.readFile(SD);

  genfile companyid(companyidPath, "Settings");
  if (!companyid.checkFile(SD)) {
    companyid.writeFile(SD, String(DEFAULT_COMPANYID));
  }
  companyid_ = companyid.readFile(SD).toInt();
  Serial.println("companyid_" + " " + String(companyid_));
}

void tokenStatusCallback(TokenInfo info) {
  if (info.status == token_status_error) {
    GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
  } else {
    GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
  }
}

void updateAttendance(int empID) {
  String empIDStr = "TWB" + String(empID / 100) + String((empID % 100) / 10) + String(empID % 10);
  TotalAttendance tempObj(SD, empIDStr, CompanyName);
  int toRead = tempObj.ToUpload.readFile(SD).toInt();
  if (toRead) {
    for (int i = toRead; i > 0; i--) {

      char data[62] = "";

      for (int j = 0; j < 6; j++) {
        data[j] = empIDStr[j];
      }
      data[6] = ',';
      String name = employeeList->returnName(SD, empID);
      int lengthn = name.length();
      for (int j = 0; j < (20 < lengthn ? 20 : lengthn); j++) {
        data[7 + j] = name[j];
      }
      while (lengthn < 20) {
        data[7 + lengthn] = ' ';
        lengthn++;
      }
      data[27] = ',';
      File file = SD.open(tempObj.path);
      int totalsize = file.size();
      file.seek(totalsize - 34 * i);
      String currRead;
      for (int j = 0; j < 34; j++) {
        currRead += char(file.read());
      }
      for (int j = 0; j < 10; j++) {
        data[28 + j] = currRead[j];
      }
      data[38] = ',';
      for (int j = 0; j < 5; j++) {
        data[39 + j] = currRead[11 + j];
      }
      data[44] = ',';
      for (int j = 0; j < 10; j++) {
        data[45 + j] = currRead[17 + j];
      }
      data[55] = ',';
      for (int j = 0; j < 5; j++) {
        data[56 + j] = currRead[28 + j];
      }
      uploaddd(data);
    }
    tempObj.ToUpload.writeFile(SD, "0");
  }
}

void uploaddd(char *data) {
  char employeeID[6 + 1];
  char name[20 + 1];
  char inDate[10 + 1];
  char inTime[5 + 1];
  char outDate[10 + 1];
  char outTime[5 + 1];

  for (int i = 0; i < SIZE_OF_DATA_ARRAY; i++) {
    if (i == SDA_ID || i == SDA_NAME || i == SDA_IND || i == SDA_INT || i == SDA_OUTD || i == SDA_OUTT) {  //Location of ","
      continue;                                                                                            //ignore
    } else if (i < SDA_ID) {
      employeeID[i] = data[i];
    } else if (i < SDA_NAME) {
      name[i - SDA_ID - 1] = data[i];
    } else if (i < SDA_IND) {
      inDate[i - SDA_NAME - 1] = data[i];
    } else if (i < SDA_INT) {
      inTime[i - SDA_IND - 1] = data[i];
    } else if (i < SDA_OUTD) {
      outDate[i - SDA_INT - 1] = data[i];
    } else if (i < SDA_OUTT) {
      outTime[i - SDA_OUTD - 1] = data[i];
    }
  }
  employeeID[6] = '\0';  //null-terminate
  name[20] = '\0';
  inDate[10] = '\0';
  inTime[5] = '\0';
  outDate[10] = '\0';
  outTime[5] = '\0';

  FirebaseJson response;
  FirebaseJson jsonObj;

  jsonObj.add("majorDimension", "ROWS");
  jsonObj.set("values/[0]/[0]", employeeID);
  jsonObj.set("values/[0]/[1]", name);
  jsonObj.set("values/[0]/[2]", inDate);
  jsonObj.set("values/[0]/[3]", inTime);
  jsonObj.set("values/[0]/[4]", outDate);
  jsonObj.set("values/[0]/[5]", outTime);

  bool success = GSheet.values.append(&response, gsheetUrl_, "EntriesLog!A1", &jsonObj);
  if (success) {
    response.toString(Serial, true);
    jsonObj.clear();
  } else {
    Serial.println(GSheet.errorReason());
  }
  Serial.println();
  Serial.println(ESP.getFreeHeap());  // remove it later
}

//-------------------------------------------------------------------------
void handleConfigureDevice() {
  String page = R"rawliteral(
    <!DOCTYPE html>
    <html lang='en'>
    <head>
      <meta charset='UTF-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>
      <title>Config Settings</title>
      <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 0; display: flex; flex-direction: column; align-items: center; }
        .container { width: 80%; max-width: 800px; margin: 20px auto; border: 1px solid #ccc; padding: 20px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }
        .form-group { margin-bottom: 15px; }
        .form-group label { display: block; margin-bottom: 5px; }
        .form-group input { width: 100%; padding: 8px; box-sizing: border-box; }
        .buttons { display: flex; justify-content: space-between; }
        .buttons button { padding: 10px 20px; border: none; cursor: pointer; }
        .buttons button.save { background-color: #4CAF50; color: white; }
        .buttons button.reset { background-color: #f44336; color: white; }
        .delete-container { margin-top: 20px; text-align: center; }
        .delete-container button { padding: 10px 20px; background-color: #FF4136; color: white; border: none; border-radius: 5px; cursor: pointer; }
        .delete-container button:hover { background-color: #e3342f; }
        .home-button-container { margin-top: 20px; text-align: center; }
        .home-button-container button { padding: 10px 20px; background-color: #007BFF; color: white; border: none; border-radius: 5px; cursor: pointer; }
        .home-button-container button:hover { background-color: #0056b3; }
      </style>
    </head>
    <body>
      <div class='container'>
        <h2>Config Settings</h2>
        <div class='form-group'>
          <label for='wifi-ssid'>WiFi SSID:</label>
          <input type='text' id='wifi-ssid' name='wifi-ssid'>
        </div>
        <div class='form-group'>
          <label for='wifi-password'>WiFi Password:</label>
          <input type='password' id='wifi-password' name='wifi-password'>
        </div>
        <div class='form-group'>
          <label for='login-id'>Login ID:</label>
          <input type='text' id='login-id' name='login-id'>
        </div>
        <div class='form-group'>
          <label for='login-password'>Login Password:</label>
          <input type='password' id='login-password' name='login-password'>
        </div>
        <div class='form-group'>
          <label for='google-url'>Google Sheets ID:</label>
          <input type='text' id='google-url' name='google-url'>
        </div>
        <div class='buttons'>
          <button class='save' onclick='saveConfig()'>Save</button>
          <button class='reset' type='reset' onclick='resetConfig()'>Reset</button>
        </div>
      </div>
      <div class='delete-container'>
        <button onclick='confirmDelete()'>Delete All Stored Fingerprints</button>
      </div>
      <div class='home-button-container'>
        <button onclick='goHome()'>Back to Home</button>
      </div>

      <script>
        function saveConfig() {
          const config = {
            wifiSsid: document.getElementById('wifi-ssid').value,
            wifiPassword: document.getElementById('wifi-password').value,
            loginId: document.getElementById('login-id').value,
            loginPassword: document.getElementById('login-password').value,
            googleUrl: document.getElementById('google-url').value,
          };

          fetch('/save-config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(config),
          })
          .then(response => response.json())
          .then(data => {
            console.log('Success:', data);
            alert('Configuration saved successfully!');
          })
          .catch((error) => {
            console.error('Error:', error);
            alert('Failed to save configuration.');
          });
        }

        function resetConfig() {
          document.getElementById('wifi-ssid').value = '';
          document.getElementById('wifi-password').value = '';
          document.getElementById('login-id').value = '';
          document.getElementById('login-password').value = '';
          document.getElementById('google-url').value = '';
        }

        function confirmDelete() {
          if (confirm('Are you sure you want to delete all stored fingerprints? This action is irreversible.')) {
            fetch('/delete-fingerprints', {
              method: 'POST'
            })
            .then(response => response.json())
            .then(data => {
              console.log('Success:', data);
              alert('All stored fingerprints deleted successfully!');
            })
            .catch((error) => {
              console.error('Error:', error);
              alert('Failed to delete stored fingerprints.');
            });
          }
        }
        
        function goHome() {
          window.location.href = '/Home';
        }
      </script>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", page);
}

void handleSaveConfig() {
  if (server.method() == HTTP_POST) {
    StaticJsonDocument<512> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, server.arg("plain"));

    if (error) {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
      return;
    }

    String ssid = jsonDoc["wifiSsid"];
    String password = jsonDoc["wifiPassword"];
    String wwwid = jsonDoc["loginId"];
    String wwwpass = jsonDoc["loginPassword"];
    String googleUrl = jsonDoc["googleUrl"];

    if (!ssid.isEmpty()) {
      genfile ssidf(SD, ssidPath, "Settings");
      ssidf.writeFile(SD, ssid);
      ssid_ = ssid;
    }

    if (!password.isEmpty()) {
      genfile passwordf(SD, passwordPath, "Settings");
      passwordf.writeFile(SD, password);
      password_ = password;
    }

    if (!wwwid.isEmpty()) {
      genfile wwwidf(SD, wwwidPath, "Settings");
      wwwidf.writeFile(SD, wwwid);
      wwwid_ = wwwid;
    }

    if (!wwwpass.isEmpty()) {
      genfile wwwpassf(SD, wwwpassPath, "Settings");
      wwwpassf.writeFile(SD, wwwpass);
      wwwpass_ = wwwpass;
    }

    if (!googleUrl.isEmpty()) {
      genfile gsheetUrlf(SD, gsheetUrlPath, "Settings");
      gsheetUrlf.writeFile(SD, googleUrl);
      gsheetUrl_ = googleUrl;
    }

    // Print the configuration to the Serial Monitor
    Serial.println("Configuration Saved:");
    if (!ssid.isEmpty()) Serial.println("SSID: " + ssid);
    if (!password.isEmpty()) Serial.println("Password: " + password);
    if (!wwwid.isEmpty()) Serial.println("Login ID: " + wwwid);
    if (!wwwpass.isEmpty()) Serial.println("Login Password: " + wwwpass);
    if (!googleUrl.isEmpty()) Serial.println("Google Sheets URL: " + googleUrl);

    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Configuration saved successfully\"}");
  } else {
    server.send(405, "application/json", "{\"status\":\"error\",\"message\":\"Method not allowed\"}");
  }
}
void handleDeleteAllFingerprints() {
  finger.emptyDatabase();
  genfile companyid(companyidPath, "Settings");
  int temp = companyid.readFile(SD).toInt();

  temp = temp + 1;

  companyid.writeFile(SD, String(temp));

  Serial.println(temp);
  Serial.println(companyid.readFile(SD));

  Serial.print("Deleted All Finger Prints");
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Successfully deleted");
  lcd.setCursor(0, 2);
  lcd.print("All Fingerprints");
  server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"All stored fingerprints deleted successfully\"}");

  ESP.restart();
}

//-------------------------------------------------------------------------

void handleRoot() {
  if (!server.authenticate(wwwid_.c_str(), wwwpass_.c_str())) {  // if authentication fails
    server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);
    return;  // Exit function
  }
  server.sendHeader("Location", "/Home");  // Redirect to home page
  server.send(303);
}
//-------------------------------------------------------------------------

void handleHome() {
  String page = R"rawliteral(
  <!DOCTYPE html>
  <html lang='en'>
  <head>
      <meta charset='UTF-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>
      <title>Home</title>
      <style>
          body {
              font-family: Arial, sans-serif;
              background-color: #f4f4f9;
              margin: 0;
              padding: 0;
              display: flex;
              flex-direction: column;
              align-items: center;
              height: 100vh;
              text-align: center;
          }
          h1 {
              margin-bottom: 20px;
              font-size: 24px;
              color: #333;
          }
          .button {
              display: inline-block;
              width: 150px;
              height: 40px;
              margin: 10px 5px;
              font-size: 16px;
              text-align: center;
              line-height: 40px;
              background-color: #4CAF50;
              color: white;
              text-decoration: none;
              border-radius: 10px;
              transition: background-color 0.3s;
          }
          .button:hover, .button:focus {
              background-color: #45a049;
          }
          .logout {
              background-color: #f44336;
          }
          .logout:hover, .logout:focus {
              background-color: #e3342f;
          }
          .container {
              padding: 20px;
              max-width: 800px;
              width: 100%;
          }
          @media (max-width: 600px) {
              .button {
                  width: 120px;
                  font-size: 14px;
              }
          }
          table {
              width: 100%;
              border-collapse: collapse;
              margin-top: 20px;
              font-size: 14px;
          }
          th, td {
              padding: 8px;
              border: 1px solid #ddd;
              text-align: left;
          }
          th {
              background-color: #f2f2f2;
          }
          .refresh-button {
              display: inline-block;
              margin-top: 20px;
              text-align: center;
              text-decoration: none;
              color: #fff;
              padding: 10px 20px;
              border-radius: 4px;
              background-color: #28a745;
          }
          .refresh-button:hover {
              background-color: #218838;
          }
      </style>
  </head>
  <body>
      <h1>Biometric Attendance Device</h1>
      <div class='container'>
          <a class='button' href='/ConfigureDevice'>Configure Device</a>
          <a class='button' href='/EnrollUser'>Enroll User</a>
          <a class='button' href='/DeleteUser'>Delete User</a>
          <a class='button logout' href='/Logout'>Logout</a>
          <h2>Currently MarkedIn Employees</h2>
  )rawliteral";

  // Fetch and display the currently marked-in employees
  String data = OverallAttendance->returnCurrentIn(SD);
  int entries = data.length() / 45;
  for (int i = 0; i < entries; i++) {
    int fingerID = (String(data[45 * i + 3]) + String(data[45 * i + 4]) + String(data[45 * i + 5])).toInt();
    String name = employeeList->returnName(SD, fingerID);
    for (int j = 0; j < (20 < name.length() ? 20 : name.length()); j++) {
      data[45 * i + 7 + j] = name[j];
    }
  }
  page += dataSheetMembers(data);
  page += R"rawliteral(
          <a href='/Home' class='refresh-button'>Refresh</a>
      </div>
  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", page);
}

String dataSheetMembers(String data) {
  char employeeID[7];  // x characters + null terminator
  char name[21];
  char inDate[11];
  char inTime[6];
  int totalEntries = data.length() / 45;
  int count = 0;
  String pg = "<table><tr><th>Employee ID</th><th>Name</th><th>In Date</th><th>In Time</th></tr>";

  for (int i = 0; i <= SDA_INT && count < totalEntries; i++) {
    if (i == SDA_INT) {
      employeeID[6] = '\0';  // null-terminate
      name[20] = '\0';
      inDate[10] = '\0';
      inTime[5] = '\0';
      pg += "<tr><td>" + String(employeeID) + "</td><td>" + String(name) + "</td><td>" + String(inDate) + "</td><td>" + String(inTime) + "</td></tr>";
      i = -1;
      count++;
      continue;
    } else if (i == SDA_ID || i == SDA_NAME || i == SDA_IND) {  // Location of ","
      continue;                                                 // ignore
    } else if (i < SDA_ID) {
      employeeID[i] = data[i + count * 45];
    } else if (i < SDA_NAME) {
      name[i - SDA_ID - 1] = data[i + count * 45];
    } else if (i < SDA_IND) {
      inDate[i - SDA_NAME - 1] = data[i + count * 45];
    } else if (i < SDA_INT) {
      inTime[i - SDA_IND - 1] = data[i + count * 45];
    }
  }
  pg += "</table>";
  return pg;
}

//-------------------------------------------------------------------------

String getPageStr() {
  String page = "<table>";
  page += "<tr> <th> Fingerprint ID</th> <th> Employee Name </th> <th> Employee ID</th> </tr>";
  for (int i = 1; i <= FINGERPRINT_CAPACITY; i++) {
    if (employeeList->returnName(SD, i) == "")
      continue;  // if empty continue

    page += "<tr> <td>" + String(i) + "</td> <td>" + employeeList->returnName(SD, i) + "</td> <td>" + employeeList->returnEmpID(SD, i) + "</td></tr>";
  }
  page += "</table>";
  return page;
}
void handleDeleteUser() {
  String userList = getPageStr();
  String page = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>Delete User</title>
    <style>
      body {
        font-family: Arial, sans-serif;
        background-color: #f4f4f9;
        margin: 0;
        padding: 20px;
      }
      h1 {
        text-align: center;
        color: #333;
      }
      .container {
        max-width: 800px;
        margin: 0 auto;
        padding: 20px;
        background-color: #fff;
        border-radius: 8px;
        box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
      }
      form {
        display: flex;
        flex-direction: column;
        align-items: center;
      }
      label {
        margin-bottom: 8px;
        font-weight: bold;
      }
      input[type='number'] {
        padding: 10px;
        margin-bottom: 20px;
        border: 1px solid #ccc;
        border-radius: 4px;
        font-size: 1em;
        width: 100%;
        max-width: 300px;
      }
      button {
        padding: 10px 20px;
        background-color: #FF4136;
        color: white;
        border: none;
        border-radius: 4px;
        cursor: pointer;
        font-size: 1em;
      }
      button:hover {
        background-color: #e3342f;
      }
      .back-button {
        display: block;
        margin-top: 20px;
        text-align: center;
        text-decoration: none;
        color: #fff;
        background-color: #007BFF;
        padding: 10px 20px;
        border-radius: 4px;
      }
      .back-button:hover {
        background-color: #0056b3;
      }
      table {
        width: 100%;
        border-collapse: collapse;
        margin-top: 20px;
      }
      th, td {
        padding: 12px;
        border: 1px solid #ddd;
        text-align: left;
      }
      th {
        background-color: #f2f2f2;
      }
      .error-message {
        color: red;
        font-size: 1em;
        margin-bottom: 20px;
      }
    </style>
    <script>
      function validateForm() {
        const id = document.getElementById('id').value;
        const validIds = Array.from(document.querySelectorAll('table tr td:first-child')).map(td => td.textContent);
        const errorMessage = document.getElementById('error-message');

        if (!validIds.includes(id)) {
          errorMessage.textContent = 'Invalid Finger Print ID. Please enter a valid ID.';
          return false;
        }

        errorMessage.textContent = '';
        return true;
      }
    </script>
  </head>
  <body>
    <div class='container'>
      <h1>Delete User</h1>
      <form onsubmit='return validateForm()' action='/DeleteUser' method='POST'>
        <label for='id'>Employee ID:</label>
        <input type='number' id='id' name='id' required>
        <div id='error-message' class='error-message'></div>
        <button type='submit'>Delete Fingerprint</button>
      </form>
      <h2>Active Users</h2>
      )rawliteral"
                + userList
                + R"rawliteral(
      <a href='/Home' class='back-button'>Back to Home</a>
    </div>
  </body>
  </html>
  )rawliteral";

  if (server.method() == HTTP_POST) {
    int id = server.arg("id").toInt();
    if (id > 0) {
      String page2 = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Deleting Fingerprint</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          background-color: #f4f4f9;
          margin: 0;
          padding: 0;
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
          text-align: center;
        }
        .container {
          background-color: #fff;
          padding: 20px 40px;
          border-radius: 10px;
          box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
        }
        h1 {
          color: #333;
          font-size: 1.5em;
          margin-bottom: 20px;
        }
        p {
          color: #666;
          font-size: 1.2em;
          margin-bottom: 20px;
        }
        a {
          display: inline-block;
          padding: 10px 20px;
          background-color: #007BFF;
          color: white;
          text-decoration: none;
          border-radius: 5px;
          transition: background-color 0.3s;
        }
        a:hover {
          background-color: #0056b3;
        }
      </style>
    </head>
    <body>
      <div class='container'>
        <h1>Deleting Fingerprint...</h1>
        <p>Finger Deletion started. Check Display.</p>
        <a href='/Home'>Click here to return to home</a>
      </div>
    </body>
    </html>
    )rawliteral";

      server.send(200, "text/html", page2);
      deleteFingerprint(id);
    } else {
      server.send(200, "text/html",
                  "<html><body><h1>Failed to delete. Please try again.<a href='/Home'>Back to Home</a></h1></body></html>");
    }
  } else {
    server.send(200, "text/html", page);
  }
}
//-------------------------------------------------------------------------
void handleLogout() {
  // Handle the Logout action
  String page = R"rawliteral(
    <!DOCTYPE html>
    <html lang='en'>
    <head>
        <meta charset='UTF-8'>
        <meta name='viewport' content='width=device-width, initial-scale=1.0'>
        <title>Logged Out</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                background-color: #f4f4f4;
                margin: 0;
                display: flex;
                justify-content: center;
                align-items: center;
                height: 100vh;
            }
            .container {
                text-align: center;
                background: #fff;
                padding: 30px;
                border-radius: 10px;
                box-shadow: 0 0 20px rgba(0, 0, 0, 0.2);
                max-width: 400px;
                width: 90%;
            }
            h1 {
                margin-bottom: 20px;
                color: #007BFF;
            }
            p {
                margin-bottom: 30px;
            }
            .button {
                display: inline-block;
                padding: 12px 24px;
                font-size: 18px;
                color: #fff;
                background: #007BFF;
                border: none;
                border-radius: 5px;
                text-decoration: none;
                transition: background 0.3s ease;
            }
            .button:hover {
                background: #0056b3;
            }
        </style>
    </head>
    <body>
        <div class='container'>
            <h1>Logged out successfully</h1>
            <p>Click below to login again:</p>
            <a href='/' class='button'>Login Again</a>
        </div>
    </body>
    </html>
    )rawliteral";
  server.sendHeader("Set-Cookie", "session=; Max-Age=0; path=/; HttpOnly");
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", page);
}
//-------------------------------------------------------------------------
void handleEnrollUser() {
  uint8_t latestEmpID = OverallAttendance->getLatestID(SD);

  String page = R"(
  <!DOCTYPE html>
  <html lang='en'>
  <head>
      <meta charset='UTF-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>
      <title>Enroll User</title>
      <style>
          body {
              font-family: Arial, sans-serif;
              background-color: #f4f4f9;
              margin: 0;
              padding: 20px;
          }
          h1 {
              text-align: center;
              color: #333;
          }
          .container {
              max-width: 600px;
              margin: 0 auto;
              padding: 20px;
              background-color: #fff;
              border-radius: 8px;
              box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
          }
          p {
              text-align: center;
              font-size: 1.2em;
              color: #666;
          }
          form {
              display: flex;
              flex-direction: column;
          }
          label {
              margin-bottom: 8px;
              font-weight: bold;
          }
          input[type='text'] {
              padding: 10px;
              margin-bottom: 20px;
              border: 1px solid #ccc;
              border-radius: 4px;
              font-size: 1em;
          }
          button {
              padding: 10px;
              background-color: #4CAF50;
              color: white;
              border: none;
              border-radius: 4px;
              cursor: pointer;
              font-size: 1em;
          }
          button:hover {
              background-color: #45a049;
          }
          .back-button {
              display: block;
              margin-top: 20px;
              text-align: center;
              text-decoration: none;
              color: #fff;
              background-color: #007BFF;
              padding: 10px 20px;
              border-radius: 4px;
          }
          .back-button:hover {
              background-color: #0056b3;
          }
          .enroll-info {
              text-align: center;
              font-size: 1.2em;
              color: #333;
              margin-bottom: 20px;
          }
      </style>
      <script>
          function validateForm() {
              const nameInput = document.getElementById('name').value;
              if (nameInput.length > 20) {
                  alert('Name should not exceed 20 characters.');
                  return false;
              }
              return true;
          }
      </script>
  </head>
  <body>
      <div class='container'>
          <h1>Enroll New User</h1>
          <p>Current number of fingerprints stored: )"
                + String(finger.templateCount) + R"( / 127</p>
          <form onsubmit='return validateForm()' action='/EnrollUser' method='POST'>
              <label for='name'>Employee Name:</label>
              <input type='text' id='name' name='name' maxlength='20' required>
              <div class='enroll-info'>Enroll as TWB)"
                + String(latestEmpID / 100) + String((latestEmpID % 100) / 10) + String(latestEmpID % 10) + R"(</div>
              <button type='submit'>Scan Fingerprint</button>
          </form>
          <a href='/Home' class='back-button'>Back to Home</a>
      </div>
  </body>
  </html>
  )";

  if (server.method() == HTTP_POST) {
    String name = server.arg("name");



    if (name.length() > 0) {
      String page2 = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Scanning Fingerprint</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          background-color: #f4f4f9;
          margin: 0;
          padding: 0;
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
          text-align: center;
        }
        .container {
          background-color: #fff;
          padding: 20px 40px;
          border-radius: 10px;
          box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
        }
        h1 {
          color: #333;
          font-size: 1.5em;
          margin-bottom: 20px;
        }
        p {
          color: #666;
          font-size: 1.2em;
          margin-bottom: 20px;
        }
        a {
          display: inline-block;
          padding: 10px 20px;
          background-color: #007BFF;
          color: white;
          text-decoration: none;
          border-radius: 5px;
          transition: background-color 0.3s;
        }
        a:hover {
          background-color: #0056b3;
        }
      </style>
    </head>
    <body>
      <div class='container'>
        <h1>Scanning Fingerprint...</h1>
        <p>Check display and follow instructions to enroll new finger.</p>
        <a href='/Home'>Click here to return to home</a>
      </div>
    </body>
    </html>
    )rawliteral";

      server.send(200, "text/html", page2);

      uint8_t successCase = getFingerprintEnroll(latestEmpID);
      if (successCase == 0) {
        Serial.println("SUCCESSSSSSSSSS");
        OverallAttendance->increaseID(SD);
        String empIDStr = String(latestEmpID / 100) + String((latestEmpID % 100) / 10) + String(latestEmpID % 10);

        employeeList->addName(SD, latestEmpID, "TWB" + empIDStr, name);
      }
    } else {
      server.send(200, "text/html",
                  "<html><body><h1>Failed to enroll. Please ensure all fields "
                  "are correctly filled. <a href='/Home'>Back to "
                  "Home</a></h1></body></html>");
    }
  } else {
    server.send(200, "text/html", page);
  }
}
//-------------------------------------------------------------------------
uint8_t getFingerprintEnroll(uint8_t ID_) {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(ID_);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enrolling as #");
  lcd.print(ID_);
  lcd.setCursor(0, 1);
  lcd.print("Place finger");

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  lcd.setCursor(0, 1);
  lcd.print("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(ID_);
  p = -1;
  Serial.println("Place same finger again");
  lcd.setCursor(0, 1);
  lcd.print("Place same finger");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("ERROR Enrolling #" + String(ID_));
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("ERROR Enrolling #" + String(ID_));
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("ERROR Enrolling #" + String(ID_));
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("ERROR Enrolling #" + String(ID_));
      return p;
    default:
      Serial.println("Unknown error");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("ERROR Enrolling #" + String(ID_));
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(ID_);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("ERROR Enrolling #" + String(ID_));
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("ERROR Enrolling #" + String(ID_));
    return p;
  } else {
    Serial.println("Unknown error");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("ERROR Enrolling #" + String(ID_));
    return p;
  }

  Serial.print("ID ");
  Serial.println(ID_);
  p = finger.storeModel(ID_);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Enrolled #" + String(ID_));
    delay(2000);
    lcd.clear();
    lcd.print("Scanner is Active!");
    return p;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("ERROR Enrolling #" + String(ID_));
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("ERROR Enrolling #" + String(ID_));
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("ERROR Enrolling #" + String(ID_));
    return p;
  } else {
    Serial.println("Unknown error");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("ERROR Enrolling #" + String(ID_));
    return p;
  }

  lcd.clear();
  lcd.print("Scanner is Active!");
  return true;
}
/*---------------------------SCAN FINGERPRINT----------------------------*/
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print(
      "Sensor doesn't contain any fingerprint data. Please run the'enroll'.");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("No Fingerprints Found!");
    delay(5000);
    return p;
  }

  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      tryNum++;
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      tryNum++;
      return p;
    default:
      Serial.println("Unknown error");
      tryNum++;
      return p;
  }

  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      tryNum++;
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      tryNum++;
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      tryNum++;
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      tryNum++;
      return p;
    default:
      Serial.println("Unknown error");
      tryNum++;
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    tryNum++;
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Employee Not Found!");
    delay(2500);
    lcd.clear();
    return p;
  } else {
    Serial.println("Unknown error");
    tryNum++;
    return p;
  }

  // found a match!
  TotalAttendance tempObject(SD, "TWB" + String(finger.fingerID / 100) + String((finger.fingerID % 100) / 10) + String(finger.fingerID % 10), CompanyName);
  char dateMark[11] = "";
  char timeMark[6] = "";

  dateToday().toCharArray(dateMark, sizeof(dateMark));
  timeNow().toCharArray(timeMark, sizeof(timeMark));
  bool inorout = tempObject.newMarked(SD, dateMark, timeMark);  // update in lcd text whether it's in attendane or out


  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("Scanned ID #" + String(finger.fingerID));
  lcd.setCursor(1, 2);
  if (inorout) {
    lcd.print("IN Marked!");
  } else {
    lcd.print("OUT Marked!");
  }

  lcd.setCursor(1, 3);
  lcd.print(dateToday() + " " + timeNow());
  updateAttendance(finger.fingerID);
  delay(1000);
  scannerReady();

  /* uploadToExcel(finger.fingerID, timeNow()); */
  return finger.fingerID;
}
//-------------------------------------------------------------------------

void deleteFingerprint(int id) {

  if (finger.deleteModel(id) == FINGERPRINT_OK) {
    Serial.print("Deleted fingerprint with ID: ");
    Serial.println(id);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Deleted #" + String(id));
    delay(3000);
    scannerReady();
    employeeList->removeName(SD, id);
  } else {
    Serial.println("Error: Failed to delete fingerprint");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("ERROR: ");
    lcd.setCursor(0, 2);
    lcd.print("Failed to Delete #" + String(id));
    delay(2000);
    scannerReady();
  }
}
//-------------------------------------------------------------------------

String dateToday() {
  DateTime now = rtc.now();
  String day = String(now.day());
  String month = String(now.month());
  String year = String(now.year());

  if (day.length() < 2)
    day = '0' + day;  // always two digits
  if (month.length() < 2)
    month = '0' + month;

  String date = year + "/" + month + "/" + day;
  return date;
}
//-------------------------------------------------------------------------
String timeNow() {
  DateTime now = rtc.now();
  String time = String(now.hour() / 10) + String(now.hour() % 10) + ":" + String(now.minute() / 10) + String(now.minute() % 10);
  return time;
}
//-------------------------------------------------------------------------

void startSoftAP() {
  Serial.println("Starting SoftAP...");
  WiFi.softAP(DEFAULT_SSID, DEFAULT_PASSWORD);
  Serial.print("SoftAP IP address: ");
  Serial.println(WiFi.softAPIP());
  lcd.setCursor(0, 1);
  lcd.print("SoftAP Started!     ");
  softapstartmilli = millis();
  softAPOn = true;
}

void setupWiFi() {
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid_, password_);

  unsigned long startTime = millis();
  foundStation = false;
  softAPOn = false;
  Serial.print("Connecting to WiFi...");
  lcd.setCursor(0, 1);
  lcd.print("Connecting to WiFi..");

  while (!foundStation && millis() - startTime < 30000) {
    if (WiFi.status() == WL_CONNECTED) {
      foundStation = true;
      break;
    }
    delay(1000);
    Serial.print(".");
  }

  if (!foundStation) {
    startSoftAP();
  } else {
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
    lcd.setCursor(0, 1);
    lcd.print("Connected!          ");
    if (!ntpReadyToUpload) {
      initializeNTPAndGSheet();
    }
  }
}

void checkWiFiConnection() {
  wl_status_t wifiStatus = WiFi.status();

  if (wifiStatus == WL_DISCONNECTED || wifiStatus == WL_CONNECTION_LOST) {  // if disconnected try again
    setupWiFi();
  } else if (softAPOn && millis() - softapstartmilli > reconnectTimer) {
    WiFi.softAPdisconnect(true);  // Turn off SoftAP
    softAPOn = false;
    setupWiFi();
  }
}

void initializeNTPAndGSheet() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);

  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));

  GSheet.setTokenCallback(tokenStatusCallback);
  GSheet.setPrerefreshSeconds(10 * 60);
  GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);

  ntpReadyToUpload = true;
}

void setupOTA() {
  if (!MDNS.begin("configBAD")) {
    Serial.println("Error: Failed to setup MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("- mDNS responder started");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/Home", handleHome);
  server.on("/ConfigureDevice", handleConfigureDevice);
  server.on("/save-config", HTTP_POST, handleSaveConfig);
  server.on("/EnrollUser", handleEnrollUser);
  server.on("/DeleteUser", handleDeleteUser);
  server.on("/delete-fingerprints", HTTP_POST, handleDeleteAllFingerprints);
  server.on("/Logout", handleLogout);

  server.begin();
  Serial.println("HTTP server started");
  lcd.setCursor(0, 2);
  lcd.print("DNS: configbad.local");
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.print("I'm Alive!");
  delay(500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BootLoader Started!");

  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }

  employeeList = new MasterList(SD, "/MasterEmployee.txt", CompanyName);
  OverallAttendance = new AttendanceSystem(SD, CompanyName);
  initializeVar();
  setupWiFi();
  setupOTA();
  initializeRTC();
  updateTime();  // sync rtc time with ntp clock

  while (!Serial) {
    delay(100);
  }
  Serial.println("\n\nAdafruit finger detect test");
  finger.begin(57600);
  delay(5);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    lcd.setCursor(0, 3);
    lcd.print("Sensor Connected!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }

  delay(3000);
  scannerReady();
}
void initializeRTC() {
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}

void updateTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);

  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);

  // Convert to GMT+5:30 (IST)
  timeinfo.tm_hour += 5;
  timeinfo.tm_min += 30;

  // Normalize the time structure
  if (timeinfo.tm_min >= 60) {
    timeinfo.tm_min -= 60;
    timeinfo.tm_hour++;
  }
  if (timeinfo.tm_hour >= 24) {
    timeinfo.tm_hour -= 24;
    timeinfo.tm_mday++;
  }

  // Handle day, month, and year overflow
  const int daysInMonth[] = { 31, 28 + ((timeinfo.tm_year % 4 == 0 && timeinfo.tm_year % 100 != 0) || (timeinfo.tm_year % 400 == 0)), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  if (timeinfo.tm_mday > daysInMonth[timeinfo.tm_mon]) {
    timeinfo.tm_mday = 1;
    timeinfo.tm_mon++;
  }
  if (timeinfo.tm_mon >= 12) {
    timeinfo.tm_mon = 0;
    timeinfo.tm_year++;
  }

  // Print the converted time
  Serial.print("Current time in GMT+5:30: ");
  Serial.print(asctime(&timeinfo));

  // Update the RTC
  rtc.adjust(DateTime(
    timeinfo.tm_year + 1900,  // tm_year is years since 1900
    timeinfo.tm_mon + 1,      // tm_mon is months since January
    timeinfo.tm_mday,
    timeinfo.tm_hour,
    timeinfo.tm_min,
    timeinfo.tm_sec));
  // Serial.println(String(dateToday) + String(timeNow());
}

void tryGetFinger() {
  getFingerprintID();
  if (tryNum == MAX_FINGER_TRY) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Please Remove finger");
    delay(2000);
    lcd.setCursor(0, 2);
    lcd.print("Clean the sensor");
    delay(1000);
    tryNum = 0;  // reset the try number
    scannerReady();
  }
}

void scannerReady() {
  lcd.clear();
  lcd.print("Scanner is Active!");
}

void loop() {
  tryGetFinger();
  server.handleClient();
  delay(10);
  checkWiFiConnection();
}
