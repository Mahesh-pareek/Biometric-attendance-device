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


#define mySerial Serial2
#define FINGERPRINT_CAPACITY 127

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS1307 rtc;

MasterList *employeeList;
AttendanceSystem *OverallAttendance;

String CompanyName = "Company13";

/*---KeyWords and Parameters*/
// Two Tabs "-   " Operation successful
// "Error:  " Task Failed

/*
char *ssid_;
char *password_;
char *wwwid_;
char *wwwpass_;

const char* ssidpath = "/ssid.txt";
const char* passwordpath = "/pass.txt";
const char* wwwidpath = "/wwwid.txt";
const char* wwwpasspath ="/wwwpass.txt";

*/

const char *ssid = "Realme";
const char *password = "12345678";
const char *wwwid = "admin";
const char *wwwpass = "esp32";
const char *www_realm = "Custom Auth Realm";  // set realm of auth to Default:"Login Required"
String authFailResponse = "Authentication Failed";
bool foundStation = false;

const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDvgnF1lbw8R1MX\nynR/DKbFnsc0tlPei/3J4bWiYoerg5EG7+1Z5Um3APF6QNh1USoMiP7kerCLkaOs\npnF2XAbdTg9BGT0M8uuhIHJkl53VaLzEPuw2onAk1xGsbNr+Dbk8TSW697cLfJ+G\ng3i1Yzj1UUOzxzXaKjDXWDe6Th7ONLxBY+9wR1b7gzLKrJs03X+4sbY/vDA3WljL\nDlULNKIkkcE3GxtHLE1hliXcFp69YtZeIEBpBUnMwBvU8WXJuN594nBnJW/DWsSO\nk5yG4aVIU/v7oZM3Hb1IVthUZS8fYazmj6yX4nJbxid3RlzW66tvVyEYuVaUVUee\nEb7vWZRLAgMBAAECggEAMFMKX2Ypx6PAcTFpFeVQlNj3anxLKcDnIN+IjU4B/6oZ\nDeRE6HlpySHIlxkUU39f042kmCblDhQBiSlABViGDgrc6w6TkdRFlIn+aqtorhFm\n3NWxRUi7BcPbDgYcnVvZfwA116Z5aTjo2vQV28s8R8rwtQpBpFYU6q4i+FUgxAvd\n7apCZP69UwjfcaTZgaMf1u/Lf7H4SF5eML5FnV7EROUz3cygKDj0QGX6b1b0dTmr\ngPuIkmutgvSs6Kn9jt3B9RElBYGZCLJD3FUR+75YoL6f4hjtFuFk23ym1X+X9fKl\nkK5X9bhx9GXhiCLWAon9P6J+hKqUoIYHpg6SsVsvSQKBgQD67IFEX09jF2vp9azw\n4JMuJlqYoOOky5iV+Mj6vU5lzFMfXND0Ix9XwV+EWEBSK2i/7zRHoNnT7T2cVeET\nPXOWZXcbofy1/TaXnzmyWqJ3DuY9cYcdyZ1YqAxEX0BPoRAl3bJIJ0xog+teNVC9\nucMB/FZZK03kaW4yyBRjRhIlpQKBgQD0WtNLicEZW3tFTGghTsKr+4z8mhL+Vihb\n+lF3HoR+NvSQkGHZwtJO6ITyOLZDCqCSP5/CYXCO7qpJLzRxvXB/PJUIA4GeBm5G\njmqAFR/URmocnbgMEsA6jwgRxtN+QYj3FcyYOiMpSYWGcfwplxbksuCZ8Z50VXt+\nTwGLcZAPLwKBgQDI5sM48vak6APG9qTY13X/5UV0Hp0lPL2WlfKUjy1N5CJJYg8K\n6XQW1wSa+e3F3GuqAONi1i6Rt8k0eq4SsYjySZrrzO6A55PmId4YJukdWhiN6W+9\nMceBTEtGEt1y5AoBx1cO3igXJkg0zYsv3KN924NnMXzT/HX2wqtQMLIm4QKBgQDy\nChVJuYXALFGKbwimFXHXETCx9Zviukz37pdLOrtamezeVWXXYZcX+lHV7D79YUV0\nF/mTfRkVO2sJWuzDzTXlkoi8i1yUvyH0WZU8kbAZgDYTalan/trSJmOD2EIdyaR1\nuu4Ry2SQvPa6GaYBPFu85UJ8ukyORy4wCuooybA8mQKBgQDiAyv3f9VDppgI12LX\nJol//+3tzL56xCKnwGBPWJpbqUKEcFR+dwAfxmoN73eD655tjSE9WXAHISyCHIkJ\npnqMRXJ2w2Wl5bbC7Fta5Nh8xWiWY5UdOcGWzfKzId+P/ucJmzKc0ZIuBgCA1fNc\nxZ5uT3w9cMunPd0qJG214xM4Dg==\n-----END PRIVATE KEY-----\n";

