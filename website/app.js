'use strict';

// ── Часы ──
function updateClock() {
  const now = new Date();
  const time = now.toLocaleTimeString('ru-RU', { hour: '2-digit', minute: '2-digit' });
  const date = now.toLocaleDateString('ru-RU', { day: '2-digit', month: '2-digit', year: 'numeric' });
  document.getElementById('clock').textContent = `${time}  ${date}`;

  const lockTime = now.toLocaleTimeString('ru-RU', { hour: '2-digit', minute: '2-digit' });
  const lockDate = now.toLocaleDateString('ru-RU', { weekday: 'long', day: 'numeric', month: 'long' });
  document.getElementById('lock-time').textContent = lockTime;
  document.getElementById('lock-date').textContent = lockDate;
}
updateClock();
setInterval(updateClock, 1000);

// ── Lockscreen ──
document.getElementById('lockscreen').addEventListener('click', () => {
  const ls = document.getElementById('lockscreen');
  ls.style.opacity = '0';
  setTimeout(() => ls.style.display = 'none', 500);
  showNotif('Welcome', 'KRON Desktop Environment started');
});

// ── Приложения ──
const apps = [
  { name: 'Files',       icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><path d="M3 7a2 2 0 012-2h4l2 2h8a2 2 0 012 2v9a2 2 0 01-2 2H5a2 2 0 01-2-2V7z"/></svg>`, action: () => openWindow('Files', filesContent()) },
  { name: 'Terminal',    icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><rect x="2" y="3" width="20" height="18" rx="2"/><path d="M8 9l3 3-3 3M13 15h3"/></svg>`, action: () => openWindow('Terminal', termContent()) },
  { name: 'Firefox',     icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><circle cx="12" cy="12" r="9"/><path d="M12 3a9 9 0 010 18M3 12h18M12 3c-2.5 2.5-4 5.5-4 9s1.5 6.5 4 9"/></svg>`, action: () => openWindow('Firefox', browserContent()) },
  { name: 'Settings',    icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 00.33 1.82l.06.06a2 2 0 010 2.83 2 2 0 01-2.83 0l-.06-.06a1.65 1.65 0 00-1.82-.33 1.65 1.65 0 00-1 1.51V21a2 2 0 01-4 0v-.09A1.65 1.65 0 009 19.4a1.65 1.65 0 00-1.82.33l-.06.06a2 2 0 01-2.83-2.83l.06-.06A1.65 1.65 0 004.68 15a1.65 1.65 0 00-1.51-1H3a2 2 0 010-4h.09A1.65 1.65 0 004.6 9a1.65 1.65 0 00-.33-1.82l-.06-.06a2 2 0 012.83-2.83l.06.06A1.65 1.65 0 009 4.68a1.65 1.65 0 001-1.51V3a2 2 0 014 0v.09a1.65 1.65 0 001 1.51 1.65 1.65 0 001.82-.33l.06-.06a2 2 0 012.83 2.83l-.06.06A1.65 1.65 0 0019.4 9a1.65 1.65 0 001.51 1H21a2 2 0 010 4h-.09a1.65 1.65 0 00-1.51 1z"/></svg>`, action: () => openWindow('Settings', settingsContent()) },
  { name: 'Text Editor', icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><path d="M14 2H6a2 2 0 00-2 2v16a2 2 0 002 2h12a2 2 0 002-2V8z"/><polyline points="14 2 14 8 20 8"/><line x1="16" y1="13" x2="8" y2="13"/><line x1="16" y1="17" x2="8" y2="17"/><polyline points="10 9 9 9 8 9"/></svg>`, action: () => openWindow('Text Editor', editorContent()) },
  { name: 'Calculator',  icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><rect x="4" y="2" width="16" height="20" rx="2"/><line x1="8" y1="6" x2="16" y2="6"/><line x1="8" y1="10" x2="8" y2="10"/><line x1="12" y1="10" x2="12" y2="10"/><line x1="16" y1="10" x2="16" y2="10"/><line x1="8" y1="14" x2="8" y2="14"/><line x1="12" y1="14" x2="12" y2="14"/><line x1="16" y1="14" x2="16" y2="14"/><line x1="8" y1="18" x2="12" y2="18"/><line x1="16" y1="18" x2="16" y2="18"/></svg>`, action: () => openWindow('Calculator', calcContent()) },
  { name: 'Music',       icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><path d="M9 18V5l12-2v13"/><circle cx="6" cy="18" r="3"/><circle cx="18" cy="16" r="3"/></svg>`, action: () => openWindow('Music Player', musicContent()) },
  { name: 'Photos',      icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><rect x="3" y="3" width="18" height="18" rx="2"/><circle cx="8.5" cy="8.5" r="1.5"/><polyline points="21 15 16 10 5 21"/></svg>`, action: () => openWindow('Photos', '<p style="color:rgba(255,255,255,.5);padding:20px">No photos found.</p>') },
  { name: 'Calendar',    icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><rect x="3" y="4" width="18" height="18" rx="2"/><line x1="16" y1="2" x2="16" y2="6"/><line x1="8" y1="2" x2="8" y2="6"/><line x1="3" y1="10" x2="21" y2="10"/></svg>`, action: () => openWindow('Calendar', calendarContent()) },
  { name: 'System Info', icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><rect x="2" y="3" width="20" height="14" rx="2"/><line x1="8" y1="21" x2="16" y2="21"/><line x1="12" y1="17" x2="12" y2="21"/></svg>`, action: () => openWindow('System Info', sysInfoContent()) },
  { name: 'Archive',     icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><polyline points="21 8 21 21 3 21 3 8"/><rect x="1" y="3" width="22" height="5"/><line x1="10" y1="12" x2="14" y2="12"/></svg>`, action: () => openWindow('Archive Manager', '<p style="color:rgba(255,255,255,.5);padding:20px">No archives open.</p>') },
  { name: 'About KRON',  icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><polygon points="12 2 15.09 8.26 22 9.27 17 14.14 18.18 21.02 12 17.77 5.82 21.02 7 14.14 2 9.27 8.91 8.26 12 2"/></svg>`, action: () => openWindow('About KRON', aboutContent()) },
];

