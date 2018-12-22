#include "main.hpp"

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
  constexpr float vaimennus = 0.9 / 4;

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

struct Aly::Toteutus {
  Lauta<float> arvokentta, cache;

  Toteutus() {
  }

  char siirto(const Peli &peli) {
    laskeArvokentta(peli.lauta, arvokentta, cache);

    const int omaX = peli.pelaajat[0].x;
    const int omaY = peli.pelaajat[0].y;

    char omaSiirto;
    float parasArvo = -1.0;

    for (const auto &siirto : siirrot) {
      const float arvo = arvokentta(
        ((omaX + siirto.dx) + leveys) % leveys,
        ((omaY + siirto.dy) + korkeus) % korkeus);

      if (arvo > parasArvo) {
        parasArvo = arvo;
        omaSiirto = siirto.merkki;
      }
    }
    return omaSiirto;
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