const char spreadsheetId[] = "1kjVsjo8GPZHqmcbIkiuYRqXgevBSdq4YZchCUihwx14";


WebServer server(80);

// uint8_t ID = finger.templateCount+1;
uint8_t ID_;  // letter we will scan this number from a file stored in the sd card


unsigned long lastUpTime = 0;
unsigned long timerDelay = 60000;  // change

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
void handleCurrentWorkingEmployees();
void handleLogout();

void uploaddd(char *data);
void deleteFingerprint(int id);
uint8_t getFingerprintEnroll();

String dateToday();
String timeNow();


/*-----------------------------------------------------------------------*/

void initializeVar() {
  /*
  String ssid_;
  String password_;
  String wwwid_;
  String wwwpass_; // Read all from the file
  */
}


void tokenStatusCallback(TokenInfo info) {
  if (info.status == token_status_error) {
    GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
  } else {
    GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
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

  bool success = GSheet.values.append(&response, spreadsheetId, "EntriesLog!A1", &jsonObj);
  if (success) {
    response.toString(Serial, true);
    jsonObj.clear();
  } else {
    Serial.println(GSheet.errorReason());
  }
  Serial.println();
  Serial.println(ESP.getFreeHeap());  // remove it later
}


// --------------------------------------------------------------------------
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
        .delete-container,  { margin-top: 20px; text-align: center; }
        .delete-container button, { padding: 10px 20px; background-color: #FF4136; color: white; border: none; border-radius: 5px; cursor: pointer; }
        .delete-container button:hover, { background-color: #e3342f; }
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
          <label for='google-url'>Google Sheets/Scripts URL:</label>
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
    String wwwid = jsonDoc["loginID"];
    String wwwpass = jsonDoc["loginPassword"];
    String googleUrl = jsonDoc["googleUrl"];



    // Print the configuration to the Serial Monitor
    Serial.println("Configuration Saved:");
    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);
    Serial.println("Www_username: " + wwwid);
    Serial.println("Www_password: " + wwwpass);
    Serial.println("Google Sheets/Scripts URL: " + googleUrl);

    // Process and save configuration as needed

    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Configuration saved successfully\"}");
  } else {
    server.send(405, "application/json", "{\"status\":\"error\",\"message\":\"Method not allowed\"}");
  }
}

void handleDeleteAllFingerprints() {
  finger.emptyDatabase();
  Serial.print("Deleted All Finger Prints");
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Successfully deleted");
  lcd.setCursor(0, 2);
  lcd.print("All Fingerprints");
  server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"All stored fingerprints deleted successfully\"}");
}
//---------------------------------------------------------------------------

void handleCheckDataSheet() {
  String pg = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>Currently MarkedIn Employees</title>
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
      .back-button, .refresh-button {
        display: inline-block;
        margin-top: 20px;
        text-align: center;
        text-decoration: none;
        color: #fff;
        padding: 10px 20px;
        border-radius: 4px;
        margin-right: 10px;
      }
      .back-button {
        background-color: #007BFF;
      }
      .back-button:hover {
        background-color: #0056b3;
      }
      .refresh-button {
        background-color: #28a745;
      }
      .refresh-button:hover {
        background-color: #218838;
      }
    </style>
  </head>
  <body>
    <div class='container'>
      <h1>Currently MarkedIn Employees</h1>
      )rawliteral";
  
  String data = OverallAttendance->returnCurrentIn(SD);  // add String wala function here
  pg += dataSheetMembers(data); 
  pg += R"rawliteral(
      <div>
        <a href='/Home' class='back-button'>Back to Home</a>
        <a href='/CheckDataSheet' class='refresh-button'>Refresh List</a>
      </div>
    </div>
  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", pg);
}

