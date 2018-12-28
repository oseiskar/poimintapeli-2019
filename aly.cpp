#include "main.hpp"

// FWD decl
std::unique_ptr<Aly> luoSuoratReitit(const Peli &peli, int maxSyvyys);

std::unique_ptr<Aly> teeAly(const Peli &peli) {
  return luoSuoratReitit(peli, 4);
}
