function onEntriesAdded() {
  try {
    console.log('Function triggered');
    var ss = SpreadsheetApp.getActiveSpreadsheet();
    console.log('Spreadsheet opened');

    // Check and create MASTERSHEET if it doesn't exist
    var masterSheet = ss.getSheetByName("MASTERSHEET");
    if (!masterSheet) {
      masterSheet = ss.insertSheet("MASTERSHEET");
      var headerRow = ["Employee ID", "Employee Name", "Hours Worked", "Shifts"]; // Adjust headers as needed
      masterSheet.getRange(1, 1, 1, headerRow.length).setValues([headerRow]);
    }

    var devToolsSheet = ss.getSheetByName("DevTools");
    if (!devToolsSheet) {
      devToolsSheet = ss.insertSheet("DevTools");
      devToolsSheet.getRange("A1").setValue("Last Updated Row");
      devToolsSheet.getRange("B1").setValue(1); // Start from 2nd Row
    }
    var lastUpdatedRow = devToolsSheet.getRange("B1").getValue();

    var entriesSheet = ss.getSheetByName("EntriesLog");
    console.log('EntriesLog sheet accessed');
    var uptoLastRow = entriesSheet.getLastRow();

    while (uptoLastRow - lastUpdatedRow > 0) {
      var entryId = entriesSheet.getRange(lastUpdatedRow + 1, 1).getValue(); // Assuming EmployeeID is in column A
      var entryName = entriesSheet.getRange(lastUpdatedRow + 1, 2).getValue(); // Employee name
      console.log('Employee ID: ' + entryId);

      var employeeSheet = ss.getSheetByName(entryId);
      if (!employeeSheet) {
        // Check if the sheet already exists
        var existingSheets = ss.getSheets();
        for (var i = 0; i < existingSheets.length; i++) {
          if (existingSheets[i].getName() === entryId) {
            employeeSheet = existingSheets[i];
            break;
          }
        }
        
        // If sheet doesn't exist, create a new one
        if (!employeeSheet) {
          employeeSheet = ss.insertSheet(entryId);
          var headerRow = ["Employee ID", "Name", "In Date", "In Time", "Out Date", "Out Time", "Hours Worked", "Shifts"];
          employeeSheet.getRange(1, 1, 1, 8).setValues([headerRow]);
        }

        // Add hyperlink to MasterSheet
        var existingHyperlinks = masterSheet.getRange("A:A").getValues().flat();
        var existingEmployeeIndex = existingHyperlinks.indexOf(entryId);
        if (existingEmployeeIndex !== -1) {
          // If hyperlink for employee exists, update it
          masterSheet.getRange(existingEmployeeIndex + 1, 1).setValue('=HYPERLINK("#gid=' + employeeSheet.getSheetId() + '", ' + entryId + ')');
          masterSheet.getRange(existingEmployeeIndex + 1, 2).setValue(entryName);
        } else {
          // If hyperlink for employee doesn't exist, append a new row
          masterSheet.appendRow(['=HYPERLINK("#gid=' + employeeSheet.getSheetId() + '","' + entryId + '")', entryName]);
        }
      }

      // Copy entry to employee's individual sheet
      var valuesToCopy = entriesSheet.getRange(lastUpdatedRow + 1, 1, 1, 7).getValues();
      var hoursWorked = valuesToCopy[0][6];
      var mins = hoursWorked.getHours() * 60 + hoursWorked.getMinutes();
      var shifts = mins > 465 ? 1 : 0; // shift duration in minutes, 07:45

      var employeeRowValues = valuesToCopy[0].concat(shifts);
      employeeSheet.appendRow(employeeRowValues);

      if (mins == 0) {
        var employeeLastRow = employeeSheet.getLastRow();
        var range = employeeSheet.getRange(employeeLastRow, 1, 1, 8);
        range.setBackground("red");
      } else if (mins < 465) { // attendance less than 7 hours 45 minutes
        var employeeLastRow = employeeSheet.getLastRow();
        var range = employeeSheet.getRange(employeeLastRow, 1, 1, 8);
        range.setBackground("yellow");
      }

      lastUpdatedRow++;
      devToolsSheet.getRange("B1").setValue(lastUpdatedRow); // Update the last updated row in DevTools
    }

  } catch (error) {
    console.log('Error: ' + error.message);
  }
}

function updateTotalHours() {
  var ss = SpreadsheetApp.getActiveSpreadsheet();
  var masterSheet = ss.getSheetByName("MASTERSHEET");
  var data = masterSheet.getDataRange().getValues();

  console.log("Starting updateTotalHours");
  
  for (var rowIndex = 1; rowIndex < data.length; rowIndex++) { // Start from 1 to skip header row
    var employeeId = data[rowIndex][0];
    console.log("Processing employee ID: " + employeeId);
    
    var employeeSheet = ss.getSheetByName(employeeId);
    if (employeeSheet) {
      try {
        var total_hours = 0;
        var total_minutes = 0;
        var total_shifts = 0; // Initialize total shifts count

        console.log("Computing total hours and shifts for sheet: " + employeeSheet.getName());
  
        // Loop through each row in the sheet from row 2 to the last row
        var lastRow = employeeSheet.getLastRow();
        for (var row = 2; row <= lastRow; row++) {
          var cellValue = employeeSheet.getRange(row, 7).getValue(); // Assuming Hours Worked are in column 7 (G)
          if (cellValue instanceof Date) {
            total_hours += cellValue.getHours();
            total_minutes += cellValue.getMinutes();

            var shifts = employeeSheet.getRange(row,8).getValue();
            total_shifts += shifts;
          } else {
            console.log("Cell in row " + row + " is not a valid date/time format.");
          }
        }

        // Handle overflow of minutes into hours
        total_hours += Math.floor(total_minutes / 60);
        total_minutes = total_minutes % 60;

        // Format total hours as "HH:mm"
        var totalHours = total_hours + ":" + (total_minutes < 10 ? '0' + total_minutes : total_minutes);

        console.log("Total hours for " + employeeId + ": " + totalHours);
        console.log("Total shifts for " + employeeId + ": " + total_shifts);

        // Update the total hours and shifts in the MASTERSHEET
        masterSheet.getRange(rowIndex + 1, 3).setValue(totalHours); // Update total hours
        masterSheet.getRange(rowIndex + 1, 4).setValue(total_shifts); // Update total shifts
      } catch (e) {
        console.log("Error processing sheet for employee ID " + employeeId + ": " + e.message);
      }
    } else {
      console.log("Sheet not found for employee ID: " + employeeId);
    }
  }
  
  console.log("Finished updateTotalHours");
}