String dataSheetMembers(String data) {
  char employeeID[7];  // x characters + null terminator
  char name[21];
  char inDate[11];
  char inTime[6];
  int totalEntries = data.length() / 45;
  int count = 0;
  String pg = "<table border='1'><tr><th>Employee ID</th><th>Name</th><th>In Date</th><th>In Time</th></tr>";

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
//----------------------------------------------------------------------
void handleRoot() {
  if (!server.authenticate(wwwid, wwwpass)) {  // if authentication fails
    server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);
    return;  // Exit function
  }
  server.sendHeader("Location", "/Home");  // Redirect to home page
  server.send(303);
}
//----------------------------------------------------------------------
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
              justify-content: center;
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
              display: block;
              width: 200px;
              height: 50px;
              margin: 10px auto;
              font-size: 20px;
              text-align: center;
              line-height: 50px;
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
              position: absolute;
              top: 10px;
              right: 10px;
              background-color: #f44336;
          }
          .logout:hover, .logout:focus {
              background-color: #e3342f;
          }
          .container {
              padding: 20px;
              max-width: 600px;
              width: 100%;
          }
          @media (max-width: 600px) {
              .button {
                  width: 80%;
                  font-size: 18px;
              }
          }
      </style>
  </head>
  <body>
      <h1>Biometric Attendance Device</h1>
      <div class='container'>
          <a class='button' href='/ConfigureDevice'>Configure Device</a>
          <a class='button' href='/EnrollUser'>Enroll User</a>
          <a class='button' href='/DeleteUser'>Delete User</a>
          <a class='button' href='/CurrentWorkingEmployees'>Current Working Employees</a>
          <a class='button logout' href='/Logout'>Logout</a>
      </div>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", page);
}
//----------------------------------------------------------------------

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
      <a href='/DeleteUser' class='refresh-button'>Refresh List</a>
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

//----------------------------------------------------------------------

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
//------------------------------------------------------------------------

// getNextEmployeeId
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
  </head>
  <body>
      <div class='container'>
          <h1>Enroll New User</h1>
          <p>Current number of fingerprints stored: )"
                + String(finger.templateCount) + R"( / 127</p>
          <form action='/EnrollUser' method='POST'>
              <label for='name'>Employee Name:</label>
              <input type='text' id='name' name='name' required>
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
                  "<html><body><h1>Failed to enroll. Please ensure all field "
                  "are correctly filled. <a href='/Home'>Back to "
                  "Home</a></h1></body></html>");
    }
  } else {
    server.send(200, "text/html", page);
  }
}
//---------------------------------------------------------------------------
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
/*---------------------SCAN FINGERPRINT--------------------*/
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
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
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

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  TotalAttendance tempObject(SD, "TWB" + String(finger.fingerID / 100) + String((finger.fingerID % 100) / 10) + String(finger.fingerID % 10), CompanyName);
  char dateMark[11] = "";
  char timeMark[6] = "";

  dateToday().toCharArray(dateMark, sizeof(dateMark));
  timeNow().toCharArray(timeMark, sizeof(timeMark));
  tempObject.newMarked(SD, dateMark, timeMark);

  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("Scanned ID #" + String(finger.fingerID));
  lcd.setCursor(1, 2);
  lcd.print("Attandance Marked!");
  lcd.setCursor(1, 3);
  lcd.print(dateToday() + " " + timeNow());
  delay(3000);
  lcd.clear();
  lcd.print("Scanner is Active!");
  /* uploadToExcel(finger.fingerID, timeNow()); */
  return finger.fingerID;
}

//----------------------------------------------------------------------

