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
    </style>
  </head>
  <body>
    <div class='container'>
      <h1>Currently MarkedIn Employees </h1>
      )rawliteral";
  
  String data = "TWB001,Erichwith20letters..,11/06/2024,12:07,TWB003,Erichwith20letters..,11/06/2024,12:07,TWB007,Erichwith20letters..,11/06/2024,12:07,TWB009,Erichwith20letters..,11/06/2024,12:07,TWB011,Erichwith20letters..,11/06/2024,12:07,TWB0016,Erichwith20letters..,11/06/2024,12:07,TWB004,Erichwith20letters..,11/06/2024,12:07,TWB004,Erichwith20letters..,11/06/2024,12:07,TWB004,Erichwith20letters..,11/06/2024,12:07,"; // add String wala function here
  pg += dataSheetMembers(data);  
  pg += R"rawliteral(
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
