#include "main.hpp"
#include <cmath>
//#include "tulostus.hpp"

namespace {
struct Ruutu {
  int x;
  int y;
  float arvo;
};

/** Lyhin etäisyys ruutujen (x0,y0) ja (x1,y1) välillä */
int lyhinEtaisyys(int x0, int y0, int x1, int y1) {
  int dx = std::abs(x1 - x0);
  int dy = std::abs(y1 - y0);

  dx = std::min(dx, leveys - dx);
  dy = std::min(dy, korkeus - dy);
  return dx + dy;
}


/** Laskee etäisyydet ruudusta (x0,y0) kaikkiin muihin ruutuihin */
void laskeEtaisyydet(int x0, int y0, Lauta<int> &etaisyydet) {
  for (int y = 0; y < korkeus; ++y) {
    for (int x = 0; x < leveys; ++x) {
      etaisyydet(x,y) = lyhinEtaisyys(x0,y0,x,y);
    }
  }
}

/**
 * Heuristiikka: jokainen (ei-tyhjä) ruutu x luo ympärilleen "potentiaalin",
 * jonka muoto on laskettu valmiiksi matriisiin M. Ruudun y kokonaispotentiaali
 * lasketaan summaamalla kaikkien ruutujen siihen aiheuttamat potentiaalit:
 *
 *    p(y) = summa_x M(x,y) * v(x)
 *
 * missä v(x) on ruudun x arvo.
 */
void laskePotentiaali(const std::vector< std::vector<float> > &matriisi, const Lauta<char> luvut, Lauta<float> &kentta) {
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

/**
 * Määrittää potentiaalin muodon (ks. laskePotentiaali)
 *
 *    M(x,y) = d(x,y)^a
 *
 * missä d on ruutujen x ja y lyhin etäisyys ja a on vakio (vedetty hatusta)
 */
void laskePotentiaalimatriisi(std::vector< std::vector<float> > &matriisi) {
  matriisi.clear();
  matriisi.reserve(korkeus*leveys);

  for (int y0 = 0; y0 < korkeus; ++y0) {
    for (int x0 = 0; x0 < leveys; ++x0) {
      matriisi.push_back({});
      std::vector< float > &rivi = matriisi.back();
      rivi.reserve(korkeus*leveys);

      for (int y1 = 0; y1 < korkeus; ++y1) {
        for (int x1 = 0; x1 < leveys; ++x1) {
          constexpr float kerroin = 0.7;
          const int d = lyhinEtaisyys(x0,y0,x1,y1);
          rivi.push_back(std::pow(kerroin, d));
        }
      }
    }
  }
}

/**
 * Hakualgoritmi 1: Laske kaikki omat siirrot syvyyteen maxSyvyys asti ja
 * valitse paras siirtosarja sen mukaan, kuinka paljon pisteitä sen varrella
 * on, kuinka todennäköisesti jokin vastustaja ehtii missäkin kohtaa väliin
 * ja kuinka hyvä "potentiaali" (heuristiikka) sarjan viimeisellä ruudulla on.
 *
 * @param x lähtöpisteen x-koordinaatti
 * @param y lähtöposteen y-koordinaatti
 * @param t lähtöaika (käytetään määrittämään vastustajien mahdollisia paikkoja)
 * @param potentiaali heuristiikka jota käytetään reitin parhaan loppuruudun
 *  valitsemiseen niiden ruutujen väliltä, joissa ei ole pisteitä
 * @param lahinVastustaja lähimmän vastustajan etäisyys kussakin pisteessä
 *  ajanhetkellä t = 0
 * @param vastustajakerroin arvioi todennäköisyyttä että ruudussa olevat
 *  pisteet ovat vielä keräämättä jos oma pelaaja ehtii sinne pahimmillaan
 *  vasta lähimmän vastustajan jälkeen
 * @param maxSyvyys kuinka syvälle omia siirtoja lasketaan
 * @return ensimmäinen parhaalla reitillä oleva ei-tyhjä ruutu, tai reitin
 *  viimeinen ruutu, jos missään parhaan reitin ruudussa ei ole pisteitä
 */
Ruutu haeKohde(int x, int y, int t, Lauta<char> &luvut,
  const Lauta<float> &potentiaali,
  const Lauta<int> &lahinVastustaja,
  const Lauta<float> &vastustajakerroin,
  int maxSyvyys)
{
  const char vanha = luvut(x,y);
  // suosi siirtosarjoja, joissa pisteet kerätään mahdollisimman aikaisin
  constexpr float diskonttauspaino = 0.05;
  // kuinka painottaa viimeisen ruudun potentiaalia verrattuna siirtosarjan
  // keräämien pisteiden määrään
  constexpr float potentiaalipaino = 0.2;

  float tamaArvo = vanha * (1 + maxSyvyys * diskonttauspaino);
  if (lahinVastustaja(x,y) < t) {
    tamaArvo *= vastustajakerroin(x,y);
  }

  Ruutu paras = { 0, 0, 0 };
  if (maxSyvyys == 0) {
    paras = { x, y, potentiaali(x,y) * potentiaalipaino };
  }
  else {
    luvut(x,y) = 0;
    for (const auto &siirto : siirrot) {
      Ruutu kohde = haeKohde(
        ((x + siirto.dx) + leveys) % leveys,
        ((y + siirto.dy) + korkeus) % korkeus,
        t + 1,
        luvut, potentiaali, lahinVastustaja, vastustajakerroin, maxSyvyys-1);
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

/**
 * Hakualgoritmi 2: Valitse jäljellä olevista ruuduista N parasta ja järjestys
 * jossa ne kannattaa yrittää kerätä, jotta pistemäärä maksimoituu.
 *
 * @param x lähtöpisteen x-koordinaatti
 * @param y lähtöposteen y-koordinaatti
 * @param t lähtöaika (käytetään määrittämään vastustajien mahdollisia paikkoja)
 * @param eiTyhjat lista ei-tyhjistä ruuduista
 * @param kaytetty hakualgoritmin työmuisti: onko ruutu jo käytetty?
 * @param lahinVastustaja lähimmän vastustajan etäisyys kussakin pisteessä
 *  ajanhetkellä t = 0
 * @param vastustajakerroin arvioi todennäköisyyttä että ruudussa olevat
 *  pisteet ovat vielä keräämättä jos oma pelaaja ehtii sinne pahimmillaan
 *  vasta lähimmän vastustajan jälkeen
 * @param maxSyvyys N: kuinka monta parasta ruutua maksimissaan yritetään kerätä
 * @return parhaan reitin ensimmäinen ei-tyhjä ruutu
 */
Ruutu haeKohdeTsp(int x, int y, int t,
  const std::vector<Ruutu> &eiTyhjat,
  Lauta<char> &kaytetty,
  const Lauta<int> &lahinVastustaja,
  const Lauta<float> &vastustajakerroin,
  int maxSyvyys)
{
  assert(maxSyvyys > 0);

  const Ruutu *paras = nullptr;
  float parasArvo = -1;
  // suosi siirtosarjoja, joissa pisteet kerätään mahdollisimman aikaisin
  float diskonttauspaino = 0.1;

  for (const Ruutu &kohde : eiTyhjat) {
    if (kaytetty(kohde.x, kohde.y) > 0) continue;

    int etaisyys = lyhinEtaisyys(x, y,  kohde.x, kohde.y);
    int vastustajanAikaKohteessa = etaisyys - lahinVastustaja(kohde.x, kohde.y) + t;
    float aikakerroin = 1.0 / (1.0 + t * diskonttauspaino);
    if (vastustajanAikaKohteessa > 0) {
      aikakerroin *= 1.0 / (1 + vastustajanAikaKohteessa) * vastustajakerroin(kohde.x, kohde.y);
    }
    if (vastustajanAikaKohteessa < 0) {
      aikakerroin *= 1.05;
    }

    float arvo = kohde.arvo * aikakerroin;
    if (maxSyvyys > 1) {
      kaytetty(kohde.x, kohde.y) = 1;
      arvo += haeKohdeTsp(kohde.x, kohde.y, t+etaisyys, eiTyhjat, kaytetty,
        lahinVastustaja, vastustajakerroin, maxSyvyys-1).arvo;
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
  // allokoidaan kaikki työmuisti valmiiksi alussa, mikä nopeuttaa ohjelmaa
  std::vector<Ruutu> eiTyhjat;
  Lauta<int> lahinVastustaja;
  Lauta<float> vastustajakerroin;
  std::vector< std::vector<float> > potentiaalimatriisi;
  Lauta<float> potentiaali;
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
    laskePotentiaalimatriisi(potentiaalimatriisi);
    eiTyhjat.reserve(leveys*korkeus);
  }
  ~Toteutus() {}

  char siirto(const Peli &peli) final {
    const int omaX = peli.pelaajat[0].x;
    const int omaY = peli.pelaajat[0].y;

    // laske lähimmän vastustajan etäisyys ja "vastustajakerroin", joka
    // arvioi (hyvin karkeasti) todennäköisyyttä, että jonkin ruudun pisteet
    // on kerätty ennen sinne saapumista jos lähin vastustaja on voinut
    // periaattessa ehtiä sinne ensin
    for (std::size_t i = 1; i < peli.pelaajat.size(); ++i) {
      const auto &p = peli.pelaajat[i];
      laskeEtaisyydet(p.x, p.y, etaisyydet);
      for (int j=0; j<leveys*korkeus; ++j) {
        if (i == 1) vastustajakerroin(j) = 1.0;
        int e = etaisyydet(j);
        if (e > 0) {
          // mitä kauempana lähin vastustaja on, sitä todennäköisemmin tämä
          // ei kerää ruutua ennen omaa pelaajaa, koska välissä on luultavasti
          // muita ruutuja, joita vastustaja voi kerätä ja tämä hidastaa matkaa
          const float sakko = 1.0 - 0.9 / e;
          vastustajakerroin(j) *= sakko;
        }
        if (i == 1 || e < lahinVastustaja(j)) {
          lahinVastustaja(j) = e;
        }
      }
    }

    // muodosta lista ei-tyhjistä ruuduista. Jos niitä on riittävän vähän,
    // käytetään loppupelin hakualgoritmia 2
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
      // hakualgoritmi 2: loppupeli
      if (eiTyhjat.size() > 1) {
        constexpr int maxTspSyvyys = 4;
        uusiKohde = haeKohdeTsp(omaX, omaY, 0, eiTyhjat, kaytetty,
          lahinVastustaja, vastustajakerroin, maxTspSyvyys);
      } else {
        uusiKohde = eiTyhjat[0];
      }
    } else {
      // hakualgoritmi 1
      // työmuisti
      hakuCache = peli.lauta;

      laskeEtaisyydet(omaX, omaY, etaisyydet);
      constexpr int koko = leveys*korkeus;
      for (int i = 0; i < koko; ++i) {
        if (etaisyydet(i) < maxSyvyys + 1) {
          hakuCache(i) = 0;
        }
      }

      laskePotentiaali(potentiaalimatriisi, hakuCache, potentiaali);
      //tulostaHeuristiikka(potentiaali, std::cerr, 200);

      hakuCache = peli.lauta;
      uusiKohde = haeKohde(omaX, omaY, 0, hakuCache, potentiaali,
        lahinVastustaja, vastustajakerroin, maxSyvyys+1);
    }

    // Kohdetta, eli seuraavaa ei-tyhjää ruutua suunnitellulla reitillä ei
    // vaihdeta ellei paremmalta vaikuttava kohde ole lähempänä tai vanha
    // kohde jo kerätty (tai vanha kohde on muusta syystä oikeasti tyhjä ruutu).
    //
    // Tämä estää tilanteen, jossa oma pelaaja jää jumiin kahden tai useamman
    // ruudun silmukkaan, koska lupaavimman näköinen kohde "hyppii" eri vaihto-
    // ehtojen välillä, riippuen siitä, missä ruudussa ollaan
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

    // Kohde voi olla usammankin ruudun päässä, määritetään mahdolliset
    // suunnat, jotka johtavat lähemmäksi kohdetta...
    int dx = kohde.x - omaX;
    int dy = kohde.y - omaY;
    if (std::abs(dx) > leveys - std::abs(dx)) dx = -dx;
    if (std::abs(dy) > korkeus - std::abs(dy)) dy = -dy;

    // ... ja valitaaan niistä, se, jolla on paras potentiaali. Tästä voi
    // ainakin teoriassa olla hyötyä, jos joku muu ehtii kohteeseen ensin:
    // Tämä voi auttaa valitsemaan monista reittivaihtoehdoista sen, joka
    // kulkee lähimpää muita kohteita.
    char omaSiirto = '\0';
    float parasArvo = -1000;
    for (const auto &siirto : siirrot) {
      if (oikeaSuunta(dx, siirto.dx) && oikeaSuunta(dy, siirto.dy)) {
        const int x = omaX + siirto.dx;
        const int y = omaY + siirto.dy;
        const float arvo = potentiaali.torus(x,y);
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

// voi käyttää luomaan eri parametreilla toimivia koevastustajia
std::unique_ptr<Aly> luoAly(const Peli &peli, int maxSyvyys, unsigned tspKynnys) {
  return std::unique_ptr<Aly>(new Toteutus(maxSyvyys, tspKynnys));
}

// varsinainen kisaversio
std::unique_ptr<Aly> teeAly(const Peli &peli) {
  return std::unique_ptr<Aly>(new Toteutus());
}
