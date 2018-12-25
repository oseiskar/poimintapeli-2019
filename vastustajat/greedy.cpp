#include "../main.hpp"

namespace {
// Heuristiikka
void laskeArvokentta(const Lauta<char> luvut, Lauta<float> &kentta, Lauta<float> &cache, float vaimennus) {
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
          arvo += lahde.torus(x + siirto.dx, y + siirto.dy);
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

class Greedy : public Aly {
private:
  Lauta<float> arvokentta, cache;
  Lauta<char> hakuCache;
  float vaimennus;

public:
  Greedy(float vaimennus) : vaimennus(vaimennus) {}
  ~Greedy() {}

  char siirto(const Peli &peli) final {
    laskeArvokentta(peli.lauta, arvokentta, cache, vaimennus);

    const int omaX = peli.pelaajat[0].x;
    const int omaY = peli.pelaajat[0].y;

    char omaSiirto;
    float parasArvo = -1.0;
    hakuCache = peli.lauta;

    for (const auto &siirto : siirrot) {
      const float arvo = arvokentta.torus(omaX + siirto.dx, omaY + siirto.dy);

      if (arvo > parasArvo) {
        parasArvo = arvo;
        omaSiirto = siirto.merkki;
      }
    }
    return omaSiirto;
  }
};
}

std::unique_ptr<Aly> luoGreedy(float parametri = 0.6) {
  return std::unique_ptr<Aly>(new Greedy(parametri / 4));
}
