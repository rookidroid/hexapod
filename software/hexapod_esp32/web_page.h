/**

  Calibration web page -- served verbatim from flash by web_ui.ino

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

*/

#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

// HTML page for calibration interface
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Hexapod Calibration</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      padding: 20px;
      background-color: #1a1a2e;
      color: #eee;
    }
    h1 { color: #bb86fc; text-align: center; }
    h2 { color: #bb86fc; margin-top: 30px; }
    .container { max-width: 1200px; margin: 0 auto; }
    .btn {
      background-color: #8b5cf6;
      color: #fff;
      border: none;
      padding: 15px 30px;
      font-size: 18px;
      border-radius: 8px;
      cursor: pointer;
      margin: 10px 5px;
      transition: all 0.3s;
    }
    .btn:hover { background-color: #7c3aed; transform: scale(1.05); }
    .btn:active { transform: scale(0.95); }
    .btn-small { padding: 8px 32px; font-size: 14px; }
    .controls { text-align: center; margin: 20px 0; }
    .calibration-panel { display: none; margin-top: 20px; }
    .leg-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 200px; margin-top: 20px; }
    @media (max-width: 768px) { .leg-grid { grid-template-columns: 1fr; } }
    .leg-column { display: flex; flex-direction: column; gap: 10px; }
    .leg-card {
      background-color: #16213e;
      padding: 20px;
      border-radius: 10px;
      border: 2px solid #8b5cf6;
    }
    .leg-title { color: #bb86fc; font-size: 20px; font-weight: bold; margin-bottom: 15px; }
    .joint-row { display: flex; align-items: center; margin: 0 0; }
    .joint-label { font-weight: bold; min-width: 80px; flex-shrink: 0;  }
    input[type="number"] {
      width: 150px;
      padding: 8px;
      background-color: #0f3460;
      color: #eee;
      border: 1px solid #a78bfa;
      border-radius: 5px;
      font-size: 16px;
      margin-right: auto;
    }
    .btn-small { margin-left: 5px; }
    .status { text-align: center; margin-top: 20px; font-size: 18px; color: #bb86fc; }
    .reminder {
      background-color: #e65100;
      color: #fff;
      padding: 15px;
      border-radius: 8px;
      margin: 20px 0;
      text-align: center;
      font-weight: bold;
      border: 2px solid #ff6f00;
    }
    .footer {
      text-align: center;
      margin-top: 40px;
      padding: 20px;
      color: #888;
      font-size: 14px;
      border-top: 1px solid #333;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Hexapod Calibration</h1>
    <div class="controls">
      <button class="btn" id="calibModeBtn" onclick="toggleCalibrationMode()">Enter Calibration Mode</button>
    </div>
    <div class="status" id="status"></div>
    <div class="calibration-panel" id="calibPanel">
      <h2>Servo Offset Adjustments (Ticks, 1 tick ≈ 0.44°)</h2>
      <div class="leg-grid" id="legGrid"></div>
      <div class="reminder">
        ⚠️ Remember to click "Save Offsets" button below to permanently save your calibration to EEPROM!
      </div>
      <div class="controls">
        <button class="btn" onclick="saveOffsets()">Save Offsets</button>
      </div>
    </div>
    <div class="footer">
      © 2024 - PRESENT rookidroid.com
    </div>
  </div>
  <script>
    let offsets = { left: [[0,0,0],[0,0,0],[0,0,0]], right: [[0,0,0],[0,0,0],[0,0,0]] };
    let isCalibrationMode = false;
    
    function toggleCalibrationMode() {
      if (isCalibrationMode) {
        exitCalibrationMode();
      } else {
        enterCalibrationMode();
      }
    }
    
    function enterCalibrationMode() {
      document.getElementById('status').innerHTML = 'Loading calibration data...';
      fetch('/enter_calibration')
        .then(response => response.json())
        .then(data => {
          offsets = data;
          displayOffsets();
          document.getElementById('calibPanel').style.display = 'block';
          document.getElementById('calibModeBtn').innerHTML = 'Exit Calibration Mode';
          document.getElementById('status').innerHTML = 'Calibration mode active - Adjust offsets and click Save Offsets when done';
          isCalibrationMode = true;
          applyOffsets();
        })
        .catch(error => {
          document.getElementById('status').innerHTML = 'Error: ' + error;
        });
    }
    
    function exitCalibrationMode() {
      document.getElementById('status').innerHTML = 'Exiting calibration mode...';
      fetch('/exit_calibration')
        .then(response => response.text())
        .then(data => {
          document.getElementById('calibPanel').style.display = 'none';
          document.getElementById('calibModeBtn').innerHTML = 'Enter Calibration Mode';
          document.getElementById('status').innerHTML = 'Calibration mode exited';
          isCalibrationMode = false;
        })
        .catch(error => {
          document.getElementById('status').innerHTML = 'Error: ' + error;
        });
    }
    
    function displayOffsets() {
      const grid = document.getElementById('legGrid');
      grid.innerHTML = '';
      const sides = [['Left', offsets.left], ['Right', offsets.right]];
      const joints = ['Joint 1', 'Joint 2', 'Joint 3'];
      
      sides.forEach(([sideName, sideData]) => {
        const column = document.createElement('div');
        column.className = 'leg-column';
        
        for (let leg = 0; leg < 3; leg++) {
          const card = document.createElement('div');
          card.className = 'leg-card';
          card.innerHTML = `<div class="leg-title">${sideName} Leg ${leg + 1}</div>`;
          
          for (let joint = 0; joint < 3; joint++) {
            const row = document.createElement('div');
            row.className = 'joint-row';
            row.innerHTML = `
              <span class="joint-label">${joints[joint]}:</span>
              <input type="number" id="${sideName}_${leg}_${joint}" value="${sideData[leg][joint]}" 
                     onchange="updateOffset('${sideName}', ${leg}, ${joint}, this.value)">
              <button class="btn btn-small" onclick="adjustOffset('${sideName}', ${leg}, ${joint}, -1)">-</button>
              <button class="btn btn-small" onclick="adjustOffset('${sideName}', ${leg}, ${joint}, 1)">+</button>
            `;
            card.appendChild(row);
          }
          column.appendChild(card);
        }
        grid.appendChild(column);
      });
    }
    
    function updateOffset(side, leg, joint, value) {
      offsets[side.toLowerCase()][leg][joint] = parseInt(value);
    }
    
    function adjustOffset(side, leg, joint, delta) {
      const inputId = `${side}_${leg}_${joint}`;
      const input = document.getElementById(inputId);
      const newValue = parseInt(input.value) + delta;
      input.value = newValue;
      updateOffset(side, leg, joint, newValue);
      applyOffsets();
    }
    
    function applyOffsets() {
      fetch('/set_offsets', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(offsets)
      })
      .then(response => response.text())
      .catch(error => {
        console.error('Apply offsets error:', error);
      });
    }
    
    function saveOffsets() {
      document.getElementById('status').innerHTML = 'Saving offsets...';
      fetch('/set_offsets', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(offsets)
      })
      .then(response => response.text())
      .then(data => {
        return fetch('/save_offsets', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify(offsets)
        });
      })
      .then(response => response.text())
      .then(data => {
        document.getElementById('status').innerHTML = data;
      })
      .catch(error => {
        document.getElementById('status').innerHTML = 'Error: ' + error;
      });
    }
  </script>
</body>
</html>
)rawliteral";

#endif  // WEB_PAGE_H
