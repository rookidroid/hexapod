/**

  Robot web page -- drive and calibration, served verbatim from flash by
  web_ui.ino

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

// HTML page for the drive and calibration interface
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Hexapod</title>
<style>
:root{
  --bg:#e5e7eb;--surface:#f9fafb;--plate:#fff;--border:#4b5563;--border-light:#9ca3af;
  --accent:#ea580c;--text:#111827;--muted:#4b5563;--ok:#16a34a;--fault:#dc2626;
  --heading:'Rajdhani',system-ui,-apple-system,'Segoe UI',Roboto,sans-serif;
  --mono:'Share Tech Mono',ui-monospace,Menlo,Consolas,'Courier New',monospace;
  --shadow:4px 4px 0 rgba(17,24,39,.15);--shadow-hover:6px 6px 0 rgba(17,24,39,.2);
}
*{box-sizing:border-box}
[hidden]{display:none!important}
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

/* Tabs: a segmented switch between driving and calibrating */
.tabs{display:flex;margin-bottom:20px;box-shadow:var(--shadow)}
.tab{flex:1;border:2px solid var(--border);background:var(--plate);color:var(--text);font-weight:700;
  letter-spacing:1px;text-transform:uppercase;padding:10px 18px;cursor:pointer}
.tab+.tab{border-left:0}
.tab:hover{color:var(--accent)}
.tab[aria-selected=true]{background:var(--border);color:#fff}

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
.btn-danger{border-color:var(--fault);color:var(--fault)}
.btn-danger:hover:not(:disabled){border-color:var(--fault)}
.row{display:flex;gap:10px;flex-wrap:wrap}
.row .btn{flex:1 1 140px}

/* Drive: hold-to-move buttons. Long presses must not select text or open a
   menu on phones, and the page must not scroll under a held finger. */
.hold{touch-action:none;user-select:none;-webkit-user-select:none;-webkit-touch-callout:none;
  display:flex;flex-direction:column;align-items:center;justify-content:center;gap:2px;padding:8px}
.hold small{font-size:.7rem;letter-spacing:1px}
.hold.is-held,.hold.is-held:hover{background:var(--accent);border-color:var(--accent);color:#fff;
  transform:none;box-shadow:none}
.steer{display:grid;grid-template-columns:1fr auto 1fr;grid-template-areas:"tl pad tr";gap:16px;
  align-items:center;justify-items:center;margin:8px 0 16px}
.pad{grid-area:pad;display:grid;grid-template-columns:repeat(3,72px);grid-template-rows:repeat(3,72px);gap:8px}
.pad .btn{font-size:1.6rem;padding:0}
.turn{width:88px;height:72px;font-size:1.6rem}
.turn-l{grid-area:tl;justify-self:end}
.turn-r{grid-area:tr;justify-self:start}
@media (max-width:520px){
  .steer{grid-template-columns:1fr 1fr;grid-template-areas:"pad pad" "tl tr"}
  .turn-l,.turn-r{justify-self:stretch;width:auto}
}
.grid4{display:grid;grid-template-columns:repeat(4,1fr);gap:10px}
@media (max-width:520px){.grid4{grid-template-columns:repeat(2,1fr)}}
.grid4 .btn{min-height:64px}
.drive-off .hold{opacity:.4;pointer-events:none;box-shadow:none}
@media (hover:none){.keys{display:none}}

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
  <div class="tabs" role="tablist">
    <button class="tab" role="tab" id="tabDrive" aria-selected="true" onclick="showTab('drive')">Drive</button>
    <button class="tab" role="tab" id="tabCalibrate" aria-selected="false" onclick="showTab('calibrate')">Calibrate</button>
  </div>

  <div id="drive" class="drive-off">
    <section class="card">
      <h2>Walk</h2>
      <p class="lede">Hold a button to move; let go and the robot settles back to standby.
        <span class="keys">Keys: W A S D or the arrows walk, Q and E turn, Space stops.</span></p>
      <div class="steer">
        <button class="btn hold turn turn-l" data-cmd="turnleft" aria-label="Turn left">&#8634;<small>Turn</small></button>
        <div class="pad">
          <button class="btn hold" data-cmd="walkl45" aria-label="Walk forward left">&nwarr;</button>
          <button class="btn hold" data-cmd="walk0" aria-label="Walk forward">&uarr;</button>
          <button class="btn hold" data-cmd="walkr45" aria-label="Walk forward right">&nearr;</button>
          <button class="btn hold" data-cmd="walkl90" aria-label="Walk left">&larr;</button>
          <button class="btn hold" data-cmd="standby" aria-label="Stop">&#9632;</button>
          <button class="btn hold" data-cmd="walkr90" aria-label="Walk right">&rarr;</button>
          <button class="btn hold" data-cmd="walkl135" aria-label="Walk back left">&swarr;</button>
          <button class="btn hold" data-cmd="walk180" aria-label="Walk back">&darr;</button>
          <button class="btn hold" data-cmd="walkr135" aria-label="Walk back right">&searr;</button>
        </div>
        <button class="btn hold turn turn-r" data-cmd="turnright" aria-label="Turn right">&#8635;<small>Turn</small></button>
      </div>
      <div class="row">
        <button class="btn btn-danger" id="relaxBtn" onclick="relax()">Relax servos</button>
      </div>
      <div class="hint">Relax cuts power to the servos and the robot sags to the ground. Any move wakes them.</div>
      <div class="message" id="driveMessage"></div>
    </section>

    <section class="card">
      <h2>Gaits</h2>
      <div class="grid4">
        <button class="btn hold" data-cmd="fastforward">&uarr;<small>Fast</small></button>
        <button class="btn hold" data-cmd="fastbackward">&darr;<small>Fast</small></button>
        <button class="btn hold" data-cmd="climbforward">&uarr;<small>Climb</small></button>
        <button class="btn hold" data-cmd="climbbackward">&darr;<small>Climb</small></button>
      </div>
    </section>

    <section class="card">
      <h2>Body</h2>
      <div class="grid4">
        <button class="btn hold" data-cmd="rotatex">X<small>Rotate</small></button>
        <button class="btn hold" data-cmd="rotatey">Y<small>Rotate</small></button>
        <button class="btn hold" data-cmd="rotatez">Z<small>Rotate</small></button>
        <button class="btn hold" data-cmd="twist">&#8645;<small>Twist</small></button>
      </div>
    </section>

    <section class="card">
      <h2>Gait speed</h2>
      <div class="speed">
        <input type="range" id="speedSlider" min="20" max="100" step="5" value="60"
               oninput="showSpeed(this.value)" onchange="setSpeed(this.value)">
        <span class="speed-value" id="speedValue">60%</span>
      </div>
      <div class="hint">Percent of the robot's tuned gait rate. Sent with every move from this page;
        not saved. A connected remote overrides it.</div>
    </section>
  </div>

  <div id="calibrate" hidden>
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
  </div>
</main>
<footer>&copy; 2024 - PRESENT rookidroid.com</footer>

<script>
const MAX_OFFSET = 100;
const JOINTS = ['Joint 1', 'Joint 2', 'Joint 3'];
// Laid out as seen from above, facing forward: the grid fills column by column,
// so the left legs land in the left column and the right legs in the right,
// each front to back.
const LEGS = [['left', 0], ['left', 1], ['left', 2], ['right', 0], ['right', 1], ['right', 2]];

// A held move is resent this often. The robot stops by itself when no command
// arrives for 500 ms, so this must stay well below that.
const DRIVE_MS = 150;

let offsets = { left: [[0,0,0],[0,0,0],[0,0,0]], right: [[0,0,0],[0,0,0],[0,0,0]] };
let calibrating = false;

let commands = [];  // Motion names from /robot_config; a name's index is its ID
let driving = null; // Name of the motion being held, or null
let driveTimer = 0;
let inFlight = false; // One request at a time: the robot serves them in turn
let resend = false;
let relaxed = false;
let activePointer = null;
const heldKeys = new Set();

const $ = id => document.getElementById(id);

function say(text, kind, id) {
  const el = $(id || 'message');
  el.textContent = text;
  el.className = 'message' + (kind ? ' is-' + kind : '');
}

// Resolves with the reply; rejects with the firmware's own error text.
function request(url, options) {
  return fetch(url, options).then(r => r.text().then(text => {
    if (!r.ok) throw new Error(text || ('HTTP ' + r.status));
    return text;
  }));
}

function updateChip() {
  let text = 'Idle';
  if (calibrating) text = 'Calibrating';
  else if (driving && driving !== 'standby') text = 'Driving';
  else if (relaxed) text = 'Relaxed';
  $('chip').textContent = text;
  $('chip').className = 'chip' + (text === 'Calibrating' || text === 'Driving' ? ' is-on' : '');
}

// ---------------------------------------------------------------------------
// Tabs
// ---------------------------------------------------------------------------

function showTab(name) {
  // Calibration mode holds the robot still, so leave it before driving.
  if (name === 'drive' && calibrating) {
    exitCalibration().then(() => { if (!calibrating) showTab('drive'); });
    return;
  }
  if (name !== 'drive') {
    heldKeys.clear();
    stop();
  }
  $('drive').hidden = name !== 'drive';
  $('calibrate').hidden = name !== 'calibrate';
  $('tabDrive').setAttribute('aria-selected', name === 'drive');
  $('tabCalibrate').setAttribute('aria-selected', name === 'calibrate');
}

// ---------------------------------------------------------------------------
// Drive
// ---------------------------------------------------------------------------

// Send the held motion, or standby once nothing is held.
function sendMotion() {
  if (inFlight) {
    resend = true;
    return;
  }
  const id = commands.indexOf(driving || 'standby');
  if (id < 0) return;
  inFlight = true;
  request('/motion?id=' + id + '&pct=' + $('speedSlider').value, { method: 'POST' })
    .then(() => {
      if ($('driveMessage').classList.contains('is-error')) say('', '', 'driveMessage');
    })
    .catch(e => say(e.message, 'error', 'driveMessage'))
    .finally(() => {
      inFlight = false;
      if (resend) {
        resend = false;
        sendMotion();
      }
    });
}

function markHeld() {
  document.querySelectorAll('.hold').forEach(b =>
    b.classList.toggle('is-held', b.dataset.cmd === driving));
}

// Start, or switch to, holding motion `name`.
function drive(name) {
  if (calibrating || commands.indexOf(name) < 0) return;
  const changed = name !== driving;
  driving = name;
  relaxed = false;
  if (!driveTimer) driveTimer = setInterval(sendMotion, DRIVE_MS);
  if (changed) sendMotion();
  markHeld();
  updateChip();
}

// Stop resending without telling the robot anything.
function halt() {
  const was = driveTimer !== 0;
  clearInterval(driveTimer);
  driveTimer = 0;
  driving = null;
  activePointer = null;
  markHeld();
  updateChip();
  return was;
}

// Let go: stop resending and send standby once, so the robot stops at once
// instead of waiting out its failsafe.
function stop() {
  if (halt()) sendMotion();
}

function relax() {
  heldKeys.clear();
  halt();
  request('/relax', { method: 'POST' }).then(text => {
    relaxed = true;
    updateChip();
    say(text, 'ok', 'driveMessage');
  }).catch(e => say(e.message, 'error', 'driveMessage'));
}

document.querySelectorAll('.hold').forEach(b => {
  b.addEventListener('pointerdown', e => {
    if (e.button !== 0) return;
    e.preventDefault();
    activePointer = e.pointerId;
    // Keep receiving this pointer's events even if it slides off the button.
    if (b.setPointerCapture) b.setPointerCapture(e.pointerId);
    drive(b.dataset.cmd);
  });
  const release = e => {
    if (e.pointerId === activePointer) stop();
  };
  b.addEventListener('pointerup', release);
  b.addEventListener('pointercancel', release);
  b.addEventListener('lostpointercapture', release);
  b.addEventListener('contextmenu', e => e.preventDefault());
});

// Keyboard: W A S D or the arrows walk (two at once for the diagonals), Q and E
// turn, Space stops.
const KEYS = {
  KeyW: 'f', ArrowUp: 'f', KeyS: 'b', ArrowDown: 'b',
  KeyA: 'l', ArrowLeft: 'l', KeyD: 'r', ArrowRight: 'r',
  KeyQ: 'tl', KeyE: 'tr'
};
// [forward/back][left/right], each -1, 0 or 1
const WALKS = {
  '1,-1': 'walkl45', '1,0': 'walk0', '1,1': 'walkr45',
  '0,-1': 'walkl90', '0,1': 'walkr90',
  '-1,-1': 'walkl135', '-1,0': 'walk180', '-1,1': 'walkr135'
};

function keyMotion() {
  const k = key => heldKeys.has(key);
  if (k('tl') !== k('tr')) return k('tl') ? 'turnleft' : 'turnright';
  const fb = (k('f') ? 1 : 0) - (k('b') ? 1 : 0);
  const lr = (k('r') ? 1 : 0) - (k('l') ? 1 : 0);
  return WALKS[fb + ',' + lr] || null;
}

function applyKeys() {
  const name = keyMotion();
  if (name) drive(name);
  else stop();
}

function drivingByKeys(e) {
  return !$('drive').hidden && !e.ctrlKey && !e.metaKey && !e.altKey &&
    !(e.target && e.target.tagName === 'INPUT');
}

document.addEventListener('keydown', e => {
  if (!drivingByKeys(e)) return;
  if (e.code === 'Space') {
    e.preventDefault();
    heldKeys.clear();
    stop();
    return;
  }
  const key = KEYS[e.code];
  if (!key) return;
  e.preventDefault();
  if (e.repeat || heldKeys.has(key)) return;
  heldKeys.add(key);
  applyKeys();
});

document.addEventListener('keyup', e => {
  const key = KEYS[e.code];
  if (!key || !heldKeys.has(key)) return;
  heldKeys.delete(key);
  applyKeys();
});

// Never keep a move going while the page is out of sight.
window.addEventListener('blur', () => { heldKeys.clear(); stop(); });
document.addEventListener('visibilitychange', () => {
  if (document.hidden) {
    heldKeys.clear();
    stop();
  }
});

function showSpeed(value) {
  $('speedSlider').value = value;
  $('speedValue').textContent = value + '%';
}

function setSpeed(value) {
  request('/set_speed?pct=' + value, { method: 'POST' })
    .then(text => showSpeed(JSON.parse(text).speed))
    .catch(e => say(e.message, 'error', 'driveMessage'));
}

// ---------------------------------------------------------------------------
// Calibration
// ---------------------------------------------------------------------------

function setCalibrating(on) {
  calibrating = on;
  $('legs').className = 'legs' + (on ? '' : ' is-idle');
  $('enterBtn').disabled = on;
  $('exitBtn').disabled = !on;
  $('saveBtn').disabled = !on;
  updateChip();
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
    relaxed = false;
    setCalibrating(true);
    say('Calibration mode: the robot is holding its calibration posture.', 'ok');
  }).catch(e => say(e.message, 'error'));
}

function exitCalibration() {
  return request('/exit_calibration').then(() => {
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

// ---------------------------------------------------------------------------
// Startup
// ---------------------------------------------------------------------------

renderOffsets();
request('/robot_config').then(text => {
  const config = JSON.parse(text);
  const speed = config.speed || {};
  $('robot').textContent = ((config.geometry && config.geometry.label) || config.name) +
    (config.ssid ? ' · ' + config.ssid : '');
  document.title = $('robot').textContent + ' — Hexapod';
  if (speed.min) $('speedSlider').min = speed.min;
  if (speed.max) $('speedSlider').max = speed.max;
  if (speed.current) showSpeed(speed.current);
  // Hide moves this firmware does not have, so IDs always come from the robot.
  commands = config.commands || [];
  // Hidden rather than removed, so the direction pad keeps its shape.
  document.querySelectorAll('.hold').forEach(b => {
    b.style.visibility = commands.indexOf(b.dataset.cmd) < 0 ? 'hidden' : '';
  });
  $('drive').classList.remove('drive-off');
}).catch(e => say(e.message, 'error', 'driveMessage'));
</script>
</body>
</html>
)rawliteral";

#endif  // WEB_PAGE_H
