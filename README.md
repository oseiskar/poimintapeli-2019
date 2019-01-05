# Poimintapelikilpailu

Suomeksi koodattu C++-teköäly Poimintapelikilpailuun.
Ks. https://www.ohjelmointiputka.net/kilpailut/2018-poimintapeli/

### Toimintaperiaate

**Algoritmi 1**: Laskee kaikki omat siirrot tiettyyn syvyyteen asti ja valitsee
parhaan siirtosarjan sen mukaan, kuinka paljon pisteitä sen varrella on,
kuinka aikaisessa vaiheessa ne kerätään, kuinka todennäköisesti jokin vastustaja
ehtii missäkin kohtaa väliin ja kuinka hyvä "potentiaali" (heuristiikka) sarjan
viimeisellä ruudulla on.

**Algoritmi 2**: Käytetään loppupelissä, jos jäljellä olevia ei-tyhjiä ruutuja
on riittävän vähän. Valitaan N-lupaavinta jäljellä olevaa ruutua jä järjestys,
jossa ne kannattaa yrittää kerätä.

Lisäksi käytetään, kummassakin algoritmissa logiikkaa, joka rajoittaa, milloin
suunniteltua reittiä voidaan vaihtaa, mikä estää oman pelaajan jäämisen jumiin
kahden tai useamman ruudun sykliin.

Tarkemmat detaljit voi katsoa tiedostosta `aly.cpp`.

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

`make js` tekee tiedostot `bin/main.js`, jota voi käyttää kilpailussa
 komennolla `node bin/main.js` (testattu Node-versiolla 8.11.4). Tämä vaatii
 Emscripten-kääntäjän (testattu versiolla 1.37.35).
