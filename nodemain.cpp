#include <emscripten/bind.h>
#include "tila.hpp"

std::unique_ptr<Tilakone> alusta() {
  return std::unique_ptr<Tilakone>(new Tilakone());
}

EMSCRIPTEN_BINDINGS(tilakone_api) {
    emscripten::function("alusta", &alusta);
    emscripten::class_<Tilakone>("Tilakone")
      .function("kasitteleRivi", &Tilakone::kasitteleRivi);
}
