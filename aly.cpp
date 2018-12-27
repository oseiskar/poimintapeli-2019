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

float haeArvo(int x, int y, int t, Lauta<char> &luvut,
  const Lauta<float> &heuristiikka,
  const Lauta<int> &lahinVastustaja,
  const Lauta<float> &vastustajasakko,
  int maxSyvyys)
{
  const char vanha = luvut(x,y);
  constexpr float diskonttauspaino = 0.05;
  constexpr float heuristiikkapaino = 0.2;

  float tamaArvo = vanha * (1 + maxSyvyys * diskonttauspaino);
  if (lahinVastustaja(x,y) < t) {
    tamaArvo *= vastustajasakko(x,y);
  }

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
        t + 1,
        luvut, heuristiikka, lahinVastustaja, vastustajasakko, maxSyvyys-1));
    }
    luvut(x,y) = vanha;
  }

  return arvo + tamaArvo;
}

int lyhinEtaisyys(int x0, int y0, int x1, int y1) {
  int dx = std::abs(x1 - y0);
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

struct Ruutu {
  int x;
  int y;
  float arvo;
};

Ruutu haeKohdeTsp(int x, int y, int t, Lauta<char> &kaytetty,
  const std::vector<Ruutu> &eiTyhjat,
  const Lauta<int> &lahinVastustaja,
  const Lauta<float> &vastustajasakko,
  int maxSyvyys)
{
  assert(maxSyvyys > 0);

  const Ruutu *paras = nullptr;
  float parasArvo = -1;
  float diskonttauspaino = 0.1;

  for (const Ruutu &kohde : eiTyhjat) {
    if (kaytetty(kohde.x, kohde.y) > 0) continue;

    int etaisyys = lyhinEtaisyys(x, y,  kohde.x, kohde.y);
    int vastustajanAikaKohteessa = etaisyys - lahinVastustaja(kohde.x, kohde.y) + t;
    float aikakerroin = 1.0 / (1.0 + t * diskonttauspaino);
    if (vastustajanAikaKohteessa > 0) {
      aikakerroin *= 1.0 / (1 + vastustajanAikaKohteessa) * vastustajasakko(kohde.x, kohde.y);
    }
    if (vastustajanAikaKohteessa < 0) {
      aikakerroin *= 1.05;
    }

    float arvo = kohde.arvo * aikakerroin;
    if (maxSyvyys > 1) {
      kaytetty(kohde.x, kohde.y) = 1;
      arvo += haeKohdeTsp(kohde.x, kohde.y, t+etaisyys, kaytetty,
        eiTyhjat, lahinVastustaja, vastustajasakko, maxSyvyys-1).arvo;
      kaytetty(kohde.x, kohde.y) = 0;
    }

    if (arvo > parasArvo) {
      paras = &kohde;
      parasArvo = arvo;
    }
  }

  if (paras == nullptr) {
    // kaikki kaytetty
    return { 0, 0, 0 };
  }

  return { paras->x, paras->y, parasArvo };
}

bool oikeaSuunta(int deltaKohde, int delta) {
  if (deltaKohde == 0) return delta == 0;
  if (deltaKohde < 0) return delta <= 0;
  return delta >= 0;
}

struct Toteutus : public Aly {
  std::vector<Ruutu> eiTyhjat;
  Lauta<int> lahinVastustaja;
  Lauta<float> vastustajasakko;
  std::vector< std::vector<float> > arvomatriisi;
  Lauta<float> arvokentta;
  Lauta<char> hakuCache;
  Lauta<int> etaisyydet;
  Lauta<int> visiitit;
  Lauta<char> kaytetty;
  Ruutu kohde;
  const int maxSyvyys;
  const bool huomioiVastustajat;

  Toteutus(int maxSyvyys = 8, bool huomioiVastustajat = true)
  :
    kohde({0,0,0}),
    maxSyvyys(maxSyvyys),
    huomioiVastustajat(huomioiVastustajat)
  {
    laskeArvomatriisi(arvomatriisi);
    for (int i=0; i<leveys*korkeus; ++i) visiitit(i) = 0;
    eiTyhjat.reserve(leveys*korkeus);
  }
  ~Toteutus() {}

  char siirto(const Peli &peli) final {
    const int omaX = peli.pelaajat[0].x;
    const int omaY = peli.pelaajat[0].y;

    visiitit(omaX, omaY) += 1;
    hakuCache = peli.lauta;

    for (std::size_t i = 1; i < peli.pelaajat.size(); ++i) {
      const auto &p = peli.pelaajat[i];
      laskeEtaisyydet(p.x, p.y, etaisyydet);
      for (int j=0; j<leveys*korkeus; ++j) {
        if (i == 1) vastustajasakko(j) = 1.0;
        int e = etaisyydet(j);
        if (huomioiVastustajat && e > 0 && e <= 1) {
          const float sakko = 0.1;
          vastustajasakko(j) *= sakko;
        }
        if (i == 1 || e < lahinVastustaja(j)) {
          lahinVastustaja(j) = e;
        }
      }
    }

    eiTyhjat.clear();
    for (int y = 0; y < korkeus; ++y) {
      for (int x = 0; x < leveys; ++x) {
        const char arvo = peli.lauta(x,y);
        if (arvo > 0) {
          eiTyhjat.push_back({x, y, (float)arvo});
        }
        kaytetty(x,y) = 0;
      }
    }

    if (eiTyhjat.size() > 0 && eiTyhjat.size() < 10) {
      // ei vaihdeta kohdetta turhaan
      if (kohde.arvo <= 0 || peli.lauta(kohde.x, kohde.y) == 0) {
        if (eiTyhjat.size() > 1) {
          constexpr int maxTspSyvyys = 4;
          kohde = haeKohdeTsp(omaX, omaY, 0, kaytetty, eiTyhjat, lahinVastustaja, vastustajasakko, maxTspSyvyys);
        } else {
          kohde = eiTyhjat[0];
        }
      }
      int dx = kohde.x - omaX;
      int dy = kohde.y - omaY;
      if (std::abs(dx) > leveys - std::abs(dx)) dx = -dx;
      if (std::abs(dy) > korkeus - std::abs(dy)) dy = -dy;

      //std::cerr << kohde.x << " " << kohde.y << " " << dx << " " << dy << std::endl;

      char omaSiirto = '\0';
      float parasArvo = -1000;
      for (const auto &siirto : siirrot) {
        if (oikeaSuunta(dx, siirto.dx) && oikeaSuunta(dy, siirto.dy)) {
          const int x = omaX + siirto.dx;
          const int y = omaY + siirto.dy;
          const float arvo = arvokentta.torus(x,y) - visiitit.torus(x,y);
          if (arvo > parasArvo) {
            parasArvo = arvo;
            omaSiirto = siirto.merkki;
          }
        }
      }

      assert(omaSiirto != '\0');
      return omaSiirto;

    } else {
      laskeEtaisyydet(omaX, omaY, etaisyydet);
      constexpr int koko = leveys*korkeus;
      for (int i = 0; i < koko; ++i) {
        if (etaisyydet(i) < maxSyvyys + 1) {
          hakuCache(i) = 0;
        }
      }

      laskeArvokentta(arvomatriisi, hakuCache, arvokentta);
      //tulostaHeuristiikka(arvokentta, std::cerr, 20);

      char omaSiirto = 0;
      float parasArvo = -1.0;
      hakuCache = peli.lauta;

      for (const auto &siirto : siirrot) {

        const int x = ((omaX + siirto.dx) + leveys) % leveys;
        const int y = ((omaY + siirto.dy) + korkeus) % korkeus;

        float sakko = 1.0;
        if (visiitit(x,y) > 2) {
          sakko = 1.0 / (1.0 + visiitit(x,y) - 2);
        }

        const float arvo = haeArvo(x, y, 0, hakuCache, arvokentta,
          lahinVastustaja, vastustajasakko, maxSyvyys) * sakko;

        //std::cerr << siirto.merkki << " -> " << arvo << std::endl;

        if (arvo > parasArvo) {
          parasArvo = arvo;
          omaSiirto = siirto.merkki;
        }
      }

      assert(omaSiirto != 0);
      return omaSiirto;
    }
  }
};
}

std::unique_ptr<Aly> luoAly(const Peli &peli, int maxSyvyys, bool huom) {
  return std::unique_ptr<Aly>(new Toteutus(maxSyvyys, huom));
}

std::unique_ptr<Aly> teeAly(const Peli &peli) {
  return std::unique_ptr<Aly>(new Toteutus());
}
