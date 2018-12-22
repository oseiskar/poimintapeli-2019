#include "main.hpp"

struct Aly::Toteutus {
  // Tiedot esimerkin liikkumista (spiraalia) varten.
  int laajuus, suunta, vaihe;

  Toteutus() {
    laajuus = 1;
    suunta = 0;
    vaihe = 0;
  }

  char siirto(const Peli &peli) {
    // Muodostetaan spiraalia.
    char oma = "wasd"[suunta];
    vaihe += 1;
    if (vaihe >= laajuus) {
      vaihe = 0;
      suunta += 1;
      if (suunta == 2) {
        laajuus += 1;
      } else if (suunta == 4) {
        laajuus += 1;
        suunta = 0;
      }
    }
    return oma;
  }
};

Aly::Aly(const Peli &peli) : toteutus(new Toteutus()) {}
Aly::~Aly() {}

char Aly::siirto(const Peli &peli) {
  return toteutus->siirto(peli);
}

const char * Aly::nimi() const {
  return "aly";
}
