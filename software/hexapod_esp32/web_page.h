/**

  Calibration web page -- served verbatim from flash by web_ui.ino

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

  Styled after Hexapod Link (github.com/rookidroid/hexapod-link), so the robot's
  own page and the desktop app look and name things the same way: legs are
  "Right/Left Leg 1-3" front to back, joints "Joint 1-3" outward from the body.
  The app's fonts are named first but not bundled -- the robot's access point
  has no route to the internet -- so the device's own fonts stand in for them.

*/

#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

// HTML page for calibration interface
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Hexapod Calibration</title>
<style>
:root{
  --bg:#e5e7eb;--surface:#f9fafb;--plate:#fff;--border:#4b5563;--border-light:#9ca3af;
  --accent:#ea580c;--text:#111827;--muted:#4b5563;--ok:#16a34a;--fault:#dc2626;
  --heading:'Rajdhani',system-ui,-apple-system,'Segoe UI',Roboto,sans-serif;
  --mono:'Share Tech Mono',ui-monospace,Menlo,Consolas,'Courier New',monospace;
  --shadow:4px 4px 0 rgba(17,24,39,.15);--shadow-hover:6px 6px 0 rgba(17,24,39,.2);
}
*{box-sizing:border-box}
body{margin:0;font-family:var(--heading);font-weight:500;background:var(--bg);color:var(--text)}
button,input{font:inherit}

/* Header: the app's navbar, down to the hazard stripe */
header{position:relative;display:flex;align-items:center;gap:12px;flex-wrap:wrap;
  background:var(--surface);border-bottom:4px solid var(--accent);padding:12px 16px;
  box-shadow:0 2px 10px rgba(0,0,0,.1);margin-bottom:24px}
header::after{content:'';position:absolute;left:0;right:0;bottom:-8px;height:4px;
  background:repeating-linear-gradient(45deg,var(--accent) 0 10px,var(--text) 10px 20px)}
.brand{font-weight:700;font-size:1.6rem;letter-spacing:2px;text-transform:uppercase}
.brand::before{content:'\2699  ';color:var(--accent)}
.robot{font-family:var(--mono);color:var(--muted);letter-spacing:1px;text-transform:uppercase}

/* Status chip: same plate and LED bar as the app's link readout */
.chip{margin-left:auto;font-family:var(--mono);font-weight:700;letter-spacing:1px;text-transform:uppercase;
  background:var(--plate);border:2px solid var(--border);border-left-width:6px;border-left-color:var(--muted);
  padding:4px 12px;box-shadow:var(--shadow)}
.chip::after{content:'\25CF';margin-left:8px;color:var(--muted)}
.chip.is-on{border-left-color:var(--accent)}
.chip.is-on::after{color:var(--accent);animation:pulse 1s steps(1,end) infinite}
@keyframes pulse{50%{opacity:.25}}

main{max-width:960px;margin:0 auto;padding:0 16px 24px}

/* Cards: sharp plates with a hard shadow and a dark top bar */
.card{position:relative;background:var(--plate);border:2px solid var(--border);box-shadow:var(--shadow);
  padding:20px;margin-bottom:20px}
.card::before{content:'';position:absolute;top:0;left:0;right:0;height:4px;background:var(--border)}
h2{display:inline-block;margin:0 0 12px;font-size:1rem;font-weight:700;letter-spacing:1px;text-transform:uppercase;
  border-bottom:2px solid var(--border-light);padding-bottom:4px}
.lede{color:var(--muted);line-height:1.5;margin:0 0 16px}

/* Buttons */
.btn{border:2px solid var(--border);background:var(--plate);color:var(--text);font-weight:700;
  letter-spacing:1px;text-transform:uppercase;padding:10px 18px;cursor:pointer;box-shadow:var(--shadow);
  transition:transform .1s,box-shadow .1s,border-color .1s}
