 // getNextEmployeeId
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
              <div class='enroll-info'>Enroll as TWB)"+ String(getNextEmployeeId) + R"(</div>
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
