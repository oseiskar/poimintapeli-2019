# Poimintapelikilpailu

Suomeksi koodattu C++-teköäly Poimintapelikilpailuun.
Ks. https://www.ohjelmointiputka.net/kilpailut/2018-poimintapeli/

(_Toistaiseksi vain esimerkki Noden käytöstä_)

### Vaatimukset

 * g++ (testattu versiolla 6.3)
 * Shell-ohjelma (sh/bash/zsh/dash/...?)
 * GNU Make (testattu versiolla 4.1)

### Käyttö

Aja `make`, joka luo kilpailuohjelman `bin/main`. Onnistuu vaihtoehtoisesti
myös suoraan komennolla

    mkdir -p bin && g++ main.cpp aly.cpp -O2 -std=c++11 -o bin/main

### JavaScript-versio

`make js` tekee tiedostot `bin/main.js`, jota voi käyttää kilpailussa
 komennolla `node bin/main.js` (testattu Node-versiolla 8.11.4). Tämä vaatii
 Emscripten-kääntäjän (testattu versiolla 1.37.35).