void deleteFingerprint(int id) {

  if (finger.deleteModel(id) == FINGERPRINT_OK) {
    Serial.print("Deleted fingerprint with ID: ");
    Serial.println(id);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Deleted #" + String(id));
    delay(3000);
    lcd.clear();
    lcd.print("Scanner is Active!");
    employeeList->removeName(SD, id);
  } else {
    Serial.println("Error: Failed to delete fingerprint");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("ERROR: ");
    lcd.setCursor(0, 2);
    lcd.print("Failed to Delete #" + String(id));
    delay(2000);
    lcd.clear();
    lcd.print("Scanner is Active!");
  }
}
//----------------------------------------------------------------------
void setupWiFi() {
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();
  foundStation = false;
  Serial.println("Connecting to WiFi...");
  while (!foundStation && millis() - startTime < 30000) {
    if (WiFi.status() == WL_CONNECTED) {
      foundStation = true;
      break;
    }
    delay(1000);
    Serial.print(".");
    lcd.setCursor(0, 1);
    lcd.print("Connecting to WiFi..");
  }

  // If no station is found within 30 seconds, start softAP
  if (!foundStation) {
    Serial.println("Starting SoftAP...");
    WiFi.softAP(ssid, password);
    Serial.print("SoftAP IP address: ");
    Serial.println(WiFi.softAPIP());
    lcd.setCursor(0, 1);
    lcd.print("SoftAP Started!     ");
  } else {
    // Connected to a station, print its IP address
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
    lcd.setCursor(0, 1);
    lcd.print("Connected!          ");
  }
}
//---------------------------------------------------------------------------

void setupOTA() {
  if (!MDNS.begin("configBAD")) {
    Serial.println("Error:  Failed to setup MDNS responder!");
    while (1) {  // Lock the code in infinite loop (until the device restarts)
      delay(1000);
    }
  }
  Serial.println("-    mDNS responder started");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/Home", handleHome);
  server.on("/ConfigureDevice", handleConfigureDevice);
  server.on("/save-config", HTTP_POST, handleSaveConfig);
  server.on("/EnrollUser", handleEnrollUser);
  server.on("/DeleteUser", handleDeleteUser);
  server.on("/CurrentWorkingEmployees", handleCurrentWorkingEmployees);
  server.on("/delete-fingerprints", HTTP_POST, handleDeleteAllFingerprints);
  server.on("/Logout", handleLogout);

  server.begin();
  Serial.println("HTTP server started");
  lcd.setCursor(0, 2);
  lcd.print("DNS: configbad.local");


  // setup GsheetUpload
  GSheet.setTokenCallback(tokenStatusCallback);

  GSheet.setPrerefreshSeconds(10 * 60);

  GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
}

//----------------------------------------------------------------------

void initializeRTC() {
  Wire.begin(21, 22);  // SDA to GPIO 21 and SCL to GPIO 22

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }
  // Check if the RTC lost power and if so, set the time
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // Following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // To set the RTC manually, for eg. to January 1, 2024 at 12:00:00
    //  rtc.adjust(DateTime(2024, 1, 1, 12, 0, 0));
  }
}

// ---------------------------------------------------------------------
String dateToday() {
  DateTime now = rtc.now();
  String day = String(now.day());
  String month = String(now.month());
  String year = String(now.year());

  if (day.length() < 2)
    day = '0' + day;  // always two digits
  if (month.length() < 2)
    month = '0' + month;

  String date = day + "_" + month + "_" + year;
  return date;
}

//----------------------------------------------------------------------
String timeNow() {
  DateTime now = rtc.now();
  String time = String(now.hour() / 10) + String(now.hour() % 10) + ":" + String(now.minute() / 10) + String(now.minute() % 10);
  return time;
}
//----------------------------------------------------------------------

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
  setupWiFi();
  setupOTA();
  initializeRTC();
  while (!Serial)
    ;  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");
  // set the data rate for the sensor serial port
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
  // Initialize NTP
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
  initializeVar();
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scanner is Active!");

  //Starting SD Card Functions

  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  employeeList = new MasterList(SD, "/MasterEmployee.txt", CompanyName);
  OverallAttendance = new AttendanceSystem(SD, CompanyName);
}

void loop() {
  getFingerprintID();
  server.handleClient();
  delay(10);
  bool ready = GSheet.ready();

  if (ready && millis() - lastUpTime > timerDelay) {
    lastUpTime = millis();
    char *data = "TWB004,Erichwith20letters..,2024-06-11,12:07,2024-06-11,16:16";
    //provide Data
    uploaddd(data);
  }


  if (WiFi.status() != WL_CONNECTED) {  // if wifi disconnets, Start SoftAP
    foundStation = false;
    Serial.println("Starting SoftAP...");
    WiFi.softAP(ssid, password);
    Serial.print("SoftAP IP address: ");
    Serial.println(WiFi.softAPIP());
    lcd.setCursor(0, 1);
    lcd.print("SoftAP Started!     ");
    delay(1000);
  }
}
