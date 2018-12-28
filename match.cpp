#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <cassert>
#include "main.hpp"
#include "tulostus.hpp"

#ifndef SHOW_MATCH
#define SHOW_MATCH 0
#endif

//#define SHOW_KYNNYS 30
#define SHOW_KYNNYS 1000000

#if SHOW_MATCH
#include <unistd.h>
#endif

void generoi(int siemen, Lauta<char> &lauta) {
  // from https://www.ohjelmointiputka.net/kilpailut/2018-poimintapeli/
  int xorshift32_state = 0xdeadbeef ^ siemen;
  for (int y = 0; y < korkeus; ++y) {
    for (int x = 0; x < leveys; ++x) {
      int rnd;
      {
        int x = xorshift32_state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        xorshift32_state = x;
        rnd = (x >> 12) & 0xffff;
      }
      lauta(x,y) = std::floor(std::pow(rnd % 100, 3) / 1e5);
    }
  }
  lauta(leveys >> 1, korkeus >> 1) = 0;
}

void generoiLauta(int siemen, Peli &peli) {
  Lauta<char> lauta;
  generoi(siemen, lauta);
  for (int y = 0; y < korkeus; ++y) {
    for (int x = 0; x < leveys; ++x) {
      peli.alustaRuutu(x,y, lauta(x,y));
    }
  }
}

typedef std::pair< std::string, std::function< std::unique_ptr<Aly>(const Peli &) > > AlyGeneraattori;

struct Tulos {
  int omaPisteEro;
  int voittaja;
};

Tulos laskeTulos(std::vector<int> pisteet) {
  int vastustajanParhaatPisteet = 0;
  int parasVastustaja = 0;
  for (std::size_t i = 1; i < pisteet.size(); ++i) {
    if (pisteet[i] > vastustajanParhaatPisteet) {
      parasVastustaja = i;
      vastustajanParhaatPisteet = pisteet[i];
    }
  }

  Tulos tulos;
  tulos.omaPisteEro = pisteet[0] - vastustajanParhaatPisteet;
  if (tulos.omaPisteEro >= 0) {
    tulos.voittaja = 0;
  } else {
    tulos.voittaja = parasVastustaja;
  }

  return tulos;
}

Tulos pelaa(int siemen, std::vector<AlyGeneraattori> generaattorit) {
  int pelaajia = generaattorit.size();
  Peli peli(pelaajia);
  generoiLauta(siemen, peli);
  std::vector< std::unique_ptr<Aly> > alyt;
  for (auto &f : generaattorit) {
    alyt.emplace_back(f.second(peli));
    std::cerr << f.first << " ";
  }
  std::cerr << std::endl;

  // Pelataan kaikki kierrokset.
  unsigned pisteettomat = 0;
  int edellinenLukusumma = peli.lukusumma;
  constexpr unsigned maxPisteettomat = 100;
  while (peli.lukusumma > 0 && pisteettomat < maxPisteettomat) {
    for (int i = 0; i < pelaajia; ++i) {
      char siirto = alyt[i]->siirto(peli);
      Pelaaja pelaaja = peli.pelaajat[0];
      peli.liikuta(pelaaja, siirto);

      // siirrä seuraava pelaaja ensimmäiseksi listassa
      peli.pelaajat.erase(peli.pelaajat.begin());
      peli.pelaajat.push_back(pelaaja);
    }
    peli.nollaaRuudut();

    if (peli.lukusumma == edellinenLukusumma) {
      pisteettomat++;
    } else {
      pisteettomat = 0;
    }
    edellinenLukusumma = peli.lukusumma;

#if SHOW_MATCH
    if (peli.lukusumma < SHOW_KYNNYS) {
      std::cerr << "\n";
      for (int i = 0; i < pelaajia; ++i) {
        std::cerr << generaattorit[i].first << ": " << peli.pelaajat[i].pisteet << "\n";
      }
      std::cerr << "-------------------------\n";
      tulostaPeli(peli, std::cerr);
      std::cerr.flush();
      usleep(60000);
    }
#endif
  }

  if (pisteettomat == maxPisteettomat) {
    assert(false && "Peli keskeytettiin!");
  }

  std::vector<int> pisteet;
  for (const auto &pelaaja : peli.pelaajat) {
    pisteet.push_back(pelaaja.pisteet);
#if !SHOW_MATCH
    std::cerr << pelaaja.pisteet << ' ';
#endif
  }

  const Tulos tulos = laskeTulos(pisteet);
  std::cerr
    << "voittaja: " << generaattorit[tulos.voittaja].first
    << ", oma piste-ero: " << tulos.omaPisteEro << std::endl;
#if SHOW_MATCH
  usleep(2000000);
#endif

  return tulos;
}

