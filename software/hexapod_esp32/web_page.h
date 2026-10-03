/**

  Robot web page -- drive and calibration, served verbatim from flash by
  web_ui.ino

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

  Styled after Hexapod Link (github.com/rookidroid/hexapod-link), so the robot's
  own page and the desktop app look and name things the same way: legs are
  "Right/Left Leg 1-3" front to back, joints "Joint 1-3" outward from the body.
  It carries the app's RX-78-2 theme in both its light and dark (cockpit)
  variants, following the device's setting until the header toggle picks one.
  The app's fonts are named first but not bundled -- the robot's access point
  has no route to the internet -- so the device's own fonts stand in for them.

  The Drive tab fills the window like a hardware control panel, laid out as in
  the Android app: body moves under the left thumb, the steering dial under the
  right.

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
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-title" content="Hexapod">
<meta name="theme-color" content="#f3f5f9" id="themeColor">
<title>Hexapod</title>
<script>
// Set the theme before first paint: the one picked with the toggle, else the device's.
(function () {
  let theme = null;
  try { theme = localStorage.getItem('theme'); } catch (e) {}
  if (theme !== 'light' && theme !== 'dark')
    theme = matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
  document.documentElement.setAttribute('data-theme', theme);
})();
</script>
<style>
/* RX-78-2 colours, as in Hexapod Link: white armour plates framed in navy,
   Federation blue as the accent, red and V-fin yellow as trim. Dark is the
   cockpit: the same tricolour on deep navy with a cyan HUD glow. The theme is
   data-theme on <html>, set before first paint by the script in <head>. */
:root{color-scheme:light;
  --bg:#dde3ec;--surface:#f3f5f9;--plate:#fff;--border:#1b2a4a;--border-light:#a9b6cc;
  --accent:#1d4fa3;--accent-hover:#163f84;--alt:#f2b705;--red:#d62828;--red-hover:#b51f1f;--ok:#16a34a;
  --text:#0f1a30;--muted:#4a5875;--hover:#e8eef8;--input-bg:#eef2f8;--input-focus-bg:#fff;--outline:#0f1a30;
  --alert-text:#9b1c1c;--alert-bg:#fdecec;--well:#1b2a4a;--well-hover:#2c3f66;
  --glow:0 0 0 2px rgba(29,79,163,.25);
  --shade:rgba(15,26,48,.18);--shadow:4px 4px 0 var(--shade);--shadow-soft:0 2px 10px rgba(15,26,48,.12);
  --heading:'Chakra Petch',system-ui,-apple-system,'Segoe UI',Roboto,sans-serif;
  --display:'Michroma',var(--heading);
  --mono:'IBM Plex Mono',ui-monospace,Menlo,Consolas,'Courier New',monospace;
  /* Mecha corners: top-right and bottom-left cut at 45 degrees. A clipped
     element loses its box-shadow, so chamfered parts have none. */
  --chamfer:8px;
  --chamfer-clip:polygon(0 0,calc(100% - var(--chamfer)) 0,100% var(--chamfer),100% 100%,var(--chamfer) 100%,0 calc(100% - var(--chamfer)));
  --gutter-l:max(16px,env(safe-area-inset-left));--gutter-r:max(16px,env(safe-area-inset-right));
}
:root[data-theme=dark]{color-scheme:dark;
  --bg:#060c18;--surface:#0c1730;--plate:#11203f;--border:#2f5fb3;--border-light:#1e3563;
  --accent:#3d7cf0;--accent-hover:#2f68d6;--alt:#ffd23f;--red:#ff4d4d;--red-hover:#e03c3c;--ok:#22c55e;
  --text:#e4eeff;--muted:#8aa2c8;--hover:#18294d;--input-bg:#08122a;--input-focus-bg:#050c1e;--outline:#02060f;
  --alert-text:#ffb4b4;--alert-bg:rgba(255,77,77,.12);--well:#1a3366;--well-hover:#22407d;
  --glow:0 0 0 1px rgba(76,201,240,.45),0 0 14px rgba(76,201,240,.25);
  --shade:rgba(0,0,0,.5);--shadow-soft:0 2px 10px rgba(0,0,0,.55);
}
*{box-sizing:border-box}
[hidden]{display:none!important}
body{margin:0;font-family:var(--heading);font-weight:500;background:var(--bg);color:var(--text)}
button,input{font:inherit}
/* Michroma has one weight; where it is missing the stand-in is set bold instead */
.brand,h2,.callout b{font-family:var(--display);font-weight:700;font-synthesis:none}
/* Keyboard focus on chamfered parts: an inset ring, which the clip leaves alone */
.btn:focus-visible,.tab:focus-visible,.theme-btn:focus-visible,.stepper button:focus-visible{
  outline:2px solid currentColor;outline-offset:-6px}

