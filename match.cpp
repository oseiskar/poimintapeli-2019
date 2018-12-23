#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include "main.hpp"

#ifndef SHOW_MATCH
#define SHOW_MATCH 0
#endif

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

void tulosta(const Peli &peli, std::ostream &virta) {
  for (int y = 0; y < korkeus; ++y) {
    for (int x = 0; x < leveys; ++x) {
      const char arvo = peli.lauta(x,y);
      if (arvo == 0) {
        for (std::size_t i = 0; i < peli.pelaajat.size(); ++i) {
          if (peli.pelaajat[i].x == x && peli.pelaajat[i].y == y) {
            virta << (char)('A' + i);
            goto seuraavaRuutu;
          }
        }
        virta << ' ';
      } else {
        virta << (char)('0' + arvo);
      }
      seuraavaRuutu:;
    }
    virta << '\n';
  }
}

typedef std::pair< std::string, std::function< std::unique_ptr<Aly>(const Peli &) > > AlyGeneraattori;

int omaPisteEro(std::vector<int> pisteet) {
  int parasVastustaja = 0;
  for (std::size_t i = 1; i < pisteet.size(); ++i)
    parasVastustaja = std::max(parasVastustaja, pisteet[i]);

  return pisteet[0] - parasVastustaja;
}

int pelaa(int siemen, std::vector<AlyGeneraattori> generaattorit) {
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
  while (peli.lukusumma > 0) {
    for (int i = 0; i < pelaajia; ++i) {
      char siirto = alyt[i]->siirto(peli);
      Pelaaja pelaaja = peli.pelaajat[0];
      peli.liikuta(pelaaja, siirto);

      // siirrä seuraava pelaaja ensimmäiseksi listassa
      peli.pelaajat.erase(peli.pelaajat.begin());
      peli.pelaajat.push_back(pelaaja);
    }
    peli.nollaaRuudut();

#if SHOW_MATCH
    std::cerr << "\n";
    for (int i = 0; i < pelaajia; ++i) {
      std::cerr << generaattorit[i].first << ": " << peli.pelaajat[i].pisteet << "\n";
    }
    std::cerr << "-------------------------\n";
    tulosta(peli, std::cerr);
    std::cerr.flush();
    usleep(40000);
#endif
  }

  std::vector<int> pisteet;
  for (const auto &pelaaja : peli.pelaajat) {
    pisteet.push_back(pelaaja.pisteet);
#if !SHOW_MATCH
    std::cerr << pelaaja.pisteet << ' ';
#endif
  }

  int pisteEro = omaPisteEro(pisteet);
  std::cerr << "tulos: " << pisteEro << std::endl;
#if SHOW_MATCH
  usleep(2000000);
#endif

  return pisteEro;
}

// vastustajat
std::unique_ptr<Aly> luoGreedy(float parametri);
std::unique_ptr<Aly> luoAly(int maxSyvyys);

int main() {
  AlyGeneraattori oma = {
    "oma",
    [](const Peli &peli){ return teeAly(peli); }
  };
  std::vector< AlyGeneraattori > vastustajat = {
    {
      "greedy06",
      [](const Peli &peli){ return luoGreedy(0.6); }
    },
    {
      "greedy09",
      [](const Peli &peli){ return luoGreedy(0.9); }
    },
    {
      "aly7",
      [](const Peli &peli){ return luoAly(7); }
    },
    {
      "aly5",
      [](const Peli &peli){ return luoAly(5); }
    },
    {
      "aly3",
      [](const Peli &peli){ return luoAly(3); }
    }
  };

  int siemen0 = time(0);
  std::cout << "siemen: " << siemen0 << std::endl;

  for (int kierros = 0; kierros < 3; ++kierros) {
    int siemen = siemen0 + kierros;
    std::cout << "====== kierros " << (kierros+1) << " ======= " << std::endl;

    // kaksintaistelut
    constexpr std::size_t start = 2;
    for (std::size_t i = start; i < vastustajat.size(); ++i) {
      pelaa(siemen, { oma, vastustajat[i] });
    }

    if (vastustajat.size() > 1) {
      // kaikki vastaan kaikki
      std::vector< AlyGeneraattori > kaikki;
      kaikki.push_back(oma);
      kaikki.insert(kaikki.end(), vastustajat.begin(), vastustajat.end());
      pelaa(siemen, kaikki);
    }
  }
}
