#include <iostream>
#include <string>

#include "main.hpp"

constexpr const char * omaNimi = "aly";

int main() {
  // Alustetaan kaikki pelaajat.
  int pelaajia = 1;
  std::cin >> pelaajia;
  Peli peli(pelaajia);
  std::unique_ptr<Aly> aly(teeAly(peli));

  // Luetaan pelilauta ja lasketaan sen summa.
  for (int y = 0; y < korkeus; ++y) {
    std::string rivi;
    std::cin >> rivi;
    for (int x = 0; x < leveys; ++x) {
      peli.alustaRuutu(x,y, rivi[x] - '0');
    }
  }

  // Tulostetaan oma nimi.
  std::cout << omaNimi << std::endl;

  // Pelataan kaikki kierrokset.
  while (peli.lukusumma > 0) {
    char oma = aly->siirto(peli);
    std::cout << oma << std::endl;

    // Luetaan vastustajien liikkeet.
    std::string vastustajat;
    std::cin >> vastustajat;

    // Jos rivi on vain viiva (-), peli on ohi.
    if (vastustajat == "-") {
      break;
    }

    // Liikutaan ja jaetaan pisteet. Itse ollaan siis pelaaja 0.
    std::string liikkeet = oma + vastustajat;
    for (int i = 0; i < pelaajia; ++i) {
      peli.liikuta(peli.pelaajat[i], liikkeet[i]);
    }

    peli.nollaaRuudut();
  }
  return 0;
}