function renderApps(list) {
  const grid = document.getElementById('app-grid');
  grid.innerHTML = '';
  list.forEach(app => {
    const btn = document.createElement('button');
    btn.className = 'app-btn';
    btn.innerHTML = `<span class="app-icon">${app.icon}</span><span class="app-name">${app.name}</span>`;
    btn.onclick = () => { app.action(); toggleLauncher(); };
    grid.appendChild(btn);
  });
}
renderApps(apps);

function filterApps(q) {
  renderApps(apps.filter(a => a.name.toLowerCase().includes(q.toLowerCase())));
}

// ── Dock ──
const dockApps = [
  { name: 'Files',       icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><path d="M3 7a2 2 0 012-2h4l2 2h8a2 2 0 012 2v9a2 2 0 01-2 2H5a2 2 0 01-2-2V7z"/></svg>`, action: () => openWindow('Files', filesContent()) },
  { name: 'Terminal',    icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><rect x="2" y="3" width="20" height="18" rx="2"/><path d="M8 9l3 3-3 3M13 15h3"/></svg>`, action: () => openWindow('Terminal', termContent()) },
  { name: 'Firefox',     icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><circle cx="12" cy="12" r="9"/><path d="M12 3a9 9 0 010 18M3 12h18M12 3c-2.5 2.5-4 5.5-4 9s1.5 6.5 4 9"/></svg>`, action: () => openWindow('Firefox', browserContent()) },
  { name: 'Settings',    icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 00.33 1.82l.06.06a2 2 0 010 2.83 2 2 0 01-2.83 0l-.06-.06a1.65 1.65 0 00-1.82-.33 1.65 1.65 0 00-1 1.51V21a2 2 0 01-4 0v-.09A1.65 1.65 0 009 19.4a1.65 1.65 0 00-1.82.33l-.06.06a2 2 0 01-2.83-2.83l.06-.06A1.65 1.65 0 004.68 15a1.65 1.65 0 00-1.51-1H3a2 2 0 010-4h.09A1.65 1.65 0 004.6 9a1.65 1.65 0 00-.33-1.82l-.06-.06a2 2 0 012.83-2.83l.06.06A1.65 1.65 0 009 4.68a1.65 1.65 0 001-1.51V3a2 2 0 014 0v.09a1.65 1.65 0 001 1.51 1.65 1.65 0 001.82-.33l.06-.06a2 2 0 012.83 2.83l-.06.06A1.65 1.65 0 0019.4 9a1.65 1.65 0 001.51 1H21a2 2 0 010 4h-.09a1.65 1.65 0 00-1.51 1z"/></svg>`, action: () => openWindow('Settings', settingsContent()) },
  { name: 'Text Editor', icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><path d="M14 2H6a2 2 0 00-2 2v16a2 2 0 002 2h12a2 2 0 002-2V8z"/><polyline points="14 2 14 8 20 8"/><line x1="16" y1="13" x2="8" y2="13"/><line x1="16" y1="17" x2="8" y2="17"/></svg>`, action: () => openWindow('Text Editor', editorContent()) },
  { name: 'Calculator',  icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><rect x="4" y="2" width="16" height="20" rx="2"/><line x1="8" y1="6" x2="16" y2="6"/><line x1="8" y1="10" x2="8" y2="10"/><line x1="12" y1="10" x2="12" y2="10"/><line x1="16" y1="10" x2="16" y2="10"/><line x1="8" y1="14" x2="8" y2="14"/><line x1="12" y1="14" x2="12" y2="14"/><line x1="16" y1="14" x2="16" y2="14"/><line x1="8" y1="18" x2="12" y2="18"/></svg>`, action: () => openWindow('Calculator', calcContent()) },
  { name: 'Music',       icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><path d="M9 18V5l12-2v13"/><circle cx="6" cy="18" r="3"/><circle cx="18" cy="16" r="3"/></svg>`, action: () => openWindow('Music Player', musicContent()) },
  { name: 'About KRON',  icon: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><polygon points="12 2 15.09 8.26 22 9.27 17 14.14 18.18 21.02 12 17.77 5.82 21.02 7 14.14 2 9.27 8.91 8.26 12 2"/></svg>`, action: () => openWindow('About KRON', aboutContent()) },
];

