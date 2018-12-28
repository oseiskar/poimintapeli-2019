#ifndef MAIN_HPP
#define MAIN_HPP

#include <vector>
#include <memory>
#include <cassert>

// Laudan koko.
constexpr int leveys = 25, korkeus = 25;

// Yhden pelaajan tiedot.
struct Pelaaja {
  int x = leveys >> 1, y = korkeus >> 1, pisteet = 0;
  bool poissa = false;
};

template <class T> class Lauta {
private:
  std::vector<T> luvut;

public:
  // jos meneee yli laidasta, jatka vastakkaisesta laidasta
  T torus(int x, int y) const {
    x = (x + leveys*2) % leveys;
    y = (y + korkeus*2) % korkeus;
    return luvut[leveys*y + x];
  }

  T operator()(int x, int y) const {
    return luvut[leveys*y + x];
  }

  T& operator()(int x, int y) {
    return luvut[leveys*y + x];
  }

  T& operator()(int i) {
    return luvut[i];
  }

  T operator()(int i) const {
    return luvut[i];
  }

  Lauta() : luvut(leveys*korkeus) {}
};

struct Peli {
  std::vector<Pelaaja> pelaajat;
  Lauta<char> lauta;
  int lukusumma = 0;

  Peli(int pelaajia) : pelaajat(pelaajia) {}

  void alustaRuutu(int x, int y, char luku) {
    lauta(x,y) = luku;
    lukusumma += (int)luku;
  }

  void liikuta(Pelaaja &p, char l) {
    if (p.poissa) {
      return;
    } else if (l == 'w') {
      p.y = (p.y + korkeus - 1) % korkeus;
    } else if (l == 'a') {
      p.x = (p.x + leveys - 1) % leveys;
    } else if (l == 's') {
      p.y = (p.y + 1) % korkeus;
    } else if (l == 'd') {
      p.x = (p.x + 1) % leveys;
    } else {
      p.poissa = true;
      return;
    }
    p.pisteet += lauta(p.x,p.y);
  }

  void nollaaRuudut() {
    for (Pelaaja& p: pelaajat) {
      if (p.poissa) {
        continue;
      }
      lukusumma -= (int)lauta(p.x, p.y);
      lauta(p.x, p.y) = 0;
    }
  }
};
struct Siirto {
  char merkki;
  int dx;
  int dy;

  char vastamerkki() const {
    if (merkki == 'w') return 's';
    if (merkki == 'a') return 'd';
    if (merkki == 's') return 'w';
    if (merkki == 'd') return 'a';
    assert(false);
    return '\0';
  }
};

constexpr Siirto siirrot[4] = {
  { 'w', 0, -1 },
  { 'a', -1, 0 },
  { 's', 0, 1 },
  { 'd', 1, 0 }
};

class Aly {
public:
  virtual char siirto(const Peli &peli) = 0;
  virtual ~Aly() {}
};

std::unique_ptr<Aly> teeAly(const Peli &peli);

#endif
