// voidaan lisätä Emscriptenin tuottaman tiedoston loppuun
var alustus = (typeof Module !== 'undefined') ?
  Module.alusta :
  require('./bin/main.js').alusta;

var tilakone = alustus();

process.stdin.setEncoding('utf-8');
process.stdin.on('readable', function () {
  var data = process.stdin.read();
  if (data === null) return;
  data.split('\n')
  .map(function(line) { return line.trim(); })
  .filter(function (line) { return !!line; })
  .forEach(function (line) {
    if (!tilakone) return;
    var vastaus = tilakone.kasitteleRivi(line);
    if (vastaus === 'LOPPU') {
      tilakone = null;
    } else if (vastaus !== '') {
      process.stdout.write(vastaus+'\n');
    }
  });
});