function renderDock() {
  const dock = document.getElementById('dock');
  dockApps.forEach(app => {
    const btn = document.createElement('button');
    btn.className = 'dock-item';
    btn.dataset.name = app.name;
    btn.innerHTML = `${app.icon}<span class="dock-label">${app.name}</span><span class="dock-dot"></span>`;
    btn.onclick = () => { app.action(); setTimeout(updateDockDots, 100); };
    dock.appendChild(btn);
  });
}
renderDock();

function updateDockDots() {
  dockApps.forEach(app => {
    const btn = document.querySelector(`.dock-item[data-name="${app.name}"]`);
    if (!btn) return;
    const isOpen = Array.from(document.querySelectorAll('.window-title'))
      .some(t => t.textContent === app.name);
    btn.classList.toggle('running', isOpen);
  });
}

// ── Лаунчер ──
function toggleLauncher() {
  const l = document.getElementById('launcher');
  if (!l.classList.contains('hidden')) {
    l.classList.add('hiding');
    setTimeout(() => { l.classList.add('hidden'); l.classList.remove('hiding'); }, 200);
  } else {
    l.classList.remove('hidden');
    l.classList.add('showing');
    setTimeout(() => l.classList.remove('showing'), 350);
    document.getElementById('search').value = '';
    renderApps(apps);
    setTimeout(() => document.getElementById('search').focus(), 50);
  }
}

document.addEventListener('keydown', e => {
  if (e.key === 'Escape') {
    document.getElementById('launcher').classList.add('hidden');
    document.getElementById('power-menu').classList.add('hidden');
  }
});

// ── Power menu ──
function togglePower() {
  document.getElementById('power-menu').classList.toggle('hidden');
}