.btn:hover:not(:disabled){transform:translate(-2px,-2px);box-shadow:var(--shadow-hover);border-color:var(--accent)}
.btn:active:not(:disabled){transform:none;box-shadow:none}
.btn:disabled{opacity:.4;cursor:default;box-shadow:none}
.btn-accent{background:var(--accent);border-color:var(--accent);color:#fff}
.btn-accent:hover:not(:disabled){border-color:var(--text)}
.row{display:flex;gap:10px;flex-wrap:wrap}
.row .btn{flex:1 1 140px}

/* Offsets: one block per leg, the leg header styled like the app's */
/* Two columns fill top to bottom: left legs on the left, right legs on the right */
.legs{display:grid;grid-template-columns:1fr 1fr;grid-template-rows:repeat(3,auto);grid-auto-flow:column;
  gap:16px 24px;margin:20px 0}
@media (max-width:720px){.legs{grid-template-columns:1fr;grid-template-rows:none;grid-auto-flow:row}}
.leg-head{font-family:var(--mono);font-weight:700;text-transform:uppercase;background:var(--bg);
  border-left:4px solid var(--accent);padding:4px 8px;margin-bottom:8px}
.joints{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}
.label{font-family:var(--mono);font-size:.75rem;color:var(--muted);text-transform:uppercase;margin-bottom:4px}
.stepper{display:flex}
.stepper button{width:30px;flex:none;border:2px solid var(--border-light);background:var(--surface);
  font-family:var(--mono);font-weight:700;cursor:pointer;padding:0}
.stepper button:hover:not(:disabled){border-color:var(--accent);color:var(--accent)}
input[type=number]{width:100%;min-width:0;font-family:var(--mono);font-size:.95rem;text-align:center;
  background:#f3f4f6;border:2px solid var(--border-light);border-left:0;border-right:0;color:var(--text);
  padding:6px 2px;border-radius:0;box-shadow:inset 2px 2px 0 rgba(0,0,0,.05);-moz-appearance:textfield}
input[type=number]::-webkit-inner-spin-button,input[type=number]::-webkit-outer-spin-button{-webkit-appearance:none;margin:0}
input[type=number]:focus{outline:none;background:#fff;border-color:var(--accent)}
.legs.is-idle{opacity:.4;filter:grayscale(1);pointer-events:none}

/* Speed */
.speed{display:flex;align-items:center;gap:16px}
.speed input{flex:1;accent-color:var(--accent)}
.speed-value{font-family:var(--mono);font-size:1.3rem;min-width:4ch;text-align:right}
.hint{color:var(--muted);font-size:.9rem;margin-top:8px}

/* Messages and the safety callout */
.message{font-family:var(--mono);text-align:center;min-height:1.4em;margin-top:14px}
.message.is-error{color:var(--fault)}
.message.is-ok{color:var(--ok)}
.callout{border-left:6px solid var(--accent);background:var(--surface);padding:10px 14px;margin:0 0 16px;
  color:var(--muted);line-height:1.5}
.callout b{color:var(--accent);text-transform:uppercase;letter-spacing:1px}
footer{text-align:center;color:var(--muted);font-family:var(--mono);font-size:.8rem;padding:8px 0 24px}
</style>
</head>
<body>
<header>
  <div class="brand">Hexapod</div>
  <div class="robot" id="robot"></div>
  <div class="chip" id="chip">Idle</div>
</header>
<main>
  <section class="card">
    <h2>Servo calibration</h2>
    <p class="lede">Trims each servo so the legs match the calibration posture. Enter calibration
      mode and the robot holds that posture; every change applies at once. Offsets are servo
      ticks, about 0.44&deg; each, &plusmn;100.</p>
    <div class="row">
      <button class="btn btn-accent" id="enterBtn" onclick="enterCalibration()">Enter calibration</button>
      <button class="btn" id="exitBtn" onclick="exitCalibration()" disabled>Exit</button>
    </div>
    <div class="legs is-idle" id="legs"></div>
    <div class="callout"><b>Save to robot</b> writes the offsets to flash. Until then they are
      lost at the next reboot.</div>
    <div class="row">
      <button class="btn" id="reloadBtn" onclick="reloadOffsets()">Reload</button>
      <button class="btn btn-accent" id="saveBtn" onclick="saveOffsets()" disabled>Save to robot</button>
    </div>
    <div class="message" id="message"></div>
  </section>

  <section class="card">
    <h2>Gait speed</h2>
    <div class="speed">
      <input type="range" id="speedSlider" min="20" max="100" step="5" value="60"
             oninput="showSpeed(this.value)" onchange="setSpeed(this.value)">
      <span class="speed-value" id="speedValue">60%</span>
    </div>
    <div class="hint">Percent of the robot's tuned gait rate. Not saved; a connected remote overrides it.</div>
  </section>
</main>
<footer>&copy; 2024 - PRESENT rookidroid.com</footer>

<script>
const MAX_OFFSET = 100;
const JOINTS = ['Joint 1', 'Joint 2', 'Joint 3'];
// Laid out as seen from above, facing forward: the grid fills column by column,
// so the left legs land in the left column and the right legs in the right,
// each front to back.
const LEGS = [['left', 0], ['left', 1], ['left', 2], ['right', 0], ['right', 1], ['right', 2]];

let offsets = { left: [[0,0,0],[0,0,0],[0,0,0]], right: [[0,0,0],[0,0,0],[0,0,0]] };
let calibrating = false;

const $ = id => document.getElementById(id);

function say(text, kind) {
  $('message').textContent = text;
  $('message').className = 'message' + (kind ? ' is-' + kind : '');
}

// Resolves with the reply; rejects with the firmware's own error text.
function request(url, options) {
  return fetch(url, options).then(r => r.text().then(text => {
    if (!r.ok) throw new Error(text || ('HTTP ' + r.status));
    return text;
  }));
}

function setCalibrating(on) {
  calibrating = on;
  $('chip').textContent = on ? 'Calibrating' : 'Idle';
  $('chip').className = 'chip' + (on ? ' is-on' : '');
  $('legs').className = 'legs' + (on ? '' : ' is-idle');
  $('enterBtn').disabled = on;
  $('exitBtn').disabled = !on;
  $('saveBtn').disabled = !on;
}

function renderOffsets() {
  const legs = $('legs');
  legs.innerHTML = '';
  LEGS.forEach(([side, leg]) => {
    const block = document.createElement('div');
    const name = (side === 'right' ? 'Right' : 'Left') + ' Leg ' + (leg + 1);
    let html = `<div class="leg-head">${name}</div><div class="joints">`;
    JOINTS.forEach((joint, j) => {
      html += `<div><div class="label">${joint}</div><div class="stepper">
        <button onclick="nudge('${side}',${leg},${j},-1)" aria-label="${name} ${joint} down">&minus;</button>
        <input type="number" id="${side}_${leg}_${j}" value="${offsets[side][leg][j]}"
               aria-label="${name} ${joint}" onchange="edit('${side}',${leg},${j},this.value)">
        <button onclick="nudge('${side}',${leg},${j},1)" aria-label="${name} ${joint} up">+</button>
      </div></div>`;
    });
    block.innerHTML = html + '</div>';
    legs.appendChild(block);
  });
}

function setOffset(side, leg, joint, value) {
  value = Math.max(-MAX_OFFSET, Math.min(MAX_OFFSET, Math.round(Number(value) || 0)));
  offsets[side][leg][joint] = value;
  $(`${side}_${leg}_${joint}`).value = value;
}

function edit(side, leg, joint, value) { setOffset(side, leg, joint, value); applyOffsets(); }
function nudge(side, leg, joint, delta) { edit(side, leg, joint, offsets[side][leg][joint] + delta); }

function postOffsets() {
  return request('/set_offsets', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(offsets)
  });
}

function applyOffsets() {
  postOffsets().then(text => say(text, 'ok')).catch(e => say(e.message, 'error'));
}

function enterCalibration() {
  say('Entering calibration mode...');
  request('/enter_calibration').then(text => {
    offsets = JSON.parse(text);
    renderOffsets();
    setCalibrating(true);
    say('Calibration mode: the robot is holding its calibration posture.', 'ok');
  }).catch(e => say(e.message, 'error'));
}

function exitCalibration() {
  request('/exit_calibration').then(() => {
    setCalibrating(false);
    say('Left calibration mode.', 'ok');
  }).catch(e => say(e.message, 'error'));
}

function reloadOffsets() {
  request('/get_offsets').then(text => {
    offsets = JSON.parse(text);
    renderOffsets();
    say("Loaded the robot's current offsets.", 'ok');
  }).catch(e => say(e.message, 'error'));
}

function saveOffsets() {
  say('Saving offsets...');
  postOffsets()
    .then(() => request('/save_offsets', { method: 'POST' }))
    .then(text => say(text, 'ok'))
    .catch(e => say(e.message, 'error'));
}

function showSpeed(value) {
  $('speedSlider').value = value;
  $('speedValue').textContent = value + '%';
}

function setSpeed(value) {
  request('/set_speed?pct=' + value, { method: 'POST' })
    .then(text => showSpeed(JSON.parse(text).speed))
    .catch(e => say(e.message, 'error'));
}

renderOffsets();
request('/robot_config').then(text => {
  const config = JSON.parse(text);
  const speed = config.speed || {};
  $('robot').textContent = ((config.geometry && config.geometry.label) || config.name) +
    (config.ssid ? ' · ' + config.ssid : '');
  document.title = $('robot').textContent + ' — Calibration';
  if (speed.min) $('speedSlider').min = speed.min;
  if (speed.max) $('speedSlider').max = speed.max;
  if (speed.current) showSpeed(speed.current);
}).catch(e => say(e.message, 'error'));
</script>
</body>
</html>
)rawliteral";

#endif  // WEB_PAGE_H
