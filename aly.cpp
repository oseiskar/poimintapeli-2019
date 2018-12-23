#include "main.hpp"
//#include <iostream>

namespace {
struct Siirto {
  char merkki;
  int dx;
  int dy;
};

const Siirto siirrot[4] = {
  { 'w', 0, -1 },
  { 'a', -1, 0 },
  { 's', 0, 1 },
  { 'd', 1, 0 }
};

// Heuristiikka
void laskeArvokentta(const Lauta<char> luvut, Lauta<float> &kentta, Lauta<float> &cache) {
  constexpr float vaimennus = 0.6 / 4;

  for (int y = 0; y < korkeus; ++y) {
    for (int x = 0; x < leveys; ++x) {
      cache(x,y) = (float)luvut(x,y);
    }
  }

  bool kohdeCache = false;
  bool yhtaan = true;
  while (yhtaan) {
    yhtaan = false;
    auto &lahde = kohdeCache ? kentta : cache;
    auto &kohde = kohdeCache ? cache : kentta;

    for (int y = 0; y < korkeus; ++y) {
      for (int x = 0; x < leveys; ++x) {
        if (lahde(x,y) > 0.0) {
          kohde(x,y) = lahde(x,y);
          continue;
        }

        float arvo = 0.0;
        for (const auto &siirto : siirrot) {
          arvo += lahde(
            ((x + siirto.dx) + leveys) % leveys,
            ((y + siirto.dy) + korkeus) % korkeus);
        }
        // ei jää ikuiseen silmukkaan, jos kaikki nollaa
        if (arvo > 0.0) yhtaan = true;

        kohde(x,y) = arvo * vaimennus;
      }
    }

    kohdeCache = !kohdeCache;
  }

  kentta = cache;
}

float haeArvo(int x, int y, Lauta<char> &luvut, const Lauta<float> &heuristiikka, int maxSyvyys) {
  const char vanha = luvut(x,y);
  constexpr float heuristiikkapaino = 0.01;
  constexpr float diskonttauspaino = 0.01;

  float arvo = 0.0;
  if (maxSyvyys == 0) {
    arvo = heuristiikka(x,y) * heuristiikkapaino;
  }
  else {
    luvut(x,y) = 0;
    for (const auto &siirto : siirrot) {
      arvo = std::max(arvo, haeArvo(
        ((x + siirto.dx) + leveys) % leveys,
        ((y + siirto.dy) + korkeus) % korkeus,
        luvut, heuristiikka, maxSyvyys-1));
    }
    luvut(x,y) = vanha;
  }

  return arvo + vanha * (1 + maxSyvyys * diskonttauspaino);
}

struct Toteutus : public Aly {
  Lauta<float> arvokentta, cache;
  Lauta<char> hakuCache;

  Toteutus() {}
  ~Toteutus() {}

  const char * nimi() const final {
    return "aly";
  }

  char siirto(const Peli &peli) final {
    laskeArvokentta(peli.lauta, arvokentta, cache);

    const int omaX = peli.pelaajat[0].x;
    const int omaY = peli.pelaajat[0].y;

    char omaSiirto;
    float parasArvo = -1.0;
    constexpr int MAX_SYVYYS = 8;
    hakuCache = peli.lauta;

    for (const auto &siirto : siirrot) {
      const float arvo = haeArvo(
        ((omaX + siirto.dx) + leveys) % leveys,
        ((omaY + siirto.dy) + korkeus) % korkeus,
        hakuCache, arvokentta, MAX_SYVYYS);

      //std::cerr << siirto.merkki << " -> " << arvo << std::endl;

      if (arvo > parasArvo) {
        parasArvo = arvo;
        omaSiirto = siirto.merkki;
      }
    }
    return omaSiirto;
  }
};
}

std::unique_ptr<Aly> teeAly(const Peli &peli) {
  return std::unique_ptr<Aly>(new Toteutus());
}
