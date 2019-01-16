#include "main.hpp"

// "esim.cpp"-teköäly

namespace {
struct Toteutus : public Aly {
	// Tiedot esimerkin liikkumista (spiraalia) varten.
	int laajuus, suunta, vaihe;

  Toteutus() : laajuus(1), suunta(0), vaihe(0) {}
  ~Toteutus() {}

  char siirto(const Peli &peli) final {
    // Muodostetaan spiraalia.
    const char oma = "wasd"[suunta];
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
}

std::unique_ptr<Aly> teeAly(const Peli &peli) {
  return std::unique_ptr<Aly>(new Toteutus());
}
