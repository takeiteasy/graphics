var stats = undefined, Module = undefined;

function toggle_class(el, className) {
  var reg = new RegExp('(\\s|^)' + className + '(\\s|$)');
  if (el.classList) {
    if (el.classList.contains(className))
      el.classList.remove(className)
    else
      el.classList.add(className)
  } else if (el.className.match(reg))
    el.className = el.className.replace(reg, ' ');
  else
    el.className += " " + className;
}

document.addEventListener("DOMContentLoaded", function(e) {
  var statusElement = document.getElementById('status');
  var progressElement = document.getElementById('progress');

  Module = {
    preRun: [],
    postRun: [],
    print: (function() {
      return function(text) {
        if (arguments.length > 1)
          text = Array.prototype.slice.call(arguments).join(' ');
        console.log(text);
      };
    })(),
    printErr: function(text) {
      if (arguments.length > 1)
        text = Array.prototype.slice.call(arguments).join(' ');
      console.error(text);
    },
    canvas: (function() {
      var canvas = document.getElementById('canvas');
      canvas.addEventListener("webglcontextlost", function(e) { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);
      return canvas;
    })(),
    setStatus: function(text) {
      if (!Module.setStatus.last) Module.setStatus.last = { time: Date.now(), text: '' };
      if (text === Module.setStatus.last.text) return;

      var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
        var now = Date.now();
        if (m && now - Module.setStatus.last.time < 30) return; // if this is a progress update, skip it if too soon
        Module.setStatus.last.time = now;
        Module.setStatus.last.text = text;
        if (m) {
          text = m[1];
          progressElement.value = parseInt(m[2])*100;
          progressElement.max = parseInt(m[4])*100;
          progressElement.hidden = false;
        } else {
          progressElement.value = null;
          progressElement.max = null;
          progressElement.hidden = true;
        }
      statusElement.innerHTML = text;
    },
    totalDependencies: 0,
    monitorRunDependencies: function(left) {
      this.totalDependencies = Math.max(this.totalDependencies, left);
      Module.setStatus(left ? 'Preparing... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : 'All downloads complete.');
    }
  };
  Module.setStatus('Downloading...');
  window.onerror = function(event) {
    // TODO: do not warn on ok events like simulating an infinite loop or exitStatus
    Module.setStatus('Exception thrown, see JavaScript console');
    Module.setStatus = function(text) {
      if (text) Module.printErr('[post-exception status] ' + text);
    };
  };

  // var canvas = document.getElementById('canvas');
  stats = new Stats();
  stats.showPanel(0); // 0: fps, 1: ms, 2: mb, 3+: custom
  document.body.appendChild(stats.dom);
});