// ── Окна ──
let zCounter = 10;
let windowCount = 0;
const openWindows = {};

function openWindow(title, content) {
  windowCount++;
  const id = 'win-' + windowCount;
  const win = document.createElement('div');
  win.className = 'window focused';
  win.id = id;
  win.style.cssText = `left:${60 + (windowCount % 4) * 25}px; top:${46 + (windowCount % 4) * 18}px; width:${title === 'Firefox' ? '800' : '520'}px; height:${title === 'Firefox' ? '560' : '360'}px; z-index:${++zCounter}`;

  win.innerHTML = `
    <div class="window-titlebar">
      <button class="win-btn win-close"  onclick="closeWindow('${id}')"></button>
      <button class="win-btn win-min"    onclick="minimizeWindow('${id}')"></button>
      <button class="win-btn win-max"    onclick="maximizeWindow('${id}')"></button>
      <span class="window-title">${title}</span>
    </div>
    <div class="window-content">${content}</div>
  `;

  document.getElementById('windows-container').appendChild(win);
  makeDraggable(win);
  focusWindow(win);
  addTaskbarItem(id, title);
  openWindows[id] = { title, minimized: false };
}

function closeWindow(id) {
  const win = document.getElementById(id);
  if (!win) return;
  win.classList.add('closing');
  setTimeout(() => {
    win.remove();
    removeTaskbarItem(id);
    delete openWindows[id];
    setTimeout(updateDockDots, 100);
  }, 200);
}

function minimizeWindow(id) {
  const win = document.getElementById(id);
  if (!win) return;
  win.classList.add('minimizing');
  setTimeout(() => {
    win.style.display = 'none';
    win.classList.remove('minimizing');
    openWindows[id].minimized = true;
  }, 250);
  const item = document.querySelector(`[data-win="${id}"]`);
  if (item) item.classList.remove('active');
}

function maximizeWindow(id) {
  const win = document.getElementById(id);
  if (!win) return;
  if (win.dataset.maximized) {
    win.style.cssText = win.dataset.prev;
    delete win.dataset.maximized;
  } else {
    win.dataset.prev = win.style.cssText;
    win.style.cssText = `left:0; top:36px; width:100vw; height:calc(100vh - 36px); z-index:${++zCounter}`;
    win.dataset.maximized = '1';
  }
}

function focusWindow(win) {
  document.querySelectorAll('.window').forEach(w => w.classList.remove('focused'));
  win.classList.add('focused');
  win.style.zIndex = ++zCounter;
  const id = win.id;
  document.querySelectorAll('.taskbar-item').forEach(i => i.classList.remove('active'));
  const item = document.querySelector(`[data-win="${id}"]`);
  if (item) item.classList.add('active');
}

// ── Taskbar ──
function addTaskbarItem(id, title) {
  const item = document.createElement('button');
  item.className = 'taskbar-item active';
  item.dataset.win = id;
  item.textContent = title;
  item.onclick = () => {
    const win = document.getElementById(id);
    if (!win) return;
    if (openWindows[id].minimized) {
      win.style.display = 'flex';
      openWindows[id].minimized = false;
    }
    focusWindow(win);
  };
  document.getElementById('taskbar').appendChild(item);
}

function removeTaskbarItem(id) {
  const item = document.querySelector(`[data-win="${id}"]`);
  if (item) item.remove();
}

// ── Drag ──
function makeDraggable(win) {
  const bar = win.querySelector('.window-titlebar');
  let ox, oy, dragging = false;

  bar.addEventListener('mousedown', e => {
    if (e.target.classList.contains('win-btn')) return;
    dragging = true;
    ox = e.clientX - win.offsetLeft;
    oy = e.clientY - win.offsetTop;
    focusWindow(win);
    e.preventDefault();
  });

  document.addEventListener('mousemove', e => {
    if (!dragging) return;
    win.style.left = (e.clientX - ox) + 'px';
    win.style.top  = Math.max(36, e.clientY - oy) + 'px';
  });

  document.addEventListener('mouseup', () => dragging = false);
  win.addEventListener('mousedown', () => focusWindow(win));
}

