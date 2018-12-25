#include "../main.hpp"
#include <cmath>
//#include <iostream>

namespace {
// Heuristiikka
void laskeArvokentta(const Lauta<char> luvut, Lauta<float> &kentta, Lauta<float> &cache) {
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
        float keski = lahde(x,y);

        float arvo = 0.0;
        for (const auto &siirto : siirrot) {
          arvo += lahde.torus(x + siirto.dx, y + siirto.dy);
        }
        // ei jää ikuiseen silmukkaan, jos kaikki nollaa
        if (arvo > 0.0 && keski <= 0.0) yhtaan = true;
        arvo = (keski * 3.0 + arvo) / 7.0;

        kohde(x,y) = arvo;
      }
    }

    kohdeCache = !kohdeCache;
  }

  kentta = cache;
}

void laskeEtaisyydet(int x0, int y0, Lauta<int> etaisyydet) {
  for (int y = 0; y < korkeus; ++y) {
    for (int x = 0; x < leveys; ++x) {
      etaisyydet(x,y) = -1;
    }
  }
  etaisyydet(x0, y0) = 0;
  bool yhtaan = true;
  int etaisyys = 1;
  while (yhtaan) {
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

float haeArvo(int x, int y, Lauta<char> &luvut,
  const Lauta<float> &heuristiikka, float heuristiikkapaino,
  const std::vector< Lauta<float> > &vastustajakerroinPerMaxSyvyys, int maxSyvyys)
{
  const char vanha = luvut(x,y);
  constexpr float diskonttauspaino = 0.02;
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

struct Toteutus : public Aly {
  Lauta<float> arvokentta, cache;
  Lauta<char> hakuCache;
  std::vector< Lauta<int> > etaisyydetPelaajiin;
  std::vector< Lauta<float> > vastustajakerroinPerMaxSyvyys;
  char edellinenSiirto;

  const int maxSyvyys;

  Toteutus(std::size_t nPelaajaa, int maxSyvyys = 8)
  :
    etaisyydetPelaajiin(nPelaajaa),
    vastustajakerroinPerMaxSyvyys(maxSyvyys+1),
    edellinenSiirto(0),
    maxSyvyys(maxSyvyys)
  {}
  ~Toteutus() {}

  char siirto(const Peli &peli) final {
    laskeArvokentta(peli.lauta, arvokentta, cache);

    for (std::size_t i = 0; i < peli.pelaajat.size(); ++i) {
      const auto &p = peli.pelaajat[i];
      laskeEtaisyydet(p.x, p.y, etaisyydetPelaajiin.at(i));
    }

    for (int syvyys = 0; syvyys <= maxSyvyys; ++syvyys) {
      Lauta<float> &kentta = vastustajakerroinPerMaxSyvyys[maxSyvyys - syvyys];
      for (int y = 0; y < korkeus; ++y) {
        for (int x = 0; x < leveys; ++x) {
          float arvo = arvokentta(x,y);
          constexpr float sakkokerroin = 0.7;
          constexpr float bonuskerroin = 1.00;
          // vain vastustajat
          for (std::size_t i = 1; i < peli.pelaajat.size(); ++i) {
            const int aika = etaisyydetPelaajiin.at(i)(x,y) - syvyys - 1;
            arvo *= std::pow(aika > 0 ? sakkokerroin : bonuskerroin, aika);
          }
          kentta(x,y) = arvo;
        }
      }
    }

    const int omaX = peli.pelaajat[0].x;
    const int omaY = peli.pelaajat[0].y;
    constexpr float vastasiirtosakko = 0.9;
    constexpr float heuristiikkapaino = 0.2;

    char omaSiirto;
    float parasArvo = -1.0;
    hakuCache = peli.lauta;

    for (const auto &siirto : siirrot) {
      float arvo = haeArvo(
        ((omaX + siirto.dx) + leveys) % leveys,
        ((omaY + siirto.dy) + korkeus) % korkeus,
        hakuCache, arvokentta, heuristiikkapaino,
        vastustajakerroinPerMaxSyvyys, maxSyvyys);

      //std::cerr << siirto.merkki << " -> " << arvo << std::endl;

      if (siirto.vastamerkki() == edellinenSiirto) {
        arvo *= vastasiirtosakko;
      }

      if (arvo > parasArvo) {
        parasArvo = arvo;
        omaSiirto = siirto.merkki;
      }
    }

    edellinenSiirto = omaSiirto;
    return omaSiirto;
  }
};
}

std::unique_ptr<Aly> luoVaistaVastustajia(const Peli &peli, int maxSyvyys) {
  return std::unique_ptr<Aly>(new Toteutus(peli.pelaajat.size(), maxSyvyys));
}
