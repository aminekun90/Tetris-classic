#!/usr/bin/env python3
"""Harnais de test par ecran de reference pour Tetris-classic.

Le jeu n'a pas de fonctions pures a tester : le rendu et la logique sont
soudes, et il relit meme l'ecran pour faire descendre les lignes. La seule
verification qui ait du sens est donc l'ecran lui-meme.

Deux familles de verifications, parce que tout n'est pas reproductible :

  * Ecrans de reference (exacts) — menus et ecran des scores. Ils ne
    dependent que des touches, donc la comparaison peut etre au caractere
    pres, dans tests/golden/.

  * Invariants de structure — pendant une partie. La gravite depend de
    l'horloge : deux executions ne donnent jamais le meme ecran. On verifie
    donc ce qui doit rester vrai quoi qu'il arrive : le cadre est ferme,
    rien ne deborde du plateau, les libelles du panneau sont la, et aucune
    notation caret n'est emise. Ce sont exactement les trois bugs de rendu
    rencontres pendant le portage.

    ./tests/play.py            verifie
    ./tests/play.py --record   regenere les ecrans de reference
"""
import os, pty, select, signal, sys, time
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent
BINAIRE = RACINE / "build" / "Tetris"
GOLDEN = Path(__file__).resolve().parent / "golden"
DUMP = "/tmp/tetris_test_screen.txt"

HAUT, BAS, GAUCHE, DROITE, ENTREE = b'\x1b[A', b'\x1b[B', b'\x1b[D', b'\x1b[C', b'\r'

# Ecrans deterministes : nom, graine, touches, attente
SCENARIOS = [
    ("menu",        1, [],      1.0),
    ("menu_scores", 1, [BAS],   0.8),
]

# Parties : nom, graine, touches, attente. Verifiees par invariants.
PARTIES = [
    ("debut",    42, [ENTREE],                             1.2),
    ("gauche",   42, [ENTREE] + [GAUCHE]*4,                1.2),
    ("rotation", 42, [ENTREE] + [HAUT]*3 + [DROITE]*2,     1.2),
    ("chute",    42, [ENTREE] + [BAS]*25,                  1.5),
    ("melange",  7,  [ENTREE] + [GAUCHE, HAUT, DROITE, BAS]*8, 2.0),
]

# Le plateau occupe les colonnes 25 a 50 ; le panneau va jusqu'a 79.
BORD_GAUCHE, BORD_DROIT, DERNIERE_LIGNE = 25, 50, 29
LIBELLES = ("Tetris 1.4", "Score:", "Next:", "Pause", "Quiter")


def verifier_invariants(nom, grille, sortie_brute):
    """Retourne la liste des invariants violes."""
    problemes = []
    lignes = grille.split("\n")
    if not lignes or not lignes[0].startswith("buffer "):
        return [f"{nom} : dump illisible"]
    lignes = lignes[1:]

    if len(lignes) < DERNIERE_LIGNE + 1:
        return [f"{nom} : {len(lignes)} lignes, {DERNIERE_LIGNE + 1} attendues"]

    # Le cadre du plateau doit etre ferme sur ses quatre cotes.
    for y in range(DERNIERE_LIGNE + 1):
        ligne = lignes[y]
        for x in (BORD_GAUCHE, BORD_DROIT):
            if x < len(ligne) and ligne[x] != "#":
                problemes.append(f"{nom} : bordure trouee en ({x},{y}) — '{ligne[x]}'")
                break
        if problemes:
            break
    for y in (0, DERNIERE_LIGNE):
        segment = lignes[y][BORD_GAUCHE:BORD_DROIT + 1]
        if set(segment) != {"#"}:
            problemes.append(f"{nom} : bordure horizontale incomplete ligne {y}")
            break

    # Les libelles du panneau doivent etre presents.
    for libelle in LIBELLES:
        if libelle not in grille:
            problemes.append(f"{nom} : libelle manquant — {libelle}")

    # Aucune notation caret : c'etait le bug des pieces eclatees en « ^@ ».
    if "^@" in sortie_brute:
        problemes.append(f"{nom} : notation caret emise (NUL rendu en ^@)")

    # Les pieces sont dessinees par leur fond colore, pas par un glyphe :
    # le plateau doit donc contenir des cellules a fond. Sans cet invariant,
    # le bug ou WriteConsoleOutputCharacter ecrasait les attributs passait
    # inapercu — les pieces s'affichaient en caracteres nus sur fond noir.
    interieur = [ligne[BORD_GAUCHE + 1:BORD_DROIT]
                 for ligne in lignes[1:DERNIERE_LIGNE]]
    blocs = sum(l.count("#") for l in interieur)
    if blocs == 0:
        problemes.append(f"{nom} : aucune piece a fond colore dans le plateau")

    # Et rien ne doit sortir du plateau : entre la bordure droite et le
    # panneau, la zone doit rester vide.
    for y in range(1, DERNIERE_LIGNE):
        marge = lignes[y][BORD_DROIT + 1:BORD_DROIT + 2]
        if marge.strip() not in ("", "#"):
            problemes.append(f"{nom} : debordement hors du plateau ligne {y}")
            break

    return problemes