// ── Уведомления ──
function showNotif(title, body) {
  let stack = document.getElementById('notif-stack');
  if (!stack) {
    stack = document.createElement('div');
    stack.id = 'notif-stack';
    document.getElementById('desktop').appendChild(stack);
  }
  const n = document.createElement('div');
  n.className = 'notif';
  n.innerHTML = `<div class="notif-title">${title}</div><div class="notif-body">${body}</div>`;
  stack.appendChild(n);
  setTimeout(() => {
    n.classList.add('hiding');
    setTimeout(() => n.remove(), 250);
  }, 4000);
}

// ── Контент окон ──
function filesContent() {
  return `<div style="display:grid;grid-template-columns:repeat(4,1fr);gap:12px">
    ${['Documents','Downloads','Pictures','Music','Videos','Desktop'].map(f =>
      `<div style="text-align:center;cursor:pointer;padding:8px;border-radius:6px" onmouseover="this.style.background='#313244'" onmouseout="this.style.background=''">
        <div style="font-size:32px">📁</div><div style="font-size:12px;margin-top:4px">${f}</div>
      </div>`).join('')}
  </div>`;
}

function termContent() {
  return `<div style="background:#11111b;padding:12px;border-radius:6px;font-family:monospace;font-size:13px;height:100%;color:#cdd6f4">
    <div style="color:#a6e3a1">user@kron</div>
    <div style="color:#89b4fa">~$</div>
    <div style="margin-top:8px;color:#cdd6f4">Welcome to KRON Terminal</div>
    <div style="color:#6c7086;margin-top:4px">Type commands here (demo only)</div>
    <div style="margin-top:12px"><span style="color:#a6e3a1">user@kron</span> <span style="color:#89b4fa">~$</span> <span id="term-cursor">▋</span></div>
  </div>`;
}

function browserContent() {
  const sites = [
    { name: 'GitHub', url: 'https://github.com/MelnikPro/kron-os-1.0.0' },
    { name: 'Google', url: 'https://www.google.com' },
    { name: 'Wikipedia', url: 'https://en.m.wikipedia.org/wiki/Wayland_(protocol)' },
  ];

  const tabs = sites.map((s, i) =>
    `<button onclick="switchTab(${i})" id="tab-${i}" style="background:${i===0?'rgba(255,255,255,0.12)':'transparent'};color:rgba(255,255,255,0.8);border:none;padding:4px 12px;border-radius:6px 6px 0 0;cursor:pointer;font-size:11px;white-space:nowrap">${s.name}</button>`
  ).join('');

  const iframes = sites.map((s, i) =>
    `<iframe id="iframe-${i}" src="${s.url}" style="width:100%;height:100%;border:none;display:${i===0?'block':'none'};border-radius:0 0 6px 6px" sandbox="allow-scripts allow-same-origin allow-forms allow-popups"></iframe>`
  ).join('');

  return `<div style="display:flex;flex-direction:column;height:100%;gap:0">
    <div style="display:flex;gap:2px;align-items:center;padding:6px 8px;background:rgba(0,0,0,0.2);border-radius:8px 8px 0 0;flex-wrap:wrap">
      <button onclick="iframeBack()" style="background:rgba(255,255,255,0.08);border:none;color:rgba(255,255,255,0.7);padding:3px 8px;border-radius:4px;cursor:pointer">←</button>
      <button onclick="iframeForward()" style="background:rgba(255,255,255,0.08);border:none;color:rgba(255,255,255,0.7);padding:3px 8px;border-radius:4px;cursor:pointer">→</button>
      <button onclick="iframeReload()" style="background:rgba(255,255,255,0.08);border:none;color:rgba(255,255,255,0.7);padding:3px 8px;border-radius:4px;cursor:pointer">↺</button>
      <input id="browser-url" value="${sites[0].url}" onkeydown="if(event.key==='Enter')iframeGo(this.value)"
        style="flex:1;min-width:120px;background:rgba(255,255,255,0.08);color:rgba(255,255,255,0.9);border:1px solid rgba(255,255,255,0.15);padding:3px 10px;border-radius:6px;font-size:12px;outline:none">
      <button onclick="iframeGo(document.getElementById('browser-url').value)" style="background:#5294E2;border:none;color:white;padding:3px 10px;border-radius:6px;cursor:pointer;font-size:12px">Go</button>
    </div>
    <div style="display:flex;gap:2px;padding:4px 8px 0;background:rgba(0,0,0,0.15)">${tabs}</div>
    <div id="browser-frames" style="flex:1;position:relative;background:#fff;border-radius:0 0 8px 8px;overflow:hidden">
      ${iframes}
      <div id="browser-blocked" style="display:none;position:absolute;inset:0;background:rgba(20,20,35,0.95);display:flex;flex-direction:column;align-items:center;justify-content:center;color:rgba(255,255,255,0.7);font-size:13px;gap:8px">
        <div style="font-size:32px">🚫</div>
        <div>This site blocks embedding</div>
        <div style="font-size:11px;color:rgba(255,255,255,0.4)">Most sites block iframes for security reasons</div>
      </div>
    </div>
  </div>`;
}

