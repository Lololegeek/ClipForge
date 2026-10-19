(() => {
  const $ = (s, r = document) => r.querySelector(s);
  const reduce = matchMedia('(prefers-reduced-motion: reduce)').matches;

  /* ---------- Hero : règle de replay interactive ---------- */
  const TOTAL = 120, SAVE = 30;
  const fill = $('#rFill'), save = $('#rSave'), time = $('#rTime'), clips = $('#rClips'), key = $('#rKey'), ticks = $('#rTicks');
  for (let i = 0; i <= 60; i++) { const t = document.createElement('i'); if (i % 5 === 0) t.className = 'm'; ticks.appendChild(t); }
  let buf = reduce ? 84 : 6, n = 0, saveTimer;
  const empty = document.createElement('li');
  empty.className = 'ruler-empty'; empty.textContent = 'Aucun clip pour l’instant.';
  clips.appendChild(empty);

  function paint() {
    fill.style.width = (buf / TOTAL * 100) + '%';
    time.textContent = Math.floor(buf);
  }
  function doSave() {
    if (buf < 4) return;
    const len = Math.min(SAVE, buf);
    save.style.left = ((buf - len) / TOTAL * 100) + '%';
    save.style.width = (len / TOTAL * 100) + '%';
    save.classList.add('on');
    clearTimeout(saveTimer);
    saveTimer = setTimeout(() => save.classList.remove('on'), 1600);
    key.classList.add('press'); setTimeout(() => key.classList.remove('press'), 140);
    empty.remove(); n++;
    const li = document.createElement('li');
    li.innerHTML = `<span class="ok">●</span><span>replay_${String(n).padStart(2, '0')}.mp4</span><span class="mono">${Math.round(len)} s</span>`;
    clips.prepend(li);
    while (clips.children.length > 3) clips.lastChild.remove();
  }
  paint();
  if (!reduce) {
    let last = performance.now();
    (function tick(now) {
      buf = Math.min(TOTAL, buf + (now - last) / 1000 * 4);
      last = now; paint();
      requestAnimationFrame(tick);
    })(last);
    setTimeout(doSave, 6500);
  }
  key.addEventListener('click', doSave);
  addEventListener('keydown', e => {
    if (e.key === 'F8' && !e.ctrlKey && !e.altKey && !e.metaKey) { e.preventDefault(); doSave(); }
  });

  /* ---------- Pages de l'application ---------- */
  const ruler = (pct) => `<div class="mini-ruler"><div class="t"></div><div class="f" style="width:${pct}%"></div><div class="ticks">${Array.from({length: 61}, (_, i) => `<i class="${i % 5 ? '' : 'm5'}"></i>`).join('')}</div></div>`;
  const card = (cls, d, name, sub) => `<div class="card"><div class="th ${cls}"><span class="du">${d}</span></div><b>${name}</b><small>${sub}</small></div>`;

  const pages = {
    home: () => `
      <h3 class="pg-title">Garde le meilleur.</h3>
      <p class="pg-sub mut">Le jeu continue. Le moment reste.</p>
      <div class="panel">
        <div class="row"><h4>Instant Replay</h4><span class="sp"></span><span class="m acc" style="font-size:16px">84 / 120 s</span></div>
        ${ruler(70)}
        <div class="row"><span class="mut" style="font-size:12px;line-height:1.6;flex:1">DXGI · écran principal<br>AMD AMF H.264</span><span class="abtn p">Arrêter</span></div>
      </div>
      <div class="stats"><div><b>24</b><small>Clips</small></div><div><b>412 Mo</b><small>RAM</small></div><div><b>60.0 FPS</b><small>Capture</small></div><div><b>212.4 Go</b><small>Stockage libre</small></div></div>
      <div class="row"><h4 class="h-sec">Derniers moments</h4><span class="sp"></span><span class="abtn">Voir mes clips</span></div>
      <div class="list">
        <div class="it"><b>replay_2026-10-18_21-42</b><span>Arena</span><span class="m">30 s &nbsp;→</span></div>
        <div class="it"><b>replay_2026-10-18_21-17</b><span>Arena</span><span class="m">30 s &nbsp;→</span></div>
        <div class="it"><b>session_2026-10-17_20-03</b><span>Route 66</span><span class="m">1412 s &nbsp;→</span></div>
      </div>`,
    clips: () => `
      <div class="row"><h3 class="pg-title">Mes clips</h3><span class="m mut">24</span><span class="sp"></span><span class="abtn">Importer</span><span class="abtn">Actualiser</span></div>
      <div class="tools"><span class="chip">Rechercher un clip…</span><span class="chip">Date</span><span class="chip">★ Favoris</span><span class="sp"></span><span class="chip on">Grille</span></div>
      <div class="grid">
        ${card('a', '0:30', '★  Clutch 1v3', 'Arena')}
        ${card('b', '0:30', 'replay_21-17', 'Route 66')}
        ${card('c', '0:45', 'Fin de partie', 'Arena')}
        ${card('d', '0:30', 'replay_20-52', 'Vidéo locale')}
        ${card('e', '1:12', '★  Course finale', 'Route 66')}
        ${card('f', '0:30', 'replay_19-08', 'Vidéo locale')}
      </div>`,
    editor: () => `
      <div class="row"><h3 class="pg-title">Éditeur</h3><span class="sp"></span><span class="abtn">Ouvrir un projet</span><span class="abtn">Enregistrer le projet</span></div>
      <div class="mut" style="margin-top:-8px">Clutch 1v3</div>
      <div class="vid"><div class="pl"><svg viewBox="0 0 24 24"><use href="#i-play"/></svg></div></div>
      <div class="row"><span class="abtn">Pause</span><span class="m mut">10.8 / 30.0 s</span><div class="scrub" style="flex:1"><i></i><u></u></div><span class="chip">1×</span></div>
      <div class="trim"><div class="sel"></div></div>
      <div class="row"><span class="abtn">Entrée ici</span><span class="m acc">7.20 s</span><span class="abtn">Sortie ici</span><span class="m acc">18.60 s</span><span class="sp"></span><span class="abtn">Ajouter</span><span class="abtn p">Garder ce passage</span></div>
      <div class="row"><span class="mut" style="font-size:12px;flex:1">Montage non destructif · export précis avec réencodage · la vidéo originale reste intacte.</span><span class="abtn p">Exporter en MP4</span></div>`,
    record: () => `
      <h3 class="pg-title">Enregistrer</h3>
      <p class="pg-sub mut">Le replay garde les dernières secondes. L'enregistrement conserve toute la session, jusqu'à son arrêt.</p>
      <div class="panel">
        <div style="font-size:18px;font-weight:600">DXGI · écran principal</div>
        <div class="mut" style="margin:4px 0 6px">AMD AMF H.264</div>
        <div class="m mut" style="font-size:13px">1920 × 1080 · 60 FPS · H264 · 120 secondes</div>
        <div class="row" style="margin-top:22px"><span class="abtn">Arrêter la capture</span><span class="abtn p">Enregistrer · F9</span><span class="abtn">Capture écran · F10</span></div>
      </div>
      <div class="metrics">
        <div><small>FPS effectifs</small><b>59.97</b></div><div><small>Images en retard</small><b>0</b></div><div><small>Rendu moyen</small><b>0.84 ms</b></div>
        <div><small>Dernière sauvegarde</small><b>186 ms</b></div><div><small>Mémoire du processus</small><b>412 Mo</b></div><div><small>CPU (tous les cœurs)</small><b>2.3 %</b></div>
      </div>
      <p class="mut" style="font-size:12px;margin:0;line-height:1.6">La charge GPU et la taille exacte des paquets du tampon ne sont pas exposées ici. La durée affichée indique le temps écoulé, plafonné au réglage.</p>`
  };

  const page = $('#page'), nav = $('#sideNav'), tabs = [...document.querySelectorAll('.tabs [role=tab]')];
  function show(p, animate = true) {
    page.innerHTML = pages[p]();
    if (animate && !reduce) { page.classList.remove('swap'); void page.offsetWidth; page.classList.add('swap'); }
    tabs.forEach(t => t.setAttribute('aria-selected', t.dataset.p === p));
    nav.querySelectorAll('li').forEach(li => li.classList.toggle('on', li.dataset.p === p));
  }
  tabs.forEach(t => t.addEventListener('click', () => show(t.dataset.p)));
  nav.querySelectorAll('li[data-p]').forEach(li => li.addEventListener('click', () => show(li.dataset.p)));
  show('home', false);

  /* mise à l'échelle de la fenêtre (conçue à 1180×740) */
  const stage = $('#stage'), win = $('#win'), W = 1180, H = 740;
  function fit() {
    const s = stage.clientWidth / W;
    win.style.transform = `scale(${s})`;
    stage.style.height = (H * s) + 'px';
  }
  new ResizeObserver(fit).observe(stage); fit();

  /* copier les commandes */
  const copy = $('#copy');
  copy.addEventListener('click', async () => {
    const label = $('span', copy);
    try { await navigator.clipboard.writeText($('#code').innerText); label.textContent = 'Copié'; }
    catch { label.textContent = 'Sélectionne le texte'; }
    setTimeout(() => label.textContent = 'Copier', 1800);
  });
})();
