#include "main.hpp"

const char * alyNimi = "aly";

// Tiedot esimerkin liikkumista (spiraalia) varten.
int laajuus, suunta, vaihe;

void alustaAly(const Peli &peli) {
	laajuus = 1;
	suunta = 0;
	vaihe = 0;
}

char siirtoAly(const Peli &peli) {
	// Muodostetaan spiraalia.
	char oma = "wasd"[suunta];
	vaihe += 1;
	if (vaihe >= laajuus) {
		vaihe = 0;
		suunta += 1;
		if (suunta == 2) {
			laajuus += 1;
		} else if (suunta == 4) {
			laajuus += 1;
			suunta = 0;
		}
	}
	return oma;
}