def jouer(graine, touches, attente):
    """Retourne (grille, sortie_brute) ou (None, sortie_brute)."""
    if os.path.exists(DUMP):
        os.remove(DUMP)
    pid, fd = pty.fork()
    if pid == 0:
        os.chdir(RACINE / "build")
        os.environ["TERM"] = "xterm-256color"
        os.environ["TETRIS_SEED"] = str(graine)
        os.environ["TETRIS_DUMP_SCREEN"] = DUMP
        os.execv(str(BINAIRE), [str(BINAIRE)])

    brut = bytearray()

    def vider(duree):
        fin = time.time() + duree
        while time.time() < fin:
            r, _, _ = select.select([fd], [], [], 0.05)
            if r:
                try:
                    brut.extend(os.read(fd, 65536))
                except OSError:
                    return

    vider(1.2)
    for t in touches:
        os.write(fd, t)
        vider(0.12)
    vider(attente)
    os.kill(pid, signal.SIGKILL)
    os.waitpid(pid, 0)

    sortie = brut.decode("utf-8", "replace")
    if not os.path.exists(DUMP):
        return None, sortie
    return Path(DUMP).read_text(), sortie


def main():
    record = "--record" in sys.argv
    GOLDEN.mkdir(exist_ok=True)
    if not BINAIRE.exists():
        print(f"binaire absent : {BINAIRE}\nlancer d'abord : cmake --build build")
        return 1

    echecs = 0
    print("ecrans de reference")
    for nom, graine, touches, attente in SCENARIOS:
        obtenu, _ = jouer(graine, touches, attente)
        ref = GOLDEN / f"{nom}.txt"
        if obtenu is None:
            print(f"  ECHEC   {nom} : aucun ecran capture")
            echecs += 1
            continue
        if record:
            ref.write_text(obtenu)
            print(f"  ecrit   {nom}")
            continue
        if not ref.exists():
            print(f"  ECHEC   {nom} : reference absente (lancer --record)")
            echecs += 1
            continue
        attendu = ref.read_text()
        if obtenu == attendu:
            print(f"  ok      {nom}")
        else:
            echecs += 1
            print(f"  ECHEC   {nom} : l'ecran a change")
            a, b = attendu.split("\n"), obtenu.split("\n")
            for i in range(max(len(a), len(b))):
                la = a[i] if i < len(a) else "<absente>"
                lb = b[i] if i < len(b) else "<absente>"
                if la != lb:
                    print(f"            ligne {i}\n            attendu |{la}|\n            obtenu  |{lb}|")
                    break

    if record:
        print("\nreferences regenerees")
        return 0

    print("\ninvariants pendant une partie")
    for nom, graine, touches, attente in PARTIES:
        grille, brut = jouer(graine, touches, attente)
        if grille is None:
            print(f"  ECHEC   {nom} : aucun ecran capture")
            echecs += 1
            continue
        problemes = verifier_invariants(nom, grille, brut)
        if problemes:
            echecs += len(problemes)
            for pb in problemes:
                print(f"  ECHEC   {pb}")
        else:
            print(f"  ok      {nom}")

    total = len(SCENARIOS) + len(PARTIES)
    print(f"\n{total} scenarios, {echecs} echec{'s' if echecs > 1 else ''}")
    return 1 if echecs else 0


if __name__ == "__main__":
    sys.exit(main())
