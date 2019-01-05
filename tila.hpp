#ifndef TILA_HPP
#define TILA_HPP
#include "main.hpp"
#include "nimi.hpp"

/**
 * Pelin kommunikaatioprotokolla muunnettuna tilakoneeksi. Helpottaa lähinnä
 * Node.js-komentoriviohjelman tekemistä.
 */
class Tilakone {
public:
  /**
   * Lue yksi syöterivi (parametrina) ja palauta oma tulosterivi (tai tyhjä
   * jos ei tulosteta mitään)
   */
  std::string kasitteleRivi(std::string rivi) {
    if (!peli) {
      const int pelaajia = std::stoi(rivi);
      peli = std::unique_ptr<Peli>(new Peli(pelaajia));
      aly = teeAly(*peli);
      laudanRiviNo = 0;
      omaSiirto = 0;
      return omaNimi;
    }

    if (laudanRiviNo < korkeus) {
      for (int x = 0; x < leveys; ++x) {
        peli->alustaRuutu(x,laudanRiviNo, rivi[x] - '0');
      }
      laudanRiviNo++;
      if (laudanRiviNo < korkeus) {
        return "";
      } else {
        omaSiirto = aly->siirto(*peli);
        return std::string() + omaSiirto;
      }
    }

    if (peli->lukusumma == 0 || rivi == "-") return "LOPPU";

    // edellinen siirto
    std::string liikkeet = omaSiirto + rivi;
    for (std::size_t i = 0; i < peli->pelaajat.size(); ++i) {
      peli->liikuta(peli->pelaajat[i], liikkeet[i]);
    }
    peli->nollaaRuudut();

    omaSiirto = aly->siirto(*peli);
    return std::string() + omaSiirto;
  }

private:
  std::unique_ptr<Peli> peli;
  std::unique_ptr<Aly> aly;
  int laudanRiviNo;
  char omaSiirto;
};

#endif
