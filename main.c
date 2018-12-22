// C

#include <stdio.h>

// Laudan koko.
const int leveys = 25, korkeus = 25;

// Pelaajien maksimimäärä.
const int MAX_PELAAJIA = 64;

// Yhden pelaajan tiedot.
struct Pelaaja {
	int x, y, pisteet, poissa;
};

int main() {
	// Alustetaan kaikki pelaajat.
	int pelaajia = 1;
	scanf("%d", &pelaajia);
	if (pelaajia <= 0 || pelaajia > MAX_PELAAJIA) {
		return 1;
	}
	struct Pelaaja pelaajat[MAX_PELAAJIA];
	for (int i = 0; i < pelaajia; ++i) {
		pelaajat[i].x = leveys >> 1;
		pelaajat[i].y = korkeus >> 1;
	}

	// Luetaan pelilauta ja lasketaan sen summa.
	int luvut[korkeus][leveys];
	int lukusumma = 0;
	for (int y = 0; y < korkeus; ++y) {
		char rivi[leveys+1];
		scanf("%s", rivi);
		for (int x = 0; x < leveys; ++x) {
			luvut[y][x] = rivi[x] - '0';
			lukusumma += luvut[y][x];
		}
	}

	// Tulostetaan oma nimi.
	puts("esim");
	fflush(stdout);

	// Tiedot esimerkin liikkumista (spiraalia) varten.
	int laajuus = 1, suunta = 0, vaihe = 0;

	// Pelataan kaikki kierrokset.
	while (lukusumma > 0) {
		// Muodostetaan spiraalia.
		char liikkeet[MAX_PELAAJIA + 1];
		char oma = liikkeet[0] = "wasd"[suunta];
		printf("%c\n", oma);
		fflush(stdout);
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

		// Luetaan vastustajien liikkeet.
		scanf("%s", &liikkeet[1]);

		// Jos rivi on vain viiva (-), peli on ohi.
		if (liikkeet[1] == '-') {
			break;
		}

		// Liikutaan ja jaetaan pisteet. Itse ollaan siis pelaaja 0.
		for (int i = 0; i < pelaajia; ++i) {
			struct Pelaaja* p = &pelaajat[i];
			char l = liikkeet[i];
			if (p->poissa) {
				continue;
			} else if (l == 'w') {
				p->y = (p->y + korkeus - 1) % korkeus;
			} else if (l == 'a') {
				p->x = (p->x + leveys - 1) % leveys;
			} else if (l == 's') {
				p->y = (p->y + 1) % korkeus;
			} else if (l == 'd') {
				p->x = (p->x + 1) % leveys;
			} else {
				p->poissa = 1;
				continue;
			}
			p->pisteet += luvut[p->y][p->x];
		}

		// Nollataan nykyiset ruudut.
		for (int i = 0; i < pelaajia; ++i) {
			struct Pelaaja* p = &pelaajat[i];
			if (p->poissa) {
				continue;
			}
			lukusumma -= luvut[p->y][p->x];
			luvut[p->y][p->x] = 0;
		}
	}
	return 0;
}
