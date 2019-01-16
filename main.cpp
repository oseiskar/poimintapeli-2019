/*
 * Kisateköälyn päätiedosto. Lukee/kirjoittaa stdin/stdout-virtoja
 * kisassa määritellyn protokollan mukaisesti. Pelin alkutila ja vastustajien
 * siirrot luetaan stdin:stä ja omat siirrot (ja nimi) kirjoitetaan stdout-
 * virtaan.
 */
#include <iostream>
#include <string>

#include "tila.hpp"

int main() {
  Tilakone tila;
  std::string syote;
  while (true) {
    if (!std::getline(std::cin, syote)) break;
    const std::string tuloste = tila.kasitteleRivi(syote);
    if (tuloste == "LOPPU") break;
    if (!tuloste.empty()) {
      std::cout << tuloste << std::endl;
    }
  }
  return 0;
}
