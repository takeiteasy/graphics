var stats = undefined, ansi_up = new AnsiUp();
var hal_settings = function() {
  this.resize_canvas = true;
  this.lock_cursor = false;
  this.show_stdout = false;
}
var settings = new hal_settings();
var Module = undefined;

function has_class(el, className) {
  if (el.classList)
    return el.classList.contains(className);
  return !!el.className.match(new RegExp('(\\s|^)' + className + '(\\s|$)'));
}

function add_class(el, className) {
  if (el.classList)
    el.classList.add(className)
  else if (!has_class(el, className))
    el.className += " " + className;
}

function remove_class(el, className) {
  if (el.classList)
    el.classList.remove(className)
  else if (has_class(el, className))
  {
    var reg = new RegExp('(\\s|^)' + className + '(\\s|$)');
    el.className = el.className.replace(reg, ' ');
  }
}

function toggle_class(el, className) {
  if (has_class(el, className))
    remove_class(el, className)
  else
    add_class(el, className)
}

document.addEventListener("DOMContentLoaded", function(e) {
  var statusElement = document.getElementById('status');
  var progressElement = document.getElementById('progress');
  var outputElement = document.getElementById("output_border");

  Module = {
    preRun: [],
    postRun: [],
    addLog: function(cls, pre, text) {
      var div = document.createElement("div");
      div.id = '';
      div.className = cls;
      div.innerHTML = '[ ' + new Date().toUTCString() + ' ] ';
      div.innerHTML += pre;
      div.innerHTML += text;
      outputElement.appendChild(div);
      outputElement.scrollTop = outputElement.scrollHeight;
    },
    print: (function() {
      return function(text) {
        if (arguments.length > 1)
          text = Array.prototype.slice.call(arguments).join(' ');
        if (text.length == 0)
          return;
        Module.addLog('log', '', ansi_up.ansi_to_html(text));
        console.log(text);
      };
    })(),
    printErr: function(text) {
      if (arguments.length > 1)
        text = Array.prototype.slice.call(arguments).join(' ');
      Module.addLog('error log', '<b>ERROR</b>: ', ansi_up.ansi_to_html(text));
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

      if (text.length > 1)
        Module.addLog('status log', '<b>STATUS</b>: ', ansi_up.ansi_to_html(text));

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

  var output_border = document.getElementById('output_border');
  var canvas = document.getElementById('canvas');

  stats = new Stats();
  stats.showPanel(0); // 0: fps, 1: ms, 2: mb, 3+: custom
  document.body.appendChild(stats.dom);
});