let currentTab = 0;
const browserSites = [
  'https://github.com/MelnikPro/kron-os-1.0.0',
  'https://www.google.com',
  'https://en.m.wikipedia.org/wiki/Wayland_(protocol)',
];

function switchTab(i) {
  document.getElementById('iframe-' + currentTab).style.display = 'none';
  document.getElementById('tab-' + currentTab).style.background = 'transparent';
  currentTab = i;
  document.getElementById('iframe-' + i).style.display = 'block';
  document.getElementById('tab-' + i).style.background = 'rgba(255,255,255,0.12)';
  document.getElementById('browser-url').value = browserSites[i];
}

function iframeGo(url) {
  if (!url.startsWith('http')) url = 'https://' + url;
  const iframe = document.getElementById('iframe-' + currentTab);
  iframe.src = url;
  document.getElementById('browser-url').value = url;
  browserSites[currentTab] = url;
}

function iframeBack()    { document.getElementById('iframe-' + currentTab).contentWindow.history.back(); }
function iframeForward() { document.getElementById('iframe-' + currentTab).contentWindow.history.forward(); }
function iframeReload()  { document.getElementById('iframe-' + currentTab).contentWindow.location.reload(); }

function settingsContent() {
  return `<div style="display:flex;flex-direction:column;gap:16px">
    <div style="display:flex;justify-content:space-between;align-items:center">
      <span>Theme</span>
      <select style="background:#313244;color:#cdd6f4;border:1px solid #45475a;padding:4px 8px;border-radius:4px">
        <option>Dark</option><option>Light</option>
      </select>
    </div>
    <div style="display:flex;justify-content:space-between;align-items:center">
      <span>Accent Color</span>
      <input type="color" value="#5294E2" style="border:none;background:none;cursor:pointer;width:32px;height:32px">
    </div>
    <div style="display:flex;justify-content:space-between;align-items:center">
      <span>Panel Height</span>
      <input type="range" min="28" max="48" value="36" style="width:120px">
    </div>
  </div>`;
}

function editorContent() {
  return `<textarea style="width:100%;height:100%;background:#11111b;color:#cdd6f4;border:none;outline:none;font-family:monospace;font-size:13px;resize:none;padding:8px" placeholder="Start typing..."></textarea>`;
}

function calcContent() {
  const btns = ['7','8','9','÷','4','5','6','×','1','2','3','-','0','.','=','+'];
  return `<div style="display:flex;flex-direction:column;gap:8px">
    <input id="calc-display" readonly style="background:#11111b;color:#cdd6f4;border:1px solid #45475a;padding:8px;border-radius:4px;font-size:18px;text-align:right;width:100%" value="0">
    <div style="display:grid;grid-template-columns:repeat(4,1fr);gap:6px">
      ${btns.map(b => `<button onclick="calcPress('${b}')" style="background:#313244;color:#cdd6f4;border:none;padding:12px;border-radius:4px;cursor:pointer;font-size:15px">${b}</button>`).join('')}
    </div>
  </div>`;
}

