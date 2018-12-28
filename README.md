# Poimintapelikilpailu

Suomeksi koodattu C++-teköäly Poimintapelikilpailuun.
Ks. https://www.ohjelmointiputka.net/kilpailut/2018-poimintapeli/

### Vaatimukset

 * g++ (testattu versiolla 6.3)
 * Shell-ohjelma (sh/bash/zsh/dash/...?)
 * GNU Make (testattu versiolla 4.1)

### Käyttö

Aja `make`, joka luo kilpailuohjelman `bin/main`. Onnistuu vaihtoehtoisesti
myös suoraan komennolla

    mkdir -p bin && g++ main.cpp aly.cpp -O2 -std=c++11 -o bin/main

### Harjoitusottelu

 * `make match` ajaa monta ottelua harjoitusvastustajia vastaan ja
    näyttää voittojen määrät.

 * `make show` näyttää myös ottelut ASCII-grafiikkana
    (käyttää `usleep`-funktiota `unistd.h`-tiedostosta, joten ei toimine
    Windowsilla)

### JavaScript-versio

 * `make js` tekee tiedostot `bin/main.js` ja `bin/ugly.js`, joita voi käyttää
   kilpailussa komennolla `node bin/ugly.js` (testattu Node-versiolla 8.11.4)
