#ifndef MAIN_HPP
#define MAIN_HPP

#include <vector>

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
  T operator()(int x, int y) const {
    return luvut[leveys*y + x];
  }

  T& operator()(int x, int y) {
    return luvut[leveys*y + x];
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

#include <memory>

class Aly {
public:
  virtual const char * nimi() const = 0;
  virtual char siirto(const Peli &peli) = 0;
  virtual ~Aly() {}
};

std::unique_ptr<Aly> teeAly(const Peli &peli);

#endif
