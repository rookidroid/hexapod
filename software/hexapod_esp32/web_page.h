/**

  Robot web page -- drive, calibration and WiFi setup, served verbatim from flash by
  web_ui.ino

  - Copyright (C) 2024 - PRESENT  rookidroid.com
  - E-mail: info@rookidroid.com
  - Website: https://rookidroid.com/

  Styled after Hexapod Link (github.com/rookidroid/hexapod-link), so the robot's
  own page and the desktop app look and name things the same way: legs are
  "Right/Left Leg 1-3" front to back, joints "Joint 1-3" outward from the body.
  It carries the app's workstation theme -- RX-78-2 colours, thin navy frames,
  filled-in plate corners, the chest trim and the LED status pill -- in both its
  light and dark (cockpit) variants, following the device's setting until the
  header toggle picks one.
  The app's fonts are named first but not bundled -- the robot's access point
  has no route to the internet -- so the device's own fonts stand in for them.

  The Drive tab fills the window like a hardware control panel, laid out as in
  the Android app: body moves under the left thumb, the steering dial under the
  right.

*/

#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

// HTML page for the drive, calibration and WiFi interface
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
/* Hexapod Link's workstation theme (src/renderer/src/styles/industrial.css in
   that repo), after the RX-78-2: white armour plates framed in navy,
   Federation blue as the accent, red and V-fin yellow as trim. Dark is the
   cockpit, navy plates with a cyan HUD glow on focus. The markings are kept
   to the app's few: the chest trim under the top bar, the filled-in corners
   on plates, and the display face on headings. The tokens carry the app's
   values under shorter names (--plate is its --ind-surface-alt, --hairline
   its --ind-border, --frame its --ind-frame). The theme is data-theme on
   <html>, set before first paint by the script in <head>. */
:root{color-scheme:light;
  --bg:#dde3ec;--surface:#f3f5f9;--plate:#fff;--hairline:#c9d2e0;--border-strong:#a9b6cc;--frame:#1b2a4a;
  --accent:#1d4fa3;--accent-hover:#163f84;--accent-active:#10306a;--alt:#f2b705;--red:#d62828;--ok:#16a34a;
  --outline:#0f1a30;--text:#0f1a30;--muted:#4a5875;--hover:#e8eef8;--input-bg:#eef2f8;--track:#c9d2e0;
  --alert-text:#9b1c1c;--alert-bg:#fdecec;--well:#1b2a4a;--well-hover:#2c3f66;
  --hud-rgb:29,79,163;--hud-bg:rgba(246,248,251,.88);--hud-border:rgba(var(--hud-rgb),.3);
  --glow:0 0 0 2px rgba(29,79,163,.28);--glow-inset:inset 0 0 0 2px rgba(29,79,163,.55);
  --shadow-soft:0 2px 10px rgba(15,26,48,.12);
  --font-ui:'Chakra Petch',system-ui,-apple-system,'Segoe UI',Roboto,sans-serif;
  --font-display:'Michroma',var(--font-ui);
  --font-mono:'IBM Plex Mono',ui-monospace,Menlo,Consolas,'Courier New',monospace;
  --topbar-h:52px;--trim-w:68px;--chamfer:7px;
  --gutter-l:max(16px,env(safe-area-inset-left));--gutter-r:max(16px,env(safe-area-inset-right));
}
:root[data-theme=dark]{color-scheme:dark;
  --bg:#060c18;--surface:#0c1730;--plate:#11203f;--hairline:#1a2b4d;--border-strong:#2c4475;--frame:#2f5fb3;
  --accent:#3d7cf0;--accent-hover:#2f68d6;--accent-active:#2455b4;--alt:#ffd23f;--red:#ff4d4d;--ok:#22c55e;
  --outline:#02060f;--text:#e4eeff;--muted:#8aa2c8;--hover:#18294d;--input-bg:#08122a;--track:#24396a;
  --alert-text:#ffb4b4;--alert-bg:rgba(255,77,77,.12);--well:#1a3366;--well-hover:#22407d;
  --hud-rgb:76,201,240;--hud-bg:rgba(8,16,34,.84);--hud-border:rgba(var(--hud-rgb),.28);
  --glow:0 0 0 1px rgba(76,201,240,.45),0 0 14px rgba(76,201,240,.25);--glow-inset:inset 0 0 0 2px rgba(76,201,240,.7);
  --shadow-soft:0 2px 10px rgba(0,0,0,.55);
}
*{box-sizing:border-box}
[hidden]{display:none!important}
body{margin:0;font-family:var(--font-ui);background:var(--bg);color:var(--text)}
button,input{font:inherit}
/* Michroma has one weight; where it is missing the stand-in is set bold instead */
.brand,h2{font-family:var(--font-display);font-weight:700;font-synthesis:none}