/* Header: the app's navbar, down to the chest trim */
header{position:relative;display:flex;align-items:center;gap:8px 12px;flex-wrap:wrap;
  background:var(--surface);border-bottom:4px solid var(--accent);
  padding:max(10px,env(safe-area-inset-top)) var(--gutter-r) 10px var(--gutter-l);
  box-shadow:var(--shadow-soft);margin-bottom:24px}
/* A red block, then yellow vent slats on blue, as on the RX-78's torso */
header::after{content:'';position:absolute;left:0;right:0;bottom:-8px;height:4px;
  background:linear-gradient(90deg,var(--red) 0 160px,transparent 160px),
    repeating-linear-gradient(90deg,var(--alt) 0 14px,transparent 14px 22px),var(--accent)}
/* The slats crawl while the robot is moving */
body.is-driving header::after{animation:crawl .5s linear infinite}
@keyframes crawl{to{background-position:0 0,22px 0}}
.brand{font-size:1.35rem;letter-spacing:2px;text-transform:uppercase}
.brand::before{content:'\2699  ';color:var(--accent)}
.robot{font-family:var(--mono);font-weight:600;color:var(--muted);letter-spacing:1px;text-transform:uppercase}

/* Tabs: a segmented switch between driving and calibrating */
.tabs{display:flex;clip-path:var(--chamfer-clip)}
.tab{border:2px solid var(--border);background:var(--plate);color:var(--muted);font-weight:700;
  letter-spacing:1px;text-transform:uppercase;padding:4px 14px;cursor:pointer;transition:background-color .1s,color .1s}
