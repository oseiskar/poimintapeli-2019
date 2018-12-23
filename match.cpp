#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>
#include "main.hpp"

// vastustajat
std::unique_ptr<Aly> luoGreedy(float parametri);

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

std::vector<int> pelaa(int siemen, std::vector<AlyGeneraattori> generaattorit) {
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
    std::cerr << pelaaja.pisteet << ' ';
  }
  std::cerr << std::endl;

  return pisteet;
}

int omaPisteEro(std::vector<int> pisteet) {
  int parasVastustaja = 0;
  for (std::size_t i = 1; i < pisteet.size(); ++i)
    parasVastustaja = std::max(parasVastustaja, pisteet[i]);

  return pisteet[0] - parasVastustaja;
}

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
    }
  };

  int siemen = 0;

  // kaksintaistelut
  for (auto &vastustaja : vastustajat) {
    int pisteEro = omaPisteEro(pelaa(siemen, { oma, vastustaja }));
    std::cout << pisteEro << std::endl;
  }

  if (vastustajat.size() > 1) {
    // kaikki vastaan kaikki
    std::vector< AlyGeneraattori > kaikki;
    kaikki.push_back(oma);
    kaikki.insert(kaikki.end(), vastustajat.begin(), vastustajat.end());
    int pisteEro = omaPisteEro(pelaa(siemen, kaikki));
    std::cout << pisteEro << std::endl;
  }
}