/* ---- Top bar: the app's, down to the chest trim along its bottom edge ---- */
header{position:relative;z-index:5;display:flex;align-items:center;gap:8px 12px;flex-wrap:wrap;
  min-height:var(--topbar-h);background:var(--surface);box-shadow:var(--shadow-soft);
  padding:max(8px,env(safe-area-inset-top)) var(--gutter-r) 12px var(--gutter-l);margin-bottom:20px}
/* A red block, a few yellow vent slats, then Federation blue, as on the RX-78's torso */
header::after{content:'';position:absolute;left:0;right:0;bottom:0;height:4px;pointer-events:none;
  background:linear-gradient(90deg,var(--red) 0 var(--trim-w),transparent var(--trim-w)),
    repeating-linear-gradient(90deg,var(--alt) 0 10px,transparent 10px 16px) calc(var(--trim-w) + 6px) 0/76px 100% no-repeat,
    var(--accent)}
.brand{font-size:1.05rem;letter-spacing:2px;text-transform:uppercase;white-space:nowrap}
/* The robot's name, and the network it is on, as the app's release stamp: a small stencilled plate */
.robot{padding:1px 6px;font-family:var(--font-mono);font-size:.68rem;font-weight:600;color:var(--muted);
  border:1px solid var(--border-strong);white-space:nowrap}
.robot:empty{display:none}

/* Tabs: one segmented plate, the one picked filled in the accent */
.tabs{display:flex}
.tab{height:31px;padding:0 12px;font-size:.82rem;font-weight:600;letter-spacing:.4px;color:var(--muted);
  background-color:var(--plate);border:1px solid var(--border-strong);cursor:pointer;
  transition:color .15s,background-color .15s}
