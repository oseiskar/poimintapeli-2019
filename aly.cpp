#include "main.hpp"
#include <cmath>
#include "tulostus.hpp"

namespace {
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

struct Ruutu {
  int x;
  int y;
  float arvo;
};

Ruutu haeKohde(int x, int y, int t, Lauta<char> &kaytetty, const std::vector<Ruutu> &eiTyhjat, const Lauta<int> &lahinVastustaja, int maxSyvyys) {
  assert(maxSyvyys > 0);

  const Ruutu *paras = nullptr;
  float parasArvo = -1;
  float diskonttauspaino = 0.1;

  for (const Ruutu &kohde : eiTyhjat) {
    if (kaytetty(kohde.x, kohde.y) > 0) continue;

    int etaisyys = std::abs(x-kohde.x) + std::abs(y-kohde.y);
    int vastustajanAikaKohteessa = etaisyys - lahinVastustaja(kohde.x, kohde.y) + t;
    float aikakerroin = 1.0 / (1.0 + t * diskonttauspaino) * (vastustajanAikaKohteessa > 0 ? (1.0 / (1 + vastustajanAikaKohteessa)) : 1.0);

    float arvo = kohde.arvo * aikakerroin;
    if (maxSyvyys > 1) {
      kaytetty(kohde.x, kohde.y) = 1;
      arvo += haeKohde(kohde.x, kohde.y, t+etaisyys, kaytetty, eiTyhjat, lahinVastustaja, maxSyvyys-1).arvo;
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

struct Toteutus : public Aly {
  std::vector< Lauta<int> > pelaajienEtaisyydet;
  Lauta<int> lahinVastustaja;
  Lauta<char> kaytetty;
  std::vector<Ruutu> eiTyhjat;

  Toteutus(int nPelaajia) :
    pelaajienEtaisyydet(nPelaajia) {
    eiTyhjat.reserve(leveys*korkeus);
  }
  ~Toteutus() {}

  char siirto(const Peli &peli) final {
    const int omaX = peli.pelaajat[0].x;
    const int omaY = peli.pelaajat[0].y;

    for (std::size_t i = 0; i < peli.pelaajat.size(); ++i) {
      const auto &p = peli.pelaajat[i];
      laskeEtaisyydet(p.x, p.y, pelaajienEtaisyydet[i]);
    }

    for (int y = 0; y < korkeus; ++y) {
      for (int x = 0; x < leveys; ++x) {
        lahinVastustaja(x,y) = 0;
        for (std::size_t i = 1; i < peli.pelaajat.size(); ++i) {
          const int e = pelaajienEtaisyydet[i](x,y);
          if (i == 1 || e < lahinVastustaja(x,y)) {
            lahinVastustaja(x,y) = e;
          }
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

    if (eiTyhjat.size() == 0) {
      std::cerr << "varoitus: kaikki tyhjaa" << std::endl;
      return 'w';
    }

    Ruutu kohde;
    if (eiTyhjat.size() > 1) {
      constexpr unsigned MAX_EVAL = 100000;
      unsigned total = 1;
      unsigned int maxSyvyys = 0;

      while (total * eiTyhjat.size() <= MAX_EVAL) {
        maxSyvyys++;
        total *= eiTyhjat.size();
      }

      std::cerr  << "eiTyhjat " << eiTyhjat.size() << ", " << "maxSyvyys " << maxSyvyys << std::endl;

      kohde = haeKohde(omaX, omaY, 0, kaytetty, eiTyhjat, lahinVastustaja, maxSyvyys);
    } else {
      kohde = eiTyhjat[0];
    }

    const int dx = kohde.x - omaX;
    const int dy = kohde.y - omaY;

    if (dx > 0) {
      return 'd';
    } else if (dx < 0) {
      return 'a';
    } else if (dy > 0) {
      return 's';
    } else if (dy < 0) {
      return 'w';
    }

    assert(false);
    return '\0';
  }
};
}

std::unique_ptr<Aly> luoAly(const Peli &peli) {
  return std::unique_ptr<Aly>(new Toteutus(peli.pelaajat.size()));
}

std::unique_ptr<Aly> teeAly(const Peli &peli) {
  return std::unique_ptr<Aly>(new Toteutus(peli.pelaajat.size()));
}
