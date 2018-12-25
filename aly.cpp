#include "main.hpp"
#include <cmath>
#include "tulostus.hpp"

namespace {
// Heuristiikka
void laskeArvokentta(const std::vector< std::vector<float> > &matriisi, const Lauta<char> luvut, Lauta<float> &kentta) {
  constexpr int koko = leveys*korkeus;
  for (int i = 0; i < koko; ++i) {
    float arvo = 0.0;
    const auto &rivi = matriisi[i];
    for (int j = 0; j < koko; ++j) {
      arvo += rivi[j]*luvut(j);
    }
    kentta(i) = arvo;
  }
}

float haeArvo(int x, int y, Lauta<char> &luvut, const Lauta<float> &heuristiikka, float heuristiikkapaino, int maxSyvyys) {
  const char vanha = luvut(x,y);
  constexpr float diskonttauspaino = 0.05;

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
        luvut, heuristiikka, heuristiikkapaino, maxSyvyys-1));
    }
    luvut(x,y) = vanha;
  }

  return arvo + vanha * (1 + maxSyvyys * diskonttauspaino);
}

void laskeEtaisyydet(int x0, int y0, Lauta<int> &etaisyydet) {
  for (int y = 0; y < korkeus; ++y) {
    for (int x = 0; x < leveys; ++x) {
      etaisyydet(x,y) = -1;
    }
  }
  etaisyydet(x0, y0) = 0;
  bool yhtaan = true;
  for (int etaisyys = 1; yhtaan; etaisyys++) {
    yhtaan = false;
    for (int y = 0; y < korkeus; ++y) {
      for (int x = 0; x < leveys; ++x) {
        for (const auto &siirto : siirrot) {
          if (etaisyydet(x,y) < 0 &&
            etaisyydet.torus(x + siirto.dx, y + siirto.dy) == etaisyys - 1) {

            yhtaan = true;
            etaisyydet(x,y) = etaisyys;
            break;
          }
        }
      }
    }
  }
}

void laskeArvomatriisi(std::vector< std::vector<float> > &matriisi) {
  constexpr int koko = korkeus*leveys;
  matriisi.clear();
  matriisi.reserve(koko);

  Lauta<int> etaisyydet;
  Lauta<float> arvokentta;
  laskeEtaisyydet(0,0, etaisyydet);
  constexpr float kerroin = 0.7;

  for (int y = 0; y < korkeus; ++y) {
    for (int x = 0; x < leveys; ++x) {
      const int d = etaisyydet(x,y);
      arvokentta(x,y) = std::pow(kerroin, d);
    }
  }

  for (int y0 = 0; y0 < korkeus; ++y0) {
    for (int x0 = 0; x0 < leveys; ++x0) {
      matriisi.push_back({});
      std::vector< float > &rivi = matriisi.back();
      rivi.reserve(koko);

      for (int y1 = 0; y1 < korkeus; ++y1) {
        for (int x1 = 0; x1 < leveys; ++x1) {
          rivi.push_back(arvokentta.torus(x1-x0, y1-y0));
        }
      }
    }
  }
}

struct Toteutus : public Aly {
  std::vector< std::vector<float> > arvomatriisi;
  Lauta<float> arvokentta;
  Lauta<char> hakuCache;
  Lauta<int> etaisyydet;
  char edellinenSiirto;
  int vastasiirrot;
  const int maxSyvyys;
  const bool poistaLahi;

  Toteutus(int maxSyvyys = 8, bool poistaLahi = true)
  :
    edellinenSiirto('\0'),
    vastasiirrot(0),
    maxSyvyys(maxSyvyys),
    poistaLahi(poistaLahi)
  {
    laskeArvomatriisi(arvomatriisi);
  }
  ~Toteutus() {}

  char siirto(const Peli &peli) final {
    constexpr float heuristiikkapaino = 0.2;

    const int omaX = peli.pelaajat[0].x;
    const int omaY = peli.pelaajat[0].y;
    hakuCache = peli.lauta;

    if (poistaLahi) {
      laskeEtaisyydet(omaX, omaY, etaisyydet);
      constexpr int koko = leveys*korkeus;
      for (int i = 0; i < koko; ++i) {
        if (etaisyydet(i) < maxSyvyys + 1) {
          hakuCache(i) = 0;
        }
      }
    }

    laskeArvokentta(arvomatriisi, hakuCache, arvokentta);
    //tulostaHeuristiikka(arvokentta, std::cerr, 20);

    char omaSiirto;
    float parasArvo = -1.0;
    hakuCache = peli.lauta;
    bool vastasiirto = false;
    const int maxSyvyysNyt = std::max(0, maxSyvyys - vastasiirrot);

    for (const auto &siirto : siirrot) {
      const float arvo = haeArvo(
        ((omaX + siirto.dx) + leveys) % leveys,
        ((omaY + siirto.dy) + korkeus) % korkeus,
        hakuCache, arvokentta, heuristiikkapaino, maxSyvyysNyt);

      //std::cerr << siirto.merkki << " -> " << arvo << std::endl;

      if (arvo > parasArvo) {
        parasArvo = arvo;
        omaSiirto = siirto.merkki;
        vastasiirto = siirto.vastamerkki() == edellinenSiirto;
      }
    }

    if (vastasiirto) vastasiirrot += 2;
    else {
      vastasiirrot = std::max(vastasiirrot - 1, 0);
    }

    edellinenSiirto = omaSiirto;
    return omaSiirto;
  }
};
}

std::unique_ptr<Aly> luoAly(int maxSyvyys, bool poistaLahi) {
  return std::unique_ptr<Aly>(new Toteutus(maxSyvyys, poistaLahi));
}

std::unique_ptr<Aly> teeAly(const Peli &peli) {
  return std::unique_ptr<Aly>(new Toteutus());
}