.tab+.tab{border-left:0}
.tab:hover{color:var(--text);background-color:var(--hover)}
.tab[aria-selected=true]{color:#fff;background-color:var(--accent);border-color:var(--accent)}

/* Status pill: the app's link readout, an LED coloured by state */
.chip{margin-left:auto;display:inline-flex;align-items:center;gap:8px;height:31px;padding:0 13px 0 11px;
  font-family:var(--font-mono);font-size:.78rem;font-weight:600;white-space:nowrap;
  color:var(--text);background-color:var(--plate);border:1px solid var(--frame)}
.chip::before{content:'';flex:none;width:8px;height:8px;border-radius:50%;background:var(--muted)}
.chip.is-online::before{background:var(--ok)}
.chip.is-fault{border-color:var(--red)}
.chip.is-fault::before{background:var(--red)}
.chip.is-streaming::before{background:var(--alt);box-shadow:0 0 0 3px color-mix(in srgb,var(--alt) 30%,transparent);
  animation:led 1s ease-in-out infinite}
@keyframes led{50%{opacity:.35}}

/* Theme toggle: shows the icon of the theme it switches to */
.theme-btn{width:31px;height:31px;display:inline-flex;align-items:center;justify-content:center;padding:0;
  font-size:1rem;line-height:1;color:var(--text);background:transparent;border:1px solid transparent;cursor:pointer;
  transition:background-color .15s}
.theme-btn:hover{background:var(--hover)}

/* Keyboard focus: the app's ring, drawn inside, as the controls sit edge to edge */
.btn:focus-visible,.tab:focus-visible,.theme-btn:focus-visible,.stepper button:focus-visible,.net:focus-visible,.cell:focus-visible{
  outline:none;box-shadow:var(--glow-inset)}

.page{max-width:960px;margin:0 auto;padding:0 var(--gutter-r) 24px var(--gutter-l)}

/* Cards: plates on the panel-line grey, framed in navy */
.card{position:relative;background-color:var(--surface);border:1px solid var(--frame);box-shadow:var(--shadow-soft);
  padding:16px 18px 20px;margin-bottom:20px;--chamfer:10px}
/* A section's title, in the display face, underlined in V-fin yellow */
h2{display:inline-block;margin:0 0 12px;padding-bottom:3px;font-size:.74rem;letter-spacing:.08em;
  text-transform:uppercase;border-bottom:2px solid var(--alt)}
.lede{color:var(--muted);line-height:1.5;margin:0 0 16px}

/* Buttons: armour plates, a navy border with two corners filled in */
.btn{--chamfer:10px;min-height:36px;padding:6px 16px;font-weight:700;letter-spacing:.4px;white-space:nowrap;
  color:var(--text);background-color:var(--plate);border:1px solid var(--frame);cursor:pointer;
  transition:filter .1s,background-color .1s,border-color .1s,color .1s}
.btn:hover:not(:disabled){background-color:var(--hover);filter:brightness(1.06)}
.btn:disabled{color:var(--muted);border-color:var(--border-strong);cursor:default}
.btn-accent{color:#fff;background-color:var(--accent);border-color:var(--outline)}
.btn-accent:hover:not(:disabled){background-color:var(--accent-hover)}
.btn-accent:active:not(:disabled){background-color:var(--accent-active)}
.btn-accent:disabled{color:#fff;border-color:var(--outline);opacity:.55}
/* Danger is drawn in outline, and fills red under the pointer */
.btn-danger{color:var(--red);background-color:transparent;border-color:color-mix(in srgb,var(--red) 55%,transparent)}
.btn-danger:hover:not(:disabled){color:#fff;background-color:var(--red);border-color:var(--red)}
.row{display:flex;gap:10px;flex-wrap:wrap}
.row .btn{flex:1 1 140px}

/* ---- Drive: the whole window is the control panel ---- */
body.mode-drive{height:100vh;height:100dvh;overflow:hidden;display:flex;flex-direction:column;
  overscroll-behavior:none;touch-action:manipulation}
body.mode-drive footer{display:none}
/* While the robot moves, the app's streaming glow round the edge of the window */
body.mode-drive::after{content:'';position:fixed;inset:0;z-index:10;pointer-events:none;opacity:0;
  transition:opacity .3s;box-shadow:inset 0 0 0 3px var(--alt),inset 0 0 60px 10px color-mix(in srgb,var(--alt) 45%,transparent)}
body.mode-drive.is-driving::after{opacity:1;animation:glow 2s ease-in-out infinite}
@keyframes glow{50%{box-shadow:inset 0 0 0 3px var(--alt),inset 0 0 34px 4px color-mix(in srgb,var(--alt) 45%,transparent)}}
@media (prefers-reduced-motion:reduce){body.mode-drive.is-driving::after,.chip.is-streaming::before{animation:none}}
.deck{flex:1;min-height:0;position:relative;display:grid;gap:20px;
  padding:0 var(--gutter-r) max(16px,env(safe-area-inset-bottom)) var(--gutter-l);
  grid-template-columns:minmax(0,1fr) minmax(220px,1.2fr) minmax(0,1.5fr);
  grid-template-rows:minmax(0,1fr);grid-template-areas:"moves console dial";align-items:center}
/* Hold-to-move surfaces: no text selection, callouts, scrolling or zooming under a finger */
.moves,.dial{touch-action:none;user-select:none;-webkit-user-select:none;-webkit-touch-callout:none}
.drive-off .moves,.drive-off .dial{opacity:.45;filter:grayscale(1);pointer-events:none}
.is-missing{opacity:.25}

/* Body moves: one plate split into cells by hairlines, two columns under the left thumb */
.moves{grid-area:moves;justify-self:center;width:100%;height:100%;max-width:420px;max-height:560px;
  display:grid;grid-template-columns:repeat(2,1fr);grid-template-rows:repeat(3,1fr);grid-auto-flow:column;
  gap:1px;background:var(--hairline);border:1px solid var(--frame);box-shadow:var(--shadow-soft)}
.cell{border:0;background:var(--plate);color:var(--text);cursor:pointer;padding:6px;min-width:0;min-height:0;
  display:flex;flex-direction:column;align-items:center;justify-content:center;gap:4px;
  font-weight:700;letter-spacing:.4px;font-size:clamp(.75rem,1.8vmin,1rem);transition:background-color .08s}
.cell b{font-size:clamp(1.5rem,6vmin,3.25rem);font-weight:400;line-height:1;color:var(--accent)}
@media (hover:hover){.cell:hover{background:var(--hover)}}
.cell.is-held,.cell.is-held:hover{background:var(--accent);color:#fff}
.cell.is-held b{color:#fff}

/* Console: speed, relax and full screen between the two pads. The speed
   readout and its slider share one plate; the buttons sit well clear of it. */
.console{grid-area:console;justify-self:center;width:100%;max-width:340px;
  display:flex;flex-direction:column;gap:28px}
.gauge{--chamfer:10px;background-color:var(--plate);border:1px solid var(--frame);box-shadow:var(--shadow-soft);
  padding:10px 14px 8px}
.gauge-head{display:flex;align-items:baseline;justify-content:space-between;gap:8px}
.gauge .label{margin:0}
.gauge .speed-value{font-size:2rem;line-height:1}
/* Relax and full screen are occasional, so they stay small and quiet: a thin
   outline that only lights up on hover. Wake stays loud while the servos are limp. */
.actions{display:flex;gap:8px}
.actions .btn{flex:1;min-height:30px;padding:4px 8px;font-size:.8rem}
.actions .btn:not(.btn-danger):not(.btn-accent){color:var(--muted);background-color:transparent;border-color:var(--border-strong)}
.actions .btn:not(.btn-danger):not(.btn-accent):hover{color:var(--accent);border-color:var(--accent);filter:none}
.console .hint{margin:0;text-align:center;font-family:var(--font-mono);font-size:.72rem}

/* Speed slider: the app's track filled up to a round thumb, sized for a thumb */
.slider{-webkit-appearance:none;appearance:none;display:block;width:100%;height:36px;margin:0;
  background:transparent;cursor:pointer;touch-action:none;--fill:50%}
.slider:focus{outline:none}
.slider::-webkit-slider-runnable-track{height:4px;border-radius:9999px;
  background:linear-gradient(to right,var(--accent) var(--fill),var(--track) var(--fill))}
.slider::-webkit-slider-thumb{-webkit-appearance:none;width:22px;height:22px;margin-top:-9px;border-radius:50%;
  background:var(--accent);border:3px solid var(--plate);box-shadow:0 0 0 1px var(--accent);transition:transform .15s}
.slider:active::-webkit-slider-thumb{transform:scale(1.125)}
.slider:focus-visible::-webkit-slider-thumb{box-shadow:0 0 0 1px var(--accent),var(--glow)}
.slider::-moz-range-track{height:4px;border-radius:9999px;background:var(--track)}
.slider::-moz-range-progress{height:4px;border-radius:9999px;background:var(--accent)}
.slider::-moz-range-thumb{width:16px;height:16px;border-radius:50%;background:var(--accent);
  border:3px solid var(--plate);box-shadow:0 0 0 1px var(--accent)}
.slider:focus-visible::-moz-range-thumb{box-shadow:0 0 0 1px var(--accent),var(--glow)}

/* Steering dial: walk directions on the inner ring, fast and turn on the outer,
   the red stop button in the middle. In the cockpit it glows like the monitor. */
.dial{grid-area:dial;width:100%;height:100%;filter:drop-shadow(0 2px 8px rgba(15,26,48,.18))}
:root[data-theme=dark] .dial{filter:drop-shadow(0 2px 8px rgba(0,0,0,.5)) drop-shadow(0 0 12px rgba(76,201,240,.18))}
.dial text{font-family:var(--font-ui);font-weight:700;text-anchor:middle;dominant-baseline:central;pointer-events:none}
.dial .face{transition:fill .08s}
.ring-out .face{fill:var(--plate);stroke:var(--frame);stroke-width:1.5}
.ring-out .glyph{fill:var(--text)}
.ring-in .face{fill:var(--well);stroke:var(--plate);stroke-width:1.5}
.ring-in .glyph{fill:#fff}
.hub .face{fill:var(--red);stroke:var(--plate);stroke-width:3}
.hub .glyph{fill:#fff}
@media (hover:hover){.ring-out:hover .face{fill:var(--hover)}.ring-in:hover .face{fill:var(--well-hover)}
  .hub:hover .face{fill:color-mix(in srgb,var(--red) 85%,#000)}}
.dial .is-held .face,.dial .is-held:hover .face{fill:var(--accent)}
.dial .is-held .glyph{fill:#fff}

/* Drive messages float over the deck as a HUD card instead of taking room from it */
.toast{--chamfer:9px;position:absolute;left:50%;bottom:max(16px,env(safe-area-inset-bottom));transform:translateX(-50%);
  max-width:calc(100% - 32px);color:var(--text);background-color:var(--hud-bg);border:1px solid var(--hud-border);
  -webkit-backdrop-filter:blur(6px);backdrop-filter:blur(6px);padding:6px 14px;margin:0;pointer-events:none}
.toast:empty{display:none}
.toast.is-error{color:var(--alert-text);border-left:3px solid var(--red)}

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
  header{min-height:0;padding-top:max(6px,env(safe-area-inset-top));padding-bottom:10px;margin-bottom:14px}
  .robot{display:none}
  .deck{gap:14px;padding-bottom:max(10px,env(safe-area-inset-bottom))}
  .console{gap:22px}
  .gauge{padding:6px 12px 4px}
  .gauge .speed-value{font-size:1.5rem}
  .console .hint{display:none}
}
/* Phones: brand, status and theme on the first row, the tabs and the robot's
   plate on the second, so the header leaves the pads their room */
@media (max-width:575px){
  header{column-gap:8px}
  .brand{font-size:.82rem;letter-spacing:1px}
  .chip{order:1;max-width:11rem;padding:0 10px}
  .theme-btn{order:2}
  .tabs{order:3}
  .tab{padding:0 10px}
  .robot{order:4;flex:0 1 auto;min-width:0;overflow:hidden;text-overflow:ellipsis}
}
@media (hover:none){.keys{display:none}}

/* ---- Offsets: one block per leg, titled like the app's field groups ---- */
/* Two columns fill top to bottom: left legs on the left, right legs on the right */
.legs{display:grid;grid-template-columns:1fr 1fr;grid-template-rows:repeat(3,auto);grid-auto-flow:column;
  gap:14px 28px;margin:20px 0;transition:opacity .15s}
@media (max-width:720px){.legs{grid-template-columns:1fr;grid-template-rows:none;grid-auto-flow:row}}
.leg-head{font-size:.78rem;font-weight:700;letter-spacing:.4px;color:var(--text);
  padding-bottom:4px;margin-bottom:8px;border-bottom:1px dashed var(--hairline)}
.joints{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}
.label{display:block;font-family:var(--font-mono);font-size:.72rem;font-weight:500;color:var(--muted);margin-bottom:3px;
  white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
/* A field between its - and + is one plate: the - has the bottom-left corner, the + the top-right */
.stepper{display:flex}
.stepper button{--chamfer:6px;width:30px;flex:none;padding:0;font-family:var(--font-mono);font-weight:700;cursor:pointer;
  color:var(--text);background-color:var(--plate);border:1px solid var(--border-strong);transition:color .1s,border-color .1s}
.stepper button:hover:not(:disabled){color:var(--accent);border-color:var(--accent)}
input[type=number],input.text{min-width:0;font-family:var(--font-mono);font-size:.85rem;font-weight:500;color:var(--text);
  background-color:var(--input-bg);border:1px solid var(--border-strong);border-radius:0}
input[type=number]{width:100%;text-align:center;border-left:0;border-right:0;padding:6px 2px;-moz-appearance:textfield}
input[type=number]::-webkit-inner-spin-button,input[type=number]::-webkit-outer-spin-button{-webkit-appearance:none;margin:0}
input[type=number]:focus,input.text:focus{outline:none;border-color:var(--accent);box-shadow:var(--glow-inset)}
input.text:disabled{color:var(--muted);background-color:var(--surface)}
.legs.is-idle{opacity:.45;filter:grayscale(1);pointer-events:none}

/* ---- WiFi: a status readout, the networks in range and the join form ---- */
.status{display:grid;grid-template-columns:max-content 1fr;gap:6px 16px;margin:0 0 20px}
.status dt{font-family:var(--font-mono);font-size:.72rem;font-weight:500;color:var(--muted);align-self:center}
.status dd{margin:0;font-family:var(--font-mono);font-size:.85rem;font-weight:600;overflow-wrap:anywhere}
.nets{display:flex;flex-direction:column;gap:6px;margin:12px 0 20px}
.nets:empty{display:none}
.net{--chamfer:6px;display:flex;align-items:center;gap:12px;text-align:left;cursor:pointer;color:var(--text);
  background-color:var(--plate);border:1px solid var(--border-strong);padding:8px 12px;font-weight:600;
  transition:color .1s,border-color .1s,background-color .1s}
.net:hover{border-color:var(--accent);color:var(--accent)}
.net[aria-pressed=true]{border-color:var(--accent);background-color:var(--hover)}
.net-name{flex:1;min-width:0;overflow-wrap:anywhere}
.net-meta{font-family:var(--font-mono);font-size:.74rem;font-weight:500;color:var(--muted);white-space:nowrap}
.fields{display:grid;grid-template-columns:1fr 1fr;gap:12px 16px;margin:0 0 16px}
@media (max-width:575px){.fields{grid-template-columns:1fr}}
.field-input{display:flex}
input.text{width:100%;padding:8px 10px}
/* A field with its button after it is one plate: the field has the bottom-left corner, the button the top-right */
.field-input .btn{--chamfer:6px;flex:none;min-height:0;padding:4px 12px;font-size:.8rem;margin-left:-1px}

.speed-value{font-family:var(--font-mono);font-weight:700;font-size:1.3rem;min-width:4ch;text-align:right;
  font-variant-numeric:tabular-nums}
.hint{color:var(--muted);font-size:.9rem;margin-top:8px}

/* Messages, and the app's alert strip for the warnings */
.message{font-family:var(--font-mono);font-size:.8rem;font-weight:500;text-align:center;min-height:1.4em;margin-top:14px}
.message.is-error{color:var(--alert-text)}
.message.is-ok{color:var(--ok)}
.callout{font-size:.9rem;line-height:1.5;color:var(--alert-text);background:var(--alert-bg);
  border-left:3px solid var(--red);padding:8px 12px;margin:0 0 16px}
.callout b{font-weight:700}
footer{text-align:center;color:var(--muted);font-family:var(--font-mono);font-size:.72rem;padding:8px 0 24px}

/* ---- Corners ----
   As in the app: the top-right and bottom-left corners of the plates are
   filled in at 45 degrees with --corner, in the plate's border colour. Laid
   as background images, so these plates set background-color, never the
   background shorthand. */
.btn,.chip,.card,.gauge,.toast,.tab,.net,.stepper button,input.text{
  background-image:linear-gradient(to bottom left,var(--corner,transparent) 50%,transparent calc(50% + .75px)),
    linear-gradient(to top right,var(--corner,transparent) 50%,transparent calc(50% + .75px));
  background-size:var(--chamfer) var(--chamfer);background-position:top right,bottom left;
  background-repeat:no-repeat;background-origin:border-box}
.btn,.chip,.card,.gauge{--corner:var(--frame)}
.btn:disabled{--corner:var(--border-strong)}
.chip.is-fault{--corner:var(--red)}
.toast{--corner:var(--hud-border)}
.toast.is-error{--corner:var(--red)}
.net,.stepper button,input.text{--chamfer:6px;--corner:var(--border-strong)}
.net:hover,.net[aria-pressed=true],.stepper button:hover:not(:disabled),input.text:focus{--corner:var(--accent)}
/* Solid plates show a light tint of their fill along the edge instead */
.btn-accent,.btn-accent:disabled{--corner:color-mix(in srgb,var(--accent) 50%,#fff)}
.btn-danger{--corner:color-mix(in srgb,var(--red) 55%,transparent)}
.btn-danger:hover:not(:disabled){--corner:color-mix(in srgb,var(--red) 45%,#fff)}
/* Joined plates keep only their outer corners: the tabs' first and last, the
   stepper's - and +, a field and the button after it */
.tab{--chamfer:6px;--corner:var(--border-strong);background-size:0 0}
.tab:first-child{background-size:0 0,var(--chamfer) var(--chamfer)}
.tab:last-child{background-size:var(--chamfer) var(--chamfer),0 0}
.tab[aria-selected=true]{--corner:color-mix(in srgb,var(--accent) 50%,#fff)}
.stepper button:first-child{background-size:0 0,var(--chamfer) var(--chamfer)}
.stepper button:last-child{background-size:var(--chamfer) var(--chamfer),0 0}
.field-input input.text{background-size:0 0,var(--chamfer) var(--chamfer)}
.field-input .btn{background-size:var(--chamfer) var(--chamfer),0 0}

/* Scrollbars: a thin line that widens to a grip under the pointer */
::-webkit-scrollbar{width:10px;height:10px;background:transparent}
::-webkit-scrollbar-track,::-webkit-scrollbar-corner{background:transparent}
::-webkit-scrollbar-thumb{min-height:24px;background-color:var(--border-strong);background-clip:padding-box;
  border:3.5px solid transparent;border-radius:10px}
::-webkit-scrollbar-thumb:hover{border-width:1px}
::-webkit-scrollbar-thumb:active{border-width:1px;background-color:var(--accent)}
@supports not selector(::-webkit-scrollbar){*{scrollbar-width:thin;scrollbar-color:var(--border-strong) transparent}}
</style>
</head>
<body class="mode-drive">
<header>
  <div class="brand">Hexapod</div>
  <div class="robot" id="robot"></div>
  <div class="tabs" role="tablist">
    <button class="tab" role="tab" id="tabDrive" aria-selected="true" onclick="showTab('drive')">Drive</button>
    <button class="tab" role="tab" id="tabCalibrate" aria-selected="false" onclick="showTab('calibrate')">Calibrate</button>
    <button class="tab" role="tab" id="tabWifi" aria-selected="false" onclick="showTab('wifi')">WiFi</button>
  </div>
  <div class="chip" id="chip">Idle</div>
  <button class="theme-btn" id="themeBtn" onclick="toggleTheme()"></button>
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

<main id="wifi" class="page" hidden>
  <section class="card">
    <h2>WiFi network</h2>
    <p class="lede">Join an existing WiFi network instead of the robot's own access point. At
      boot the robot tries the saved network first; if it cannot connect, it starts its access
      point as usual.</p>
    <dl class="status" id="wifiStatus"></dl>
    <div class="row">
      <button class="btn" id="scanBtn" onclick="scanWifi()">Scan for networks</button>
    </div>
    <div class="nets" id="nets"></div>
    <form class="fields" id="wifiForm" onsubmit="event.preventDefault(); saveWifi()">
      <div class="field">
        <label class="label" for="wifiSsid">Network name (SSID)</label>
        <input class="text" id="wifiSsid" maxlength="32" autocomplete="off" autocapitalize="none"
               spellcheck="false" oninput="updatePasswordHint()">
      </div>
      <div class="field">
        <label class="label" for="wifiPass">Password</label>
        <div class="field-input">
          <input class="text" id="wifiPass" type="password" maxlength="63" autocomplete="off">
          <button type="button" class="btn" id="showPassBtn" onclick="togglePassword()">Show</button>
        </div>
      </div>
    </form>
    <div class="callout" id="wifiCallout"><b>Save and reboot</b> restarts the robot to join the
      network. Find it there by its hostname or the address your router gives it. If it cannot
      join, it comes back as its own access point.</div>
    <div class="row">
      <button class="btn" id="forgetBtn" onclick="forgetWifi()" disabled>Forget network</button>
      <button class="btn btn-accent" id="saveWifiBtn" onclick="saveWifi()">Save and reboot</button>
    </div>
    <div class="message" id="wifiMessage"></div>
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
let robotLabel = ''; // Robot name for the header, from /robot_config
let wifi = null;     // Last /wifi_status reply
let wifiBusy = false; // Saved or forgotten: the robot is rebooting
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
  [['drive', 'tabDrive'], ['calibrate', 'tabCalibrate'], ['wifi', 'tabWifi']].forEach(([page, tab]) => {
    $(page).hidden = name !== page;
    $(tab).setAttribute('aria-selected', name === page);
  });
  if (name === 'wifi' && !wifiBusy) loadWifi();
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
// WiFi
// ---------------------------------------------------------------------------

// The header names the robot and the network it is on.
function updateHeader() {
  const ssid = wifi ? wifi.ssid : '';
  $('robot').textContent = robotLabel + (robotLabel && ssid ? ' · ' : '') + ssid;
  if (robotLabel) document.title = $('robot').textContent + ' — Hexapod';
}

function renderWifi() {
  const dl = $('wifiStatus');
  dl.innerHTML = '';
  const rows = [['Mode', wifi.mode === 'sta' ? 'Joined network' : 'Access point'],
                ['Network', wifi.ssid],
                ['Address', wifi.ip],
                ['Hostname', wifi.hostname + '.local']];
  if (wifi.mode === 'sta' && wifi.rssi !== undefined) rows.push(['Signal', wifi.rssi + ' dBm']);
  rows.push(['Saved', wifi.saved || 'None (access point only)']);
  rows.forEach(([key, value]) => {
    const dt = document.createElement('dt');
    const dd = document.createElement('dd');
    dt.textContent = key;
    dd.textContent = value;
    dl.append(dt, dd);
  });
  if (!$('wifiSsid').value && wifi.saved) $('wifiSsid').value = wifi.saved;
  $('forgetBtn').disabled = !wifi.saved;
  $('wifiCallout').innerHTML = '<b>Save and reboot</b> restarts the robot to join the network. Find it ' +
    'there at <b></b> or the address your router gives it. If it cannot join within ' +
    Math.round((wifi.timeout_ms || 10000) / 1000) + ' s, it comes back as access point <b></b> ' +
    'at 192.168.4.1.';
  const marks = $('wifiCallout').querySelectorAll('b');
  marks[1].textContent = 'http://' + wifi.hostname + '.local/';
  marks[2].textContent = wifi.ap_ssid;
  updatePasswordHint();
  updateHeader();
}

function loadWifi() {
  return request('/wifi_status').then(text => {
    wifi = JSON.parse(text);
    renderWifi();
  }).catch(e => say(e.message, 'error', 'wifiMessage'));
}

// A blank password keeps the saved one when re-saving the same network.
function updatePasswordHint() {
  const same = wifi && wifi.saved && $('wifiSsid').value === wifi.saved;
  $('wifiPass').placeholder = same ? 'Blank keeps the saved password' : 'Blank for an open network';
  document.querySelectorAll('.net').forEach(el =>
    el.setAttribute('aria-pressed', el.dataset.ssid === $('wifiSsid').value));
}

function togglePassword() {
  const show = $('wifiPass').type === 'password';
  $('wifiPass').type = show ? 'text' : 'password';
  $('showPassBtn').textContent = show ? 'Hide' : 'Show';
}

// Signal strength as a word, from the RSSI in dBm
function signalWord(rssi) {
  return rssi >= -55 ? 'Strong' : rssi >= -70 ? 'Good' : rssi >= -80 ? 'Weak' : 'Poor';
}

function scanWifi() {
  $('scanBtn').disabled = true;
  $('scanBtn').textContent = 'Scanning...';
  say('', '', 'wifiMessage');
  request('/wifi_scan').then(text => {
    const nets = JSON.parse(text);
    const list = $('nets');
    list.innerHTML = '';
    nets.forEach(net => {
      const btn = document.createElement('button');
      btn.className = 'net';
      btn.type = 'button';
      btn.dataset.ssid = net.ssid;
      const name = document.createElement('span');
      name.className = 'net-name';
      name.textContent = net.ssid;
      const meta = document.createElement('span');
      meta.className = 'net-meta';
      meta.textContent = (net.secure ? '🔒︎ ' : 'Open · ') + signalWord(net.rssi) + ' ' + net.rssi + ' dBm';
      btn.append(name, meta);
      btn.onclick = () => {
        $('wifiSsid').value = net.ssid;
        $('wifiPass').value = '';
        updatePasswordHint();
        if (net.secure) $('wifiPass').focus();
      };
      list.appendChild(btn);
    });
    updatePasswordHint();
    say(nets.length ? 'Pick a network, or type its name.' : 'No networks found.', nets.length ? 'ok' : 'error', 'wifiMessage');
  }).catch(e => say(e.message, 'error', 'wifiMessage')).finally(() => {
    $('scanBtn').disabled = wifiBusy;
    $('scanBtn').textContent = 'Scan for networks';
  });
}

// The robot reboots after a save or forget, so the page has nothing left to do.
function wifiRebooting(text) {
  wifiBusy = true;
  ['scanBtn', 'saveWifiBtn', 'forgetBtn', 'wifiSsid', 'wifiPass', 'showPassBtn'].forEach(id => $(id).disabled = true);
  say(text, 'ok', 'wifiMessage');
}

function saveWifi() {
  if (wifiBusy) return;
  const ssid = $('wifiSsid').value;
  const pass = $('wifiPass').value;
  const keepsSaved = wifi && ssid === wifi.saved;
  if (!ssid || ssid.length > 32) return say('Enter a network name of 1-32 characters.', 'error', 'wifiMessage');
  if (pass && (pass.length < 8 || pass.length > 63)) {
    return say('WiFi passwords are 8-63 characters.', 'error', 'wifiMessage');
  }
  if (!pass && !keepsSaved && !confirm(`Join "${ssid}" as an open network, with no password?`)) return;
  say('Saving...', '', 'wifiMessage');
  request('/wifi_config', { method: 'POST', body: new URLSearchParams({ ssid: ssid, password: pass }) })
    .then(text => wifiRebooting(text + ' Reconnect to that network, then open http://' +
      (wifi ? wifi.hostname : 'hexapod') + '.local/'))
    .catch(e => say(e.message, 'error', 'wifiMessage'));
}

function forgetWifi() {
  if (wifiBusy || !confirm('Forget the saved network and reboot as an access point?')) return;
  request('/wifi_forget', { method: 'POST' })
    .then(text => wifiRebooting(text + ' Join ' + (wifi ? wifi.ap_ssid : "the robot's WiFi") +
      ', then open http://192.168.4.1/'))
    .catch(e => say(e.message, 'error', 'wifiMessage'));
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
  robotLabel = (config.geometry && config.geometry.label) || config.name;
  updateHeader();
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
loadWifi();
</script>
</body>
</html>
)rawliteral";

#endif  // WEB_PAGE_H
