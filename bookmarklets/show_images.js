const imgs = [];
const re = /url\([ \t]*['"`]\x3f([^\)'"`]+)['"`]\x3f[ \t]*\)/;
const fnc = function(parent) {
  Array.from(parent.children).forEach(child => {
    if (child.tagName === 'IMG') {
      imgs.push(child.src);
    }
    const bg = child.style.backgroundImage;
    if (bg && bg.toLowerCase().includes("url(")) {
      const m = bg.match(re);
      if (m) {
        imgs.push(m[1]);
      } else {
        console.warn("Failed to extract image URL from:", bg);
      }
    }
    fnc(child);
  });
};
fnc(document.body);
document.body.innerHTML = "";
imgs.forEach(img => {
  const div = document.createElement("DIV");
  const p = document.createElement("P");
  const a = document.createElement("A");
  a.href = img;
  a.innerText = img;
  p.appendChild(a);
  div.appendChild(p);
  const el = new Image();
  el.src = img;
  div.appendChild(el);
  document.body.appendChild(div);
});
