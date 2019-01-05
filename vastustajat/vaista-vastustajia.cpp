// Huono vastustajia jotenkin välttelevä harjoitusvastustaja
#include "../main.hpp"
#include <cmath>
#include <algorithm>
//#include <iostream>
//#include "../tulostus.hpp"

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

int lyhinEtaisyys(int x0, int y0, int x1, int y1) {
  int dx = std::abs(x1 - x0);
  int dy = std::abs(y1 - y0);

  dx = std::min(dx, leveys - dx);
  dy = std::min(dy, korkeus - dy);
  return dx + dy;
}

void laskeEtaisyydet(int x0, int y0, Lauta<int> &etaisyydet) {
  for (int y = 0; y < korkeus; ++y) {
    for (int x = 0; x < leveys; ++x) {
      etaisyydet(x,y) = lyhinEtaisyys(x0,y0,x,y);
    }
  }
}

float haeArvo(int x, int y, Lauta<char> &luvut,
  const Lauta<float> &heuristiikka, float heuristiikkapaino,
  const std::vector< Lauta<float> > &vastustajakerroinPerMaxSyvyys, int maxSyvyys)
{
  const char vanha = luvut(x,y);
  constexpr float diskonttauspaino = 0.05;
  const Lauta<float> &vastustajakerroin = vastustajakerroinPerMaxSyvyys[maxSyvyys];

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
        luvut, heuristiikka, heuristiikkapaino,
        vastustajakerroinPerMaxSyvyys, maxSyvyys-1));
    }
    luvut(x,y) = vanha;
  }

  return arvo * vastustajakerroin(x,y) + vanha * (1 + maxSyvyys * diskonttauspaino);
}

void laskeArvomatriisi(std::vector< std::vector<float> > &matriisi) {
  constexpr int koko = korkeus*leveys;
  matriisi.clear();
  matriisi.reserve(koko);

  Lauta<int> etaisyydet;
  Lauta<float> arvokentta;
  laskeEtaisyydet(0,0, etaisyydet);
  constexpr float kerroin = 0.6;

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
  std::vector< Lauta<int> > etaisyydetPelaajiin;
  std::vector< Lauta<float> > vastustajakerroinPerMaxSyvyys;
  char edellinenSiirto;
  int vastasiirrot;

  const int maxSyvyys;

  Toteutus(std::size_t nPelaajaa, int maxSyvyys = 8)
  :
    etaisyydetPelaajiin(nPelaajaa),
    vastustajakerroinPerMaxSyvyys(maxSyvyys+1),
    edellinenSiirto(0),
    vastasiirrot(0),
    maxSyvyys(maxSyvyys)
  {
    laskeArvomatriisi(arvomatriisi);
  }
  ~Toteutus() {}

  char siirto(const Peli &peli) final {
    laskeArvokentta(arvomatriisi, peli.lauta, arvokentta);

    for (std::size_t i = 0; i < peli.pelaajat.size(); ++i) {
      const auto &p = peli.pelaajat[i];
      laskeEtaisyydet(p.x, p.y, etaisyydetPelaajiin[i]);
    }

    const int maxSyvyysNyt = std::max(0, maxSyvyys - vastasiirrot);
    //tulostaHeuristiikka(etaisyydetPelaajiin[1], std::cerr);

    for (int syvyys = 0; syvyys <= maxSyvyysNyt; ++syvyys) {
      Lauta<float> &kentta = vastustajakerroinPerMaxSyvyys[maxSyvyysNyt - syvyys];
      for (int y = 0; y < korkeus; ++y) {
        for (int x = 0; x < leveys; ++x) {
          float arvo = arvokentta(x,y);
          constexpr float sakkokerroin = 0.3;
          constexpr float bonuskerroin = 1.0;
          // vain vastustajat
          for (std::size_t i = 1; i < peli.pelaajat.size(); ++i) {
            const int aika = syvyys - etaisyydetPelaajiin.at(i)(x,y) - 1;
            arvo *= std::pow(aika > 0 ? sakkokerroin : bonuskerroin, aika);
          }
          kentta(x,y) = arvo;
        }
      }
    }
    //tulostaHeuristiikka(vastustajakerroinPerMaxSyvyys[0], std::cerr);
    //tulostaHeuristiikka(arvokentta, std::cerr);

    const int omaX = peli.pelaajat[0].x;
    const int omaY = peli.pelaajat[0].y;
    constexpr float heuristiikkapaino = 0.1;

    char omaSiirto = '\0';
    float parasArvo = -1.0;
    bool vastasiirto = false;
    hakuCache = peli.lauta;

    for (const auto &siirto : siirrot) {
      float arvo = haeArvo(
        ((omaX + siirto.dx) + leveys) % leveys,
        ((omaY + siirto.dy) + korkeus) % korkeus,
        hakuCache, arvokentta, heuristiikkapaino,
        vastustajakerroinPerMaxSyvyys, maxSyvyysNyt);

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

std::unique_ptr<Aly> luoVaistaVastustajia(const Peli &peli, int maxSyvyys) {
  return std::unique_ptr<Aly>(new Toteutus(peli.pelaajat.size(), maxSyvyys));
}
