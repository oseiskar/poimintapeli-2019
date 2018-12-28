#include <iostream>
#include <string>

#include "tila.hpp"

int main() {
  // Alustetaan kaikki pelaajat.
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
