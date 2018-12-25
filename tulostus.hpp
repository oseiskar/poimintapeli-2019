#ifndef TULOSTUS_HPP
#define TULOSTUS_HPP

#include "main.hpp"
#include <iostream>
#include <iomanip>

namespace {

template <class T>
void tulostaHeuristiikka(const Lauta<T> &heuristiikka, std::ostream &s, int kerroin = 1) {
    for (int y = 0; y < korkeus; ++y) {
      for (int x = 0; x < leveys; ++x) {
        s
          << std::setw(3) << std::setfill('0')
          << std::min((int)std::floor(heuristiikka(x,y) * kerroin), 999)
          << ' ';
      }
      s << std::endl;
    }
}

void tulostaPeli(const Peli &peli, std::ostream &virta) {
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

}

#endif
