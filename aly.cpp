#include "main.hpp"
#include <cmath>
//#include "tulostus.hpp"

namespace {
struct Ruutu {
  int x;
  int y;
  float arvo;
};

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

Ruutu haeKohde(int x, int y, int t, Lauta<char> &luvut,
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

  Ruutu paras = { 0, 0, 0 };
  if (maxSyvyys == 0) {
    paras = { x, y, heuristiikka(x,y) * heuristiikkapaino };
  }
  else {
    luvut(x,y) = 0;
    for (const auto &siirto : siirrot) {
      Ruutu kohde = haeKohde(
        ((x + siirto.dx) + leveys) % leveys,
        ((y + siirto.dy) + korkeus) % korkeus,
        t + 1,
        luvut, heuristiikka, lahinVastustaja, vastustajasakko, maxSyvyys-1);
      if (kohde.arvo > paras.arvo) {
        paras = kohde;
      }
    }
    luvut(x,y) = vanha;
  }

  if (tamaArvo > 0) {
    paras.x = x;
    paras.y = y;
  }
  paras.arvo += tamaArvo;
  return paras;
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
  Lauta<char> kaytetty;
  Ruutu kohde;
  const int maxSyvyys;
  const unsigned tspKynnys;

  Toteutus(int maxSyvyys = 8, unsigned tspKynnys = 10)
  :
    kohde({0,0,0}),
    maxSyvyys(maxSyvyys),
    tspKynnys(tspKynnys)
  {
    laskeArvomatriisi(arvomatriisi);
    eiTyhjat.reserve(leveys*korkeus);
  }
  ~Toteutus() {}

  char siirto(const Peli &peli) final {
    const int omaX = peli.pelaajat[0].x;
    const int omaY = peli.pelaajat[0].y;

    hakuCache = peli.lauta;

    for (std::size_t i = 1; i < peli.pelaajat.size(); ++i) {
      const auto &p = peli.pelaajat[i];
      laskeEtaisyydet(p.x, p.y, etaisyydet);
      for (int j=0; j<leveys*korkeus; ++j) {
        if (i == 1) vastustajasakko(j) = 1.0;
        int e = etaisyydet(j);
        if (e > 0) {
          const float sakko = 1.0 - 0.9 / e;
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

    Ruutu uusiKohde;
    if (eiTyhjat.size() > 0 && eiTyhjat.size() < tspKynnys) {
      if (eiTyhjat.size() > 1) {
        constexpr int maxTspSyvyys = 4;
        uusiKohde = haeKohdeTsp(omaX, omaY, 0, kaytetty, eiTyhjat, lahinVastustaja, vastustajasakko, maxTspSyvyys);
      } else {
        uusiKohde = eiTyhjat[0];
      }
    } else {
      laskeEtaisyydet(omaX, omaY, etaisyydet);
      constexpr int koko = leveys*korkeus;
      for (int i = 0; i < koko; ++i) {
        if (etaisyydet(i) < maxSyvyys + 1) {
          hakuCache(i) = 0;
        }
      }

      laskeArvokentta(arvomatriisi, hakuCache, arvokentta);
      //tulostaHeuristiikka(arvokentta, std::cerr, 200);
      //tulostaHeuristiikka(etaisyydet, std::cerr, 1);
      //tulostaHeuristiikka(hakuCache, std::cerr, 1);

      hakuCache = peli.lauta;
      uusiKohde = haeKohde(omaX, omaY, 0, hakuCache, arvokentta,
        lahinVastustaja, vastustajasakko, maxSyvyys+1);
    }

    if (kohde.arvo <= 0 ||
        (kohde.x == omaX && kohde.y == omaY) ||
        (peli.lauta(uusiKohde.x, uusiKohde.y) > 0 && (
          peli.lauta(kohde.x, kohde.y) == 0 ||
          lyhinEtaisyys(omaX, omaY, uusiKohde.x, uusiKohde.y) < lyhinEtaisyys(omaX, omaY, kohde.x, kohde.y)
        ))
      )
    {
      kohde = uusiKohde;
    }

    int dx = kohde.x - omaX;
    int dy = kohde.y - omaY;
    if (std::abs(dx) > leveys - std::abs(dx)) dx = -dx;
    if (std::abs(dy) > korkeus - std::abs(dy)) dy = -dy;

    //std::cerr << kohde.x << " " << kohde.y << std::endl;

    char omaSiirto = '\0';
    float parasArvo = -1000;
    for (const auto &siirto : siirrot) {
      if (oikeaSuunta(dx, siirto.dx) && oikeaSuunta(dy, siirto.dy)) {
        const int x = omaX + siirto.dx;
        const int y = omaY + siirto.dy;
        const float arvo = arvokentta.torus(x,y);
        if (arvo > parasArvo) {
          parasArvo = arvo;
          omaSiirto = siirto.merkki;
        }
      }
    }

    assert(omaSiirto != '\0');
    return omaSiirto;
  }
};
}

std::unique_ptr<Aly> luoAly(const Peli &peli, int maxSyvyys, unsigned tspKynnys) {
  return std::unique_ptr<Aly>(new Toteutus(maxSyvyys, tspKynnys));
}

std::unique_ptr<Aly> teeAly(const Peli &peli) {
  return std::unique_ptr<Aly>(new Toteutus());
}