let calcVal = '0';
function calcPress(b) {
  const d = document.getElementById('calc-display');
  if (!d) return;
  if (b === '=') { try { d.value = eval(calcVal.replace('÷','/').replace('×','*')); calcVal = d.value; } catch(e) { d.value = 'Error'; calcVal = '0'; } }
  else { calcVal = calcVal === '0' ? b : calcVal + b; d.value = calcVal; }
}

function musicContent() {
  return `<div style="text-align:center;padding:20px">
    <div style="font-size:64px;margin-bottom:16px">🎵</div>
    <div style="font-size:16px;margin-bottom:4px">No music playing</div>
    <div style="color:#6c7086;font-size:13px;margin-bottom:24px">Open a music file to start</div>
    <div style="display:flex;justify-content:center;gap:16px">
      <button style="background:#313244;border:none;color:#cdd6f4;padding:8px 16px;border-radius:4px;cursor:pointer;font-size:18px">⏮</button>
      <button style="background:#5294E2;border:none;color:white;padding:8px 20px;border-radius:4px;cursor:pointer;font-size:18px">▶</button>
      <button style="background:#313244;border:none;color:#cdd6f4;padding:8px 16px;border-radius:4px;cursor:pointer;font-size:18px">⏭</button>
    </div>
  </div>`;
}

function calendarContent() {
  const now = new Date();
  const month = now.toLocaleDateString('en-US', { month: 'long', year: 'numeric' });
  return `<div style="text-align:center">
    <div style="font-size:16px;margin-bottom:12px;font-weight:bold">${month}</div>
    <div style="display:grid;grid-template-columns:repeat(7,1fr);gap:4px;font-size:12px">
      ${['Su','Mo','Tu','We','Th','Fr','Sa'].map(d => `<div style="color:#6c7086;padding:4px">${d}</div>`).join('')}
      ${Array.from({length:35},(_,i)=>{
        const d = i - new Date(now.getFullYear(), now.getMonth(), 1).getDay() + 1;
        const isToday = d === now.getDate();
        return `<div style="padding:4px;border-radius:4px;${isToday?'background:#5294E2;color:white':'color:#cdd6f4'}">${d > 0 && d <= 31 ? d : ''}</div>`;
      }).join('')}
    </div>
  </div>`;
}

function sysInfoContent() {
  return `<div style="font-family:monospace;font-size:13px;display:flex;flex-direction:column;gap:8px">
    <div style="color:#89b4fa;font-size:20px;margin-bottom:8px">⬡ KRON</div>
    ${[
      ['OS', 'Arch Linux'],
      ['DE', 'KRON 1.0.0'],
      ['WM', 'kron (wlroots)'],
      ['Shell', 'kron-shell (GTK4)'],
      ['Display', 'Wayland'],
      ['Resolution', window.screen.width + 'x' + window.screen.height],
      ['Browser', navigator.userAgent.split(' ').pop()],
    ].map(([k,v]) => `<div><span style="color:#6c7086">${k}:</span> <span style="color:#cdd6f4">${v}</span></div>`).join('')}
  </div>`;
}

function aboutContent() {
  return `<div style="text-align:center;padding:20px">
    <div style="font-size:48px;margin-bottom:12px">⬡</div>
    <div style="font-size:22px;font-weight:bold;margin-bottom:4px">KRON</div>
    <div style="color:#6c7086;margin-bottom:16px">Desktop Environment v1.0.0</div>
    <div style="font-size:13px;color:#a6adc8;line-height:1.6">
      A minimal Wayland desktop environment<br>
      built with wlroots + GTK4<br><br>
      <a href="https://github.com/MelnikPro/kron-os-1.0.0" style="color:#5294E2">github.com/MelnikPro/kron-os-1.0.0</a>
    </div>
    <div style="margin-top:16px;font-size:12px;color:#6c7086">MIT License © 2026 MelnikPro</div>
  </div>`;
}

// Открываем About при старте
setTimeout(() => {
  openWindow('About KRON', aboutContent());
  showNotif('KRON', 'Welcome to KRON Desktop Environment');
}, 600);