.tab+.tab{border-left:0}
.tab:hover{color:var(--text);background:var(--hover)}
.tab[aria-selected=true]{background:var(--accent);color:#fff}

/* Theme toggle: shows the icon of the theme it switches to */
.theme-btn{margin-left:auto;align-self:stretch;min-width:2.4rem;font-size:1.1rem;line-height:1;
  color:var(--text);background:var(--plate);border:2px solid var(--border);clip-path:var(--chamfer-clip);
  cursor:pointer;transition:transform .1s,color .1s}
.theme-btn:hover{color:var(--accent);transform:translate(-1px,-1px)}
.theme-btn:active{transform:translate(1px,1px)}

/* Status chip: the app's link readout, an LED and a left bar coloured by state */
.chip{font-family:var(--mono);font-weight:700;letter-spacing:1px;text-transform:uppercase;white-space:nowrap;
  background:var(--plate);border:2px solid var(--border);border-left-width:6px;border-left-color:var(--muted);
  padding:4px 14px 4px 12px;clip-path:var(--chamfer-clip)}
.chip::after{content:'\25CF';margin-left:8px;color:var(--muted)}
.chip.is-online{border-left-color:var(--ok)}
.chip.is-online::after{color:var(--ok)}
.chip.is-streaming{border-left-color:var(--alt)}
.chip.is-streaming::after{color:var(--alt);animation:pulse 1s steps(1,end) infinite}
.chip.is-fault{border-left-color:var(--red)}
.chip.is-fault::after{color:var(--red)}
@keyframes pulse{50%{opacity:.25}}

.page{max-width:960px;margin:0 auto;padding:0 var(--gutter-r) 24px var(--gutter-l)}

/* Cards: armour plates. A red/blue trim along the top ending in yellow vent
   slats, a navy plate in the bottom-right corner and a small unit marking. */
.card{position:relative;background:var(--plate);border:2px solid var(--border);box-shadow:var(--shadow);
  padding:20px 20px 24px;margin-bottom:20px}
.card::before{content:'';position:absolute;top:0;left:0;right:0;height:4px;pointer-events:none;
  background:repeating-linear-gradient(90deg,var(--alt) 0 5px,transparent 5px 9px) right 6px top/32px 100% no-repeat,
    linear-gradient(90deg,var(--red) 0 28px,var(--accent) 28px calc(100% - 44px),transparent calc(100% - 44px))}
.card::after{content:'RX-78-2';position:absolute;right:0;bottom:0;padding:0 20px 3px 0;
  font-family:var(--mono);font-size:.55rem;font-weight:600;line-height:1;letter-spacing:1.5px;
  color:color-mix(in srgb,var(--muted) 65%,transparent);pointer-events:none;
  background:linear-gradient(135deg,transparent 50%,var(--border) 50%) right bottom/14px 14px no-repeat}
h2{display:inline-block;margin:0 0 12px;font-size:.9rem;letter-spacing:.5px;text-transform:uppercase;
  border-bottom:2px solid var(--alt);padding-bottom:4px}
.lede{color:var(--muted);line-height:1.5;margin:0 0 16px}

/* Buttons: heavy border and chamfered corners, lifting on hover */
.btn{border:2px solid var(--border);background:var(--plate);color:var(--text);font-weight:700;
  letter-spacing:1px;text-transform:uppercase;padding:10px 18px;cursor:pointer;clip-path:var(--chamfer-clip);
  transition:transform .1s,background-color .1s,border-color .1s,color .1s}
.btn:hover:not(:disabled){transform:translate(-1px,-1px);background:var(--hover);border-color:var(--text)}
.btn:active:not(:disabled){transform:translate(1px,1px)}
.btn:disabled{opacity:.4;cursor:default}
.btn-accent{background:var(--accent);border-color:var(--outline);color:#fff}
.btn-accent:hover:not(:disabled){background:var(--accent-hover);border-color:var(--outline)}
.btn-danger{background:var(--red);border-color:var(--outline);color:#fff}
.btn-danger:hover:not(:disabled){background:var(--red-hover);border-color:var(--outline)}
.row{display:flex;gap:10px;flex-wrap:wrap}
.row .btn{flex:1 1 140px}

/* ---- Drive: the whole window is the control panel ---- */
body.mode-drive{height:100vh;height:100dvh;overflow:hidden;display:flex;flex-direction:column;
  overscroll-behavior:none;touch-action:manipulation}
body.mode-drive footer{display:none}
.deck{flex:1;min-height:0;position:relative;display:grid;gap:20px;
  padding:0 var(--gutter-r) max(16px,env(safe-area-inset-bottom)) var(--gutter-l);
  grid-template-columns:minmax(0,1fr) minmax(220px,1.2fr) minmax(0,1.5fr);
  grid-template-rows:minmax(0,1fr);grid-template-areas:"moves console dial";align-items:center}
/* Hold-to-move surfaces: no text selection, callouts, scrolling or zooming under a finger */
.moves,.dial{touch-action:none;user-select:none;-webkit-user-select:none;-webkit-touch-callout:none}
.drive-off .moves,.drive-off .dial{opacity:.4;pointer-events:none}
.is-missing{opacity:.25}

/* Body moves: one plate split into cells, two columns under the left thumb */
.moves{grid-area:moves;justify-self:center;width:100%;height:100%;max-width:420px;max-height:560px;
  display:grid;grid-template-columns:repeat(2,1fr);grid-template-rows:repeat(3,1fr);grid-auto-flow:column;
  gap:2px;background:var(--border-light);border:2px solid var(--border);box-shadow:var(--shadow)}
.cell{border:0;background:var(--plate);color:var(--text);cursor:pointer;padding:6px;min-width:0;min-height:0;
  display:flex;flex-direction:column;align-items:center;justify-content:center;gap:2px;
  font-weight:700;letter-spacing:1px;text-transform:uppercase;font-size:clamp(.75rem,1.8vmin,1rem);
  transition:background .08s}
.cell b{font-size:clamp(1.5rem,6vmin,3.25rem);line-height:1}
@media (hover:hover){.cell:hover{background:var(--hover);color:var(--accent)}}
.cell.is-held,.cell.is-held:hover{background:var(--accent);color:#fff}

/* Console: speed, relax and full screen between the two pads. The speed
   readout and its slider share one plate; the buttons sit well clear of it. */
.console{grid-area:console;justify-self:center;width:100%;max-width:340px;
  display:flex;flex-direction:column;gap:28px}
.gauge{background:var(--plate);border:2px solid var(--border);border-left:6px solid var(--accent);
  box-shadow:var(--shadow);padding:10px 14px 12px}
.gauge-head{display:flex;align-items:baseline;justify-content:space-between;gap:8px;margin-bottom:8px}
.gauge .label{margin:0}
.gauge .speed-value{font-size:2rem;line-height:1}
/* Relax and full screen are occasional, so they stay small and quiet: a thin
   outline that only lights up on hover. Wake stays loud while the servos are limp. */
.actions{display:flex;gap:8px}
.actions .btn{flex:1;padding:5px 8px;font-size:.8rem;background:transparent;color:var(--muted);
  border-color:var(--border-light)}
.actions .btn:hover:not(:disabled){transform:none;background:transparent;border-color:var(--accent);color:var(--accent)}
.actions .btn-danger:hover:not(:disabled){border-color:var(--red);color:var(--red)}
.actions .btn-accent,.actions .btn-accent:hover:not(:disabled){background:var(--accent);border-color:var(--outline);color:#fff}
.console .hint{margin:0;text-align:center}

/* Speed slider: a thick track and a big square thumb, easy to catch with a thumb */
.slider{-webkit-appearance:none;appearance:none;display:block;width:100%;height:34px;margin:0;
  background:transparent;cursor:pointer;touch-action:none;--fill:50%}
.slider:focus{outline:none}
.slider::-webkit-slider-runnable-track{height:12px;border:2px solid var(--border);
  background:linear-gradient(to right,var(--accent) var(--fill),var(--input-bg) var(--fill))}
.slider::-webkit-slider-thumb{-webkit-appearance:none;width:24px;height:32px;margin-top:-12px;
  background:var(--plate);border:2px solid var(--border);box-shadow:2px 2px 0 var(--shade)}
.slider:focus-visible::-webkit-slider-thumb{border-color:var(--accent);box-shadow:var(--glow)}
.slider::-moz-range-track{height:12px;border:2px solid var(--border);background:var(--input-bg)}
.slider::-moz-range-progress{height:12px;border:2px solid var(--border);border-right:0;background:var(--accent)}
.slider::-moz-range-thumb{width:24px;height:32px;border:2px solid var(--border);border-radius:0;
  background:var(--plate);box-shadow:2px 2px 0 var(--shade)}
.slider:focus-visible::-moz-range-thumb{border-color:var(--accent);box-shadow:var(--glow)}

/* Steering dial: walk directions on the inner ring, fast and turn on the outer,
   the red stop button in the middle. In the cockpit it glows like the monitor. */
.dial{grid-area:dial;width:100%;height:100%;filter:drop-shadow(4px 4px 0 var(--shade))}
:root[data-theme=dark] .dial{filter:drop-shadow(4px 4px 0 var(--shade)) drop-shadow(0 0 12px rgba(76,201,240,.2))}
.dial text{font-family:var(--heading);font-weight:700;text-anchor:middle;dominant-baseline:central;pointer-events:none}
.dial .face{transition:fill .08s}
.ring-out .face{fill:var(--plate);stroke:var(--border);stroke-width:2}
.ring-out .glyph{fill:var(--text)}
.ring-in .face{fill:var(--well);stroke:var(--plate);stroke-width:2}
.ring-in .glyph{fill:#fff}
.hub .face{fill:var(--red);stroke:var(--plate);stroke-width:3}
.hub .glyph{fill:#fff}
@media (hover:hover){.ring-out:hover .face{fill:var(--hover)}.ring-in:hover .face{fill:var(--well-hover)}
  .hub:hover .face{fill:var(--red-hover)}}
.dial .is-held .face,.dial .is-held:hover .face{fill:var(--accent)}
.dial .is-held .glyph{fill:#fff}

/* Drive messages float over the deck instead of taking room from it */
.toast{position:absolute;left:50%;bottom:max(16px,env(safe-area-inset-bottom));transform:translateX(-50%);
  max-width:calc(100% - 32px);background:var(--plate);border:2px solid var(--border);box-shadow:var(--shadow);
  padding:6px 14px;margin:0;pointer-events:none}
.toast:empty{display:none}
.toast.is-error{border-left:6px solid var(--red)}

/* Portrait: console on top, then the dial, then the moves three across */
@media (orientation:portrait){
  .deck{grid-template-columns:minmax(0,1fr);grid-template-rows:auto minmax(0,1.6fr) minmax(0,1fr);
    grid-template-areas:"console" "dial" "moves";gap:16px}
  .moves{grid-template-columns:repeat(3,1fr);grid-template-rows:repeat(2,1fr);grid-auto-flow:row;max-width:none}
  .console{max-width:none;gap:14px}
  .gauge .speed-value{font-size:1.5rem}
  .console .hint{display:none}
}
/* Short landscape screens (phones): squeeze the chrome, keep the pads big */
@media (orientation:landscape) and (max-height:520px){
  header{padding-top:max(6px,env(safe-area-inset-top));padding-bottom:6px;margin-bottom:16px}
  .brand{font-size:1.05rem;letter-spacing:1px}
  .robot{display:none}
  .deck{gap:14px;padding-bottom:max(10px,env(safe-area-inset-bottom))}
  .console{gap:22px}
  .gauge{padding:6px 12px 8px}
  .gauge-head{margin-bottom:4px}
  .gauge .speed-value{font-size:1.5rem}
  .console .hint{display:none}
}
/* Phones: brand, theme and status on the first row, the tabs and a truncated
   robot name on the second, so the header leaves the pads their room */
@media (max-width:575px){
  .brand{font-size:1.1rem;letter-spacing:1px}
  .theme-btn{order:1}
  .chip{order:2;padding:4px 10px}
  .tabs{order:3}
  .tab{padding:4px 10px}
  .robot{order:4;flex:1 1 0;min-width:0;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
}
@media (hover:none){.keys{display:none}}

/* Offsets: one block per leg, the leg header styled like the app's */
/* Two columns fill top to bottom: left legs on the left, right legs on the right */
.legs{display:grid;grid-template-columns:1fr 1fr;grid-template-rows:repeat(3,auto);grid-auto-flow:column;
  gap:16px 24px;margin:20px 0}
@media (max-width:720px){.legs{grid-template-columns:1fr;grid-template-rows:none;grid-auto-flow:row}}
.leg-head{font-family:var(--mono);font-size:.85rem;font-weight:700;text-transform:uppercase;background:var(--bg);
  border-left:4px solid var(--accent);padding:4px 8px;margin-bottom:8px}
.joints{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}
.label{font-family:var(--mono);font-size:.75rem;font-weight:600;color:var(--muted);text-transform:uppercase;margin-bottom:4px}
.stepper{display:flex}
.stepper button{width:30px;flex:none;border:2px solid var(--border-light);background:var(--surface);color:var(--text);
  font-family:var(--mono);font-weight:700;cursor:pointer;padding:0}
.stepper button:hover:not(:disabled){border-color:var(--accent);color:var(--accent)}
input[type=number]{width:100%;min-width:0;font-family:var(--mono);font-size:.95rem;font-weight:600;text-align:center;
  background:var(--input-bg);border:2px solid var(--border-light);border-left:0;border-right:0;color:var(--text);
  padding:6px 2px;border-radius:0;box-shadow:inset 2px 2px 0 rgba(0,0,0,.05);-moz-appearance:textfield}
input[type=number]::-webkit-inner-spin-button,input[type=number]::-webkit-outer-spin-button{-webkit-appearance:none;margin:0}
input[type=number]:focus{outline:none;background:var(--input-focus-bg);border-color:var(--accent);
  box-shadow:inset 2px 2px 0 rgba(0,0,0,.05),var(--glow)}
.legs.is-idle{opacity:.4;filter:grayscale(1);pointer-events:none}

.speed-value{font-family:var(--mono);font-weight:700;font-size:1.3rem;min-width:4ch;text-align:right}
.hint{color:var(--muted);font-size:.9rem;margin-top:8px}

/* Messages and the safety callout */
.message{font-family:var(--mono);font-weight:600;text-align:center;min-height:1.4em;margin-top:14px}
.message.is-error{color:var(--alert-text)}
.message.is-ok{color:var(--ok)}
.callout{border-left:6px solid var(--red);background:var(--surface);padding:10px 14px;margin:0 0 16px;
  color:var(--muted);line-height:1.5}
.callout b{color:var(--red);font-size:.8rem;text-transform:uppercase;letter-spacing:.5px}
footer{text-align:center;color:var(--muted);font-family:var(--mono);font-size:.8rem;padding:8px 0 24px}

::-webkit-scrollbar{width:10px}
::-webkit-scrollbar-track{background:var(--bg);border-left:1px solid var(--border-light)}
::-webkit-scrollbar-thumb{background:var(--border);border:1px solid var(--bg)}
::-webkit-scrollbar-thumb:hover{background:var(--accent)}
</style>
</head>
<body class="mode-drive">
<header>
  <div class="brand">Hexapod</div>
  <div class="robot" id="robot"></div>
  <div class="tabs" role="tablist">
    <button class="tab" role="tab" id="tabDrive" aria-selected="true" onclick="showTab('drive')">Drive</button>
    <button class="tab" role="tab" id="tabCalibrate" aria-selected="false" onclick="showTab('calibrate')">Calibrate</button>
  </div>
  <button class="theme-btn" id="themeBtn" onclick="toggleTheme()"></button>
  <div class="chip" id="chip">Idle</div>
</header>

<main id="drive" class="deck drive-off">
  <div class="moves" id="moves">
    <button class="cell" data-cmd="rotatey"><b>&harr;</b>Rotate Y</button>
    <button class="cell" data-cmd="rotatex"><b>&varr;</b>Rotate X</button>
    <button class="cell" data-cmd="rotatez"><b>&#8635;</b>Rotate Z</button>
    <button class="cell" data-cmd="climbforward"><b>&uArr;</b>Climb</button>
    <button class="cell" data-cmd="twist"><b>&#8645;</b>Twist</button>
    <button class="cell" data-cmd="climbbackward"><b>&dArr;</b>Climb</button>
  </div>

  <div class="console">
    <div class="gauge">
      <div class="gauge-head">
        <span class="label">Gait speed</span>
        <span class="speed-value" id="speedValue">60%</span>
      </div>
      <input type="range" class="slider" id="speedSlider" min="20" max="100" step="5" value="60"
             aria-label="Gait speed" oninput="showSpeed(this.value)" onchange="setSpeed(this.value)">
    </div>
    <div class="actions">
      <button class="btn btn-danger" id="relaxBtn" onclick="relax()">Relax</button>
      <button class="btn" id="fullBtn" onclick="toggleFullscreen()" hidden>Full screen</button>
    </div>
    <p class="hint keys">W A S D walk &middot; Q E turn &middot; Space stops</p>
  </div>

  <svg class="dial" id="dial" viewBox="0 0 300 300" role="group" aria-label="Steering dial"></svg>

  <p class="message toast" id="driveMessage">Connecting to the robot...</p>
</main>

<main id="calibrate" class="page" hidden>
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
let linked = null;  // Whether /robot_config answered: null until it does
let activePointer = null;
const heldKeys = new Set();

const $ = id => document.getElementById(id);

function say(text, kind, id) {
  const el = $(id || 'message');
  el.textContent = text;
  el.className = el.className.replace(/ ?is-(ok|error)/g, '') + (kind ? ' is-' + kind : '');
}

// Resolves with the reply; rejects with the firmware's own error text.
function request(url, options) {
  return fetch(url, options).then(r => r.text().then(text => {
    if (!r.ok) throw new Error(text || ('HTTP ' + r.status));
    return text;
  }));
}

// The chip's state names are the app's link readout: online (green), streaming
// (yellow, pulsing) and fault (red); a relaxed or not yet linked robot is grey.
function updateChip() {
  const moving = !!driving && driving !== 'standby';
  let text = 'Idle', state = linked ? 'online' : '';
  if (linked === false) {
    text = 'Offline';
    state = 'fault';
  } else if (calibrating) {
    text = 'Calibrating';
    state = 'streaming';
  } else if (moving) {
    text = 'Driving';
    state = 'streaming';
  } else if (relaxed) {
    text = 'Relaxed';
    state = '';
  }
  $('chip').textContent = text;
  $('chip').className = 'chip' + (state ? ' is-' + state : '');
  document.body.classList.toggle('is-driving', moving);
  $('relaxBtn').textContent = relaxed ? 'Wake' : 'Relax';
  $('relaxBtn').className = 'btn ' + (relaxed ? 'btn-accent' : 'btn-danger');
}

// ---------------------------------------------------------------------------
// Theme
// ---------------------------------------------------------------------------

// The button offers the other theme, so it shows that one's icon. U+FE0E keeps
// the sun a text glyph rather than an emoji.
const THEME_ICON = { light: '☾', dark: '☀︎' };
const THEME_TITLE = { light: 'Switch to dark theme', dark: 'Switch to light theme' };
const THEME_BAR = { light: '#f3f5f9', dark: '#0c1730' };
const darkQuery = matchMedia('(prefers-color-scheme: dark)');

function applyTheme(theme) {
  document.documentElement.setAttribute('data-theme', theme);
  $('themeColor').content = THEME_BAR[theme];
  $('themeBtn').textContent = THEME_ICON[theme];
  $('themeBtn').title = THEME_TITLE[theme];
  $('themeBtn').setAttribute('aria-label', THEME_TITLE[theme]);
}

function toggleTheme() {
  const theme = document.documentElement.getAttribute('data-theme') === 'dark' ? 'light' : 'dark';
  try { localStorage.setItem('theme', theme); } catch (e) {}
  applyTheme(theme);
}

// Follow the device's setting until a theme is picked with the toggle.
darkQuery.addEventListener('change', e => {
  let saved = null;
  try { saved = localStorage.getItem('theme'); } catch (err) {}
  if (!saved) applyTheme(e.matches ? 'dark' : 'light');
});

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
  document.body.classList.toggle('mode-drive', name === 'drive');
  $('drive').hidden = name !== 'drive';
  $('calibrate').hidden = name !== 'calibrate';
  $('tabDrive').setAttribute('aria-selected', name === 'drive');
  $('tabCalibrate').setAttribute('aria-selected', name === 'calibrate');
}

// ---------------------------------------------------------------------------
// Steering dial
// ---------------------------------------------------------------------------

// Radii in viewBox units: hub (stop), inner ring (walk), outer ring (fast, turn).
const C = 150, R_HUB = 38, R_IN = 100, R_OUT = 146;
// Angles in degrees, clockwise from pointing right (screen coordinates).
const OUTER = [['fastforward', -90, '⇑', 'Fast', 'Fast forward'],
               ['turnright', 0, '↻', 'Turn', 'Turn right'],
               ['fastbackward', 90, '⇓', 'Fast', 'Fast backward'],
               ['turnleft', 180, '↺', 'Turn', 'Turn left']];
const INNER = ['walk0', 'walkr45', 'walkr90', 'walkr135', 'walk180', 'walkl135', 'walkl90', 'walkl45'];
const INNER_LABELS = ['forward', 'forward right', 'right', 'back right', 'back', 'back left', 'left', 'forward left'];

function polar(r, deg) {
  const a = deg * Math.PI / 180;
  return [C + r * Math.cos(a), C + r * Math.sin(a)];
}

function sector(r0, r1, a0, a1) {
  const p = (r, a) => polar(r, a).map(v => v.toFixed(1)).join(',');
  return `M${p(r1, a0)}A${r1},${r1} 0 0 1 ${p(r1, a1)}L${p(r0, a1)}A${r0},${r0} 0 0 0 ${p(r0, a0)}Z`;
}

function buildDial() {
  let svg = '';
  OUTER.forEach(([cmd, a, glyph, label, title]) => {
    // Top and bottom stack glyph and label along the ring; the sides, glyph over label.
    const [gx, gy] = polar(a % 180 ? 128 : 123, a);
    const [lx, ly] = a % 180 ? polar(107, a) : [gx, gy + 20];
    svg += `<g class="ring-out" data-cmd="${cmd}"><title>${title}</title>
      <path class="face" d="${sector(R_IN, R_OUT, a - 45, a + 45)}"/>
      <text class="glyph" x="${gx}" y="${gy}" font-size="26">${glyph}</text>
      <text class="glyph" x="${lx}" y="${ly}" font-size="10" letter-spacing="1">${label.toUpperCase()}</text></g>`;
  });
  INNER.forEach((cmd, i) => {
    const a = -90 + 45 * i;
    svg += `<g class="ring-in" data-cmd="${cmd}"><title>Walk ${INNER_LABELS[i]}</title>
      <path class="face" d="${sector(R_HUB, R_IN, a - 22.5, a + 22.5)}"/>
      <path class="glyph" transform="rotate(${a + 90} ${C} ${C})" d="M${C},${C - 82}l-9,15h18z"/></g>`;
  });
  svg += `<g class="hub" data-cmd="standby"><title>Stop</title>
    <circle class="face" cx="${C}" cy="${C}" r="${R_HUB}"/>
    <rect class="glyph" x="${C - 10}" y="${C - 10}" width="20" height="20"/></g>`;
  $('dial').innerHTML = svg;
}

// The dial zone under a pointer, by distance and angle from the centre, so a
// finger can slide from one zone to the next without lifting.
function dialZone(e) {
  const box = $('dial').getBoundingClientRect();
  const scale = Math.min(box.width, box.height) / 300;
  const x = (e.clientX - box.left - box.width / 2) / scale;
  const y = (e.clientY - box.top - box.height / 2) / scale;
  const r = Math.hypot(x, y);
  if (r > R_OUT + 4) return null;
  if (r < R_HUB) return 'standby';
  const deg = (Math.atan2(y, x) * 180 / Math.PI + 360) % 360;
  if (r < R_IN) return INNER[Math.round((deg + 90) / 45) % 8];
  return OUTER[(Math.round(deg / 90) + 1) % 4][0];
}

// The move cell under a pointer.
function cellZone(e) {
  const el = document.elementFromPoint(e.clientX, e.clientY);
  const cell = el && el.closest('.cell');
  return cell && $('moves').contains(cell) ? cell.dataset.cmd : null;
}

// Hold-to-move on a pad: press drives the zone under the finger, sliding
// switches zones, lifting stops. Sliding off the pad keeps the last zone.
function bindPad(pad, zoneAt) {
  pad.addEventListener('pointerdown', e => {
    if (e.button !== 0) return;
    const cmd = zoneAt(e);
    if (!cmd) return;
    e.preventDefault();
    activePointer = e.pointerId;
    drive(cmd);
    // Keep receiving this pointer's events even once it leaves the pad. A
    // failed capture only loses that, never the press itself.
    try {
      pad.setPointerCapture(e.pointerId);
    } catch (err) {}
  });
  pad.addEventListener('pointermove', e => {
    if (e.pointerId !== activePointer) return;
    const cmd = zoneAt(e);
    if (cmd) drive(cmd);
  });
  const release = e => {
    if (e.pointerId === activePointer) stop();
  };
  pad.addEventListener('pointerup', release);
  pad.addEventListener('pointercancel', release);
  pad.addEventListener('lostpointercapture', release);
  pad.addEventListener('contextmenu', e => e.preventDefault());
  // Android Chrome buzzes when a touch is held, as it starts a long-press
  // (context menu, text selection) gesture. Cancelling the touch itself stops
  // that gesture from ever starting; the pointer events above still arrive.
  pad.addEventListener('touchstart', e => {
    if (e.cancelable) e.preventDefault();
  }, { passive: false });
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
  document.querySelectorAll('[data-cmd]').forEach(el =>
    el.classList.toggle('is-held', el.dataset.cmd === driving));
}

// Start, or switch to, holding motion `name`.
function drive(name) {
  if (calibrating || commands.indexOf(name) < 0) return;
  const changed = name !== driving;
  driving = name;
  if (relaxed) {
    relaxed = false;
    say('', '', 'driveMessage');
  }
  if (!driveTimer) driveTimer = setInterval(sendMotion, DRIVE_MS);
  if (changed) {
    sendMotion();
    markHeld();
    updateChip();
  }
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

// Relax the servos, or wake them again: any motion command, standby included,
// re-enables the drivers.
function relax() {
  heldKeys.clear();
  halt();
  if (relaxed) {
    relaxed = false;
    updateChip();
    sendMotion();
    say('', '', 'driveMessage');
    return;
  }
  request('/relax', { method: 'POST' }).then(() => {
    relaxed = true;
    updateChip();
    say('Servos relaxed. Tap Wake or any control to resume.', '', 'driveMessage');
  }).catch(e => say(e.message, 'error', 'driveMessage'));
}

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
  const slider = $('speedSlider');
  slider.value = value;
  // The track fills up to the thumb.
  slider.style.setProperty('--fill', (slider.value - slider.min) / (slider.max - slider.min) * 100 + '%');
  $('speedValue').textContent = slider.value + '%';
}

function setSpeed(value) {
  request('/set_speed?pct=' + value, { method: 'POST' })
    .then(text => showSpeed(JSON.parse(text).speed))
    .catch(e => say(e.message, 'error', 'driveMessage'));
}

// Full screen where the browser allows it (Android, iPad, desktop). iPhone
// Safari has no full-screen API; adding the page to the home screen opens it
// without the browser bars instead.
const root = document.documentElement;
const enterFull = root.requestFullscreen || root.webkitRequestFullscreen;
const exitFull = document.exitFullscreen || document.webkitExitFullscreen;
const fullElement = () => document.fullscreenElement || document.webkitFullscreenElement;

function toggleFullscreen() {
  if (fullElement()) {
    exitFull.call(document);
    return;
  }
  const pending = enterFull.call(root);
  // Hold the current orientation so the panel does not flip mid-drive.
  const lock = () => {
    try {
      screen.orientation.lock(screen.orientation.type.split('-')[0]).catch(() => {});
    } catch (e) {}
  };
  if (pending && pending.then) pending.then(lock).catch(() => {});
  else lock();
}

function updateFullButton() {
  $('fullBtn').textContent = fullElement() ? 'Exit full screen' : 'Full screen';
}

if (enterFull) {
  $('fullBtn').hidden = false;
  document.addEventListener('fullscreenchange', updateFullButton);
  document.addEventListener('webkitfullscreenchange', updateFullButton);
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

applyTheme(document.documentElement.getAttribute('data-theme'));
buildDial();
showSpeed($('speedSlider').value);
bindPad($('dial'), dialZone);
bindPad($('moves'), cellZone);
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
  // Dim moves this firmware does not have, so IDs always come from the robot.
  commands = config.commands || [];
  document.querySelectorAll('[data-cmd]').forEach(el =>
    el.classList.toggle('is-missing', commands.indexOf(el.dataset.cmd) < 0));
  $('drive').classList.remove('drive-off');
  linked = true;
  updateChip();
  say('', '', 'driveMessage');
}).catch(e => {
  linked = false;
  updateChip();
  say(e.message, 'error', 'driveMessage');
});
</script>
</body>
</html>
)rawliteral";

#endif  // WEB_PAGE_H