// vastustajat
std::unique_ptr<Aly> luoGreedy(float parametri);
std::unique_ptr<Aly> luoAly(const Peli &peli, int maxSyvyys, unsigned tspKynnys);
std::unique_ptr<Aly> luoEiHuomVast(int maxSyvyys);
std::unique_ptr<Aly> luoSuoratReitit(const Peli &peli, int maxSyvyys);
std::unique_ptr<Aly> luoVaistaVastustajia(const Peli &peli, int maxSyvyys);

int main() {
  AlyGeneraattori oma = {
    "oma",
    [](const Peli &peli){ return teeAly(peli); }
  };
  std::vector< AlyGeneraattori > vastustajat = {
    {
      "vaistaVast6",
      [](const Peli &peli){ return luoVaistaVastustajia(peli, 6); }
    },
    {
      "suoratReitit4",
      [](const Peli &peli){ return luoSuoratReitit(peli, 4); }
    },
    {
      "alyTsp5",
      [](const Peli &peli){ return luoAly(peli, 8, 5); }
    },
    {
      "vaistaVast8",
      [](const Peli &peli){ return luoVaistaVastustajia(peli, 6); }
    },
    {
      "aly6Tsp10",
      [](const Peli &peli){ return luoAly(peli, 6, 10); }
    },
    {
      "eiHuomVast6",
      [](const Peli &peli){ return luoEiHuomVast(6); }
    },
    {
      "eiHuomVast8",
      [](const Peli &peli){ return luoEiHuomVast(8); }
    },
    {
      "greedy06",
      [](const Peli &peli){ return luoGreedy(0.6); }
    }
  };

  std::vector< AlyGeneraattori > kaikki;
  kaikki.push_back(oma);
  kaikki.insert(kaikki.end(), vastustajat.begin(), vastustajat.end());

  int siemen0 = time(0);
  std::cout << "siemen: " << siemen0 << std::endl;

  std::vector<int> kaksintaisteluvoitot;
  std::map<int, int> monipelaajavoitot;
  for (std::size_t i = 0; i < vastustajat.size(); ++i) {
    monipelaajavoitot[i] = 0;
  }

  constexpr int nKierrokset = 200;
  for (int kierros = 0; kierros < nKierrokset; ++kierros) {
    int siemen = siemen0 + kierros;
    std::cout << "====== kierros " << (kierros+1) << " ======= "
      << "siemen: " << siemen << std::endl;
    std::vector<int> kierroksenVoitot;

    // kaksintaistelut
    constexpr std::size_t start = 0;
    for (std::size_t i = start; i < vastustajat.size(); ++i) {
      const Tulos tulos = pelaa(siemen, { oma, vastustajat[i] });
      kierroksenVoitot.push_back(tulos.voittaja == 0);
    }

    if (vastustajat.size() > 1) {
      // kaikki vastaan kaikki
      const Tulos tulos = pelaa(siemen, kaikki);
      monipelaajavoitot[tulos.voittaja] += 1;
    }

    if (kaksintaisteluvoitot.size() == 0) kaksintaisteluvoitot = kierroksenVoitot;
    else for (std::size_t i = 0; i < kaksintaisteluvoitot.size(); ++i) {
      kaksintaisteluvoitot[i] += kierroksenVoitot[i];
    }
  }

  std::cout << "--------------" << std::endl;
  for (int i : kaksintaisteluvoitot)
    std::cout << i / float(nKierrokset) << std::endl;

  std::cout << "--------------" << std::endl;
  for (const auto &itr : monipelaajavoitot) {
    std::cout << kaikki[itr.first].first << ": " << itr.second << std::endl;
  }
}
