
function updateTotalHours() {
  var ss = SpreadsheetApp.getActiveSpreadsheet();
  var masterSheet = ss.getSheetByName("MASTERSHEET");
  var data = masterSheet.getDataRange().getValues();

  console.log("Starting updateTotalHours");
  
  for (var i = 1; i < data.length; i++) { // Start from 1 to skip header row
    var employeeId = data[i][0];
    console.log("Processing employee ID: " + employeeId);
    
    var employeeSheet = ss.getSheetByName(employeeId);
    if (employeeSheet) {
      try {
        // var totalHours = computeTotalHours(employeeSheet);
        // function here
        sheet = employeeSheet;   
        var total_hours = 0;
        var total_minutes = 0;

        console.log("Computing total hours for sheet: " + sheet.getName());
        
        // Loop through each cell in column 7 (G) from row 2 to the last row
        for (var i = 2; i <= lastRow; i++) {
          var cellValue = sheet.getRange(i, 7).getValue();
          console.log("Row " + i + " value: " + cellValue);
          
          if (cellValue) {
            try {
                total_hours += cellValue.getHours();
                total_minutes += cellValue.getMinutes();
            } catch (e) {
              console.log("Error processing time value in row " + i + ": " + e.message);
            }
          }
        }

        // Handle overflow of minutes into hours
        total_hours += Math.floor(total_minutes / 60);
        total_minutes = total_minutes % 60;

        // Return the total hours as a single value (e.g., "20:30" format)
        var totalHours = total_hours + ":" + (total_minutes < 10 ? '0' + total_minutes : total_minutes);
  
        console.log("Total hours for " + employeeId + ": " + totalHours);

        // Update the total hours in the third column of MASTERSHEET
        masterSheet.getRange(i + 1, 3).setValue(totalHours);
      } catch (e) {
        console.log("Error processing sheet for employee ID " + employeeId + ": " + e.message);
      }
    } else {
      console.log("Sheet not found for employee ID: " + employeeId);
    }
  }
  
  console.log("Finished updateTotalHours");
}
