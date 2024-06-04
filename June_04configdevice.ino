#include <ArduinoJson.h>
#include "RTClib.h"
#include <Adafruit_Fingerprint.h>
#include <Adafruit_LiquidCrystal.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>

#define mySerial Serial2
#define FINGERPRINT_CAPACITY 127

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS1307 rtc;

/*---KeyWords and Parameters*/
// Two Tabs "-   " Operation successful
// "Error:  " Task Failed

const char *ssid = "Realme";
const char *password = "12345678";
const char *www_username = "admin";
const char *www_password = "esp32";
const char *www_realm =
    "Custom Auth Realm"; // set realm of auth to Default:"Login Required"
String authFailResponse = "Authentication Failed";
bool foundStation = false;

String GOOGLE_SCRIPT_ID = "AKfycbzsswdsP0DcNJtddG-JTp-OgB0HzmnVdx8lrseqiF8Cwfdujxj3c3aCm8U_W5NP8ocWDw";
WebServer server(80);

// uint8_t ID = finger.templateCount+1;
uint8_t
    ID_; // letter we will scan this number from a file stored in the sd card

/*----Function Definitions----*/
void setupWiFi();
void setupOTA();
void initializeRTC();
void handleRoot();
void handleHome();
void handleConfigureDevice();
void handleSaveConfig();
void handleDeleteAllFingerprints(); 
void handleEnrollUser();
void handleDeleteUser();
void handleCheckDataSheet();
void handleLogout();
void deleteFingerprint(int id);
uint8_t getFingerprintEnroll();
String dateToday();
String timeNow();
void uploadToExcel(uint8_t fingid,String time);

/*-----------------------------------------------------------------------*/
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
          <label for='ssid'>SSID:</label>
          <input type='text' id='ssid' name='ssid'>
        </div>
        <div class='form-group'>
          <label for='password'>Password:</label>
          <input type='password' id='password' name='password'>
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
            ssid: document.getElementById('ssid').value,
            password: document.getElementById('password').value,
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
          document.getElementById('ssid').value = '';
          document.getElementById('password').value = '';
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

    String ssid = jsonDoc["ssid"];
    String password = jsonDoc["password"];
    String googleUrl = jsonDoc["googleUrl"];

    // Print the configuration to the Serial Monitor
    Serial.println("Configuration Saved:");
    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);
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
  lcd.setCursor(0,1);
  lcd.print("Successfully deleted");
  lcd.setCursor(0,2);
  lcd.print("All Fingerprints");
  server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"All stored fingerprints deleted successfully\"}");
}
//---------------------------------------------------------------------------

void handleCheckDataSheet() {
  server.send(200, "text/html", "Check data sheet page");
}
//----------------------------------------------------------------------
void handleRoot() {
  if (!server.authenticate(www_username,
                           www_password)) { // if authentication fails
    server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);
    return; // Exit function
  }
  server.sendHeader("Location", "/Home"); // Redirect to home page
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
          <a class='button' href='/CheckDataSheet'>Check DataSheet</a>
          <a class='button logout' href='/Logout'>Logout</a>
      </div>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", page);
}
//----------------------------------------------------------------------

void handleDeleteUser() {
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
        max-width: 600px;
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
    </style>
  </head>
  <body>
    <div class='container'>
      <h1>Delete User</h1>
      <form action='/DeleteUser' method='POST'>
        <label for='id'>Employee ID:</label>
        <input type='number' id='id' name='id' required>
        <button type='submit'>Delete Fingerprint</button>
      </form>
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

void handleEnrollUser() {
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
            input[type='text'], input[type='number'] {
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
        </style>
    </head>
    <body>
        <div class='container'>
            <h1>Enroll New User</h1>
            <p>Current number of fingerprints stored: )" + String(finger.templateCount) + "( / 127</p>
            <form action='/EnrollUser' method='POST'>
                <label for='name'>Employee Name:</label>
                <input type='text' id='name' name='name' required>
                <label for='id'>Fingerprint ID (1-127):</label>
                <input type='number' id='id' name='id' min='1' max='127' required>
                <button type='submit'>Scan Fingerprint</button>
            </form>
            <a href='/Home' class='back-button'>Back to Home</a>
        </div>
    </body>
    </html>
    )";


  if (server.method() == HTTP_POST) {
    String name = server.arg("name");
    String idStr = server.arg("id");
    int id = idStr.toInt();
    if (name.length() > 0 && id > 0 && id <= 127) {
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
      ID_ = id;
      getFingerprintEnroll();
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
uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(ID_);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enrolling as #");
  lcd.print(ID_);
  lcd.setCursor(0,1);
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
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("Scanned ID #" + String(finger.fingerID));
  lcd.setCursor(1, 2);
  lcd.print("Attandance Marked!");
  lcd.setCursor(1,3);
  lcd.print(dateToday() + " " + timeNow());
  delay(3000);
  lcd.clear();
  lcd.print("Scanner is Active!");
  /* uploadToExcel(finger.fingerID, timeNow()); */
  return finger.fingerID;
}

void uploadToExcel(uint8_t fingid,String time){
    if(foundStation){
    String url = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+"id=" + String(fingid) + "&time=" + String(time);
    HTTPClient http;

    http.begin(url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET(); 
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    lcd.setCursor(1,3);
    lcd.print("uploaded!");
    http.end();
  }
}
//----------------------------------------------------------------------

void deleteFingerprint(int id) {

  if (finger.deleteModel(id) == FINGERPRINT_OK) {
    Serial.print("Deleted fingerprint with ID: ");
    Serial.println(id);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Deleted #" + String(id));
    delay(2000);
    lcd.clear();
    lcd.print("Scanner is Active!");
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
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();
  foundStation = false;
  while (!foundStation && millis() - startTime < 30000) {
    if (WiFi.status() == WL_CONNECTED) {
      foundStation = true;
      break;
    }
    delay(1000);
    Serial.println("Connecting to WiFi...");
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
    lcd.print("SoftAP Started!");
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
    while (1) { // Lock the code in infinite loop (until the device restarts)
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
  server.on("/CheckDataSheet", handleCheckDataSheet);
  server.on("/delete-fingerprints", HTTP_POST, handleDeleteAllFingerprints);
  server.on("/Logout", handleLogout);

  server.begin();
  Serial.println("HTTP server started");
  lcd.setCursor(0, 2);
  lcd.print("DNS: configbad.local");
}

//----------------------------------------------------------------------

void initializeRTC() {
  Wire.begin(21, 22); // SDA to GPIO 21 and SCL to GPIO 22

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
    day = '0' + day; // always two digits
  if (month.length() < 2)
    month = '0' + month;

  String date = day + "_" + month + "_" + year;
  return date;
}

//----------------------------------------------------------------------
String timeNow() {
  DateTime now = rtc.now();
  String ampm = "AM";
  int hour = now.hour();
  // Convert to 12-hour format
  if (hour == 0) {
    hour = 12;
  } else if (hour == 12) {
    ampm = "PM"; // Noon case
  } else if (hour > 12) {
    hour -= 12;
    ampm = "PM";
  }
  String time = String(hour) + ":" + String(now.minute()) + " " + ampm;
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
    ; // For Yun/Leo/Micro/Zero/...
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
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scanner is Active!");
}

void loop() {
  getFingerprintID();
  server.handleClient();
  delay(10);
}
