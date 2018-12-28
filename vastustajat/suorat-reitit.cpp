#include "../main.hpp"
#include <cmath>
#include <algorithm>
//#include "../tulostus.hpp"

namespace {
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

struct Ruutu {
  int x;
  int y;
  float arvo;
};

void laskeSuoratReitit(int x0, int y0, const Lauta<char> &luvut, const Lauta<int> &lahinVastustaja, Lauta<float> &arvokentta, Lauta<Siirto> &suunnat) {
  for (int y = 0; y < korkeus; ++y) {
    for (int x = 0; x < leveys; ++x) {
      arvokentta(x,y) = -1;
    }
  }
  arvokentta(x0, y0) = 0;
  int etaisyys = 1;
  while (true) {
    bool yhtaan = false;
    for (int y = 0; y < korkeus; ++y) {
      for (int x = 0; x < leveys; ++x) {
        if (arvokentta(x,y) >= 0) continue;

        for (const auto &siirto : siirrot) {
          if (arvokentta.torus(x + siirto.dx, y + siirto.dy) >= 0) {
            yhtaan = true;
            arvokentta(x,y) = -2;
            break;
          }
        }
      }
    }

    if (!yhtaan) break;

    for (int y = 0; y < korkeus; ++y) {
      for (int x = 0; x < leveys; ++x) {
        if (arvokentta(x,y) >= -1) continue;

        float arvo = -1;
        for (const auto &siirto : siirrot) {
          const float a = arvokentta.torus(x + siirto.dx, y + siirto.dy);
          if (a > arvo) {
            arvo = a;
            suunnat(x,y) = siirto;
          }
        }
        const int vastustajanAika = etaisyys - lahinVastustaja(x, y);
        const float aikakerroin = vastustajanAika > 0 ? std::pow(0.5, vastustajanAika) : 1.0;
        arvokentta(x,y) = arvo + luvut(x,y) * aikakerroin;
      }
    }
  }
}

Ruutu haeKohdeTsp(int x, int y, int t, Lauta<char> &kaytetty, const std::vector<Ruutu> &eiTyhjat, const Lauta<int> &lahinVastustaja, int maxSyvyys) {
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
      arvo += haeKohdeTsp(kohde.x, kohde.y, t+etaisyys, kaytetty, eiTyhjat, lahinVastustaja, maxSyvyys-1).arvo;
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

float haeArvo(int x, int y, int t, Lauta<char> &luvut, const Lauta<int> &lahinVastustaja, int maxSyvyys) {
  const char vanha = luvut(x,y);
  float arvo = 0;
  constexpr float diskonttauspaino = 0.01;
  if (maxSyvyys > 0) {
    luvut(x,y) = 0;
    for (const auto &siirto : siirrot) {
      arvo = std::max(arvo, haeArvo(
        ((x + siirto.dx) + leveys) % leveys,
        ((y + siirto.dy) + korkeus) % korkeus,
        t+1, luvut, lahinVastustaja, maxSyvyys-1));
    }
    luvut(x,y) = vanha;
  }
  float aikakerroin = 1.0 / (1.0 + diskonttauspaino*t);
  if (vanha > 0) {
    const int vastustajanAika = t - lahinVastustaja(x, y);
    if (vastustajanAika > 0) {
      aikakerroin = 1.0 / (vastustajanAika + 1);
    }
  }
  return arvo + vanha * aikakerroin;
}

float haeArvo(int omaX, int omaY, const Ruutu &kohde,
  const Lauta<Siirto> &suunnat, const Lauta<char> &luvut, const Lauta<int> lahinVastustaja,
  Lauta<char> &hakuCache, int maxSyvyys)
{
  hakuCache = luvut;
  int x = kohde.x, y = kohde.y;
  int t = 0;
  while (x != omaX || y != omaY) {
    hakuCache(x, y) = 0;
    const Siirto &suunta = suunnat(x, y);
    x = (x + suunta.dx + leveys) % leveys;
    y = (y + suunta.dy + korkeus) % korkeus;
    t++;
  }

  float arvo = 0;
  if (maxSyvyys > 0) {
    for (const auto &siirto : siirrot) {
      // TODO
      arvo = std::max(arvo, haeArvo(
        ((kohde.x + siirto.dx) + leveys) % leveys,
        ((kohde.y + siirto.dy) + korkeus) % korkeus,
        t+1, hakuCache, lahinVastustaja, maxSyvyys));
    }
  }

  return kohde.arvo + arvo;
}

struct Toteutus : public Aly {
  std::vector< Lauta<int> > pelaajienEtaisyydet;
  const int maxSyvyys;
  Lauta<float> arvokentta;
  Lauta<Siirto> suunnat;
  Lauta<int> lahinVastustaja;
  Lauta<char> kaytetty;
  Lauta<char> hakuCache;
  std::vector<Ruutu> eiTyhjat;

  Toteutus(int nPelaajia, int maxSyvyys = 4) :
    pelaajienEtaisyydet(nPelaajia), maxSyvyys(maxSyvyys) {
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

    laskeSuoratReitit(omaX, omaY, peli.lauta, lahinVastustaja, arvokentta, suunnat);

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
      assert(false && "kaikki tyhjaa");
      return 'w';
    }

    Ruutu kohde;
    if (eiTyhjat.size() > 1) {
      constexpr unsigned MAX_EVAL = 100000;
      unsigned total = 1;
      unsigned int maxTspSyvyys = 0;

      while (total * eiTyhjat.size() <= MAX_EVAL) {
        maxTspSyvyys++;
        total *= eiTyhjat.size();
      }

      if (eiTyhjat.size() < 10) {
        kohde = haeKohdeTsp(omaX, omaY, 0, kaytetty, eiTyhjat, lahinVastustaja, maxTspSyvyys);
      } else {
        //std::cerr  << "eiTyhjat " << eiTyhjat.size() << ", " << "maxSyvyys " << maxSyvyys << std::endl;
        //kohde = haeKohde(omaX, omaY, 0, kaytetty, eiTyhjat, lahinVastustaja, maxSyvyys);

        float arvo = 0;
        for (const auto &ruutu : eiTyhjat) {
          float a = haeArvo(omaX, omaY, ruutu, suunnat, peli.lauta, lahinVastustaja, hakuCache, maxSyvyys);
          if (a > arvo) {
            arvo = a;
            kohde = ruutu;
          }
        }
        //std::cerr << kohde.x << " " << kohde.y << " " << kohde.arvo << std::endl;
        //tulostaHeuristiikka(hakuCache, std::cerr);
      }


      Ruutu reitti = kohde;
      char siirto = 'w';
      while (reitti.x != omaX || reitti.y != omaY) {
        const Siirto &suunta = suunnat(reitti.x, reitti.y);
        siirto = suunta.vastamerkki();
        reitti = {
          (reitti.x + suunta.dx + leveys) % leveys,
          (reitti.y + suunta.dy + korkeus) % korkeus,
          0
        };
      }
      return siirto;
    } else {
      kohde = eiTyhjat[0];

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
    }

    assert(false);
    return '\0';
  }
};
}

std::unique_ptr<Aly> luoSuoratReitit(const Peli &peli, int maxSyvyys) {
  return std::unique_ptr<Aly>(new Toteutus(peli.pelaajat.size(), maxSyvyys));
}
