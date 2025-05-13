# SO---secret-
Proiectul la Sisteme de Operare -- flood
Tanase Elena Alexandra - 3.2C

ADDITIONAL MENTIONS:

1. TREASURE_MANAGER PHASE 1
  -> fisierul de treasure este binar (.dat) iar fisierul de log este text (.txt)
  -> id-ul comorilor se presupune a fi unic mereu, nu verificam asta, it's a known fact! Also, nu poate fi niciodata 0, only >0
  -> latitudinea si longitudinea nu pot fi 0 si pot primi doar valorile corecte dpdv geografic :>
  -> id-ul pt comorile de vazut si de sters le dam tot ca argumente in linie de comanda, se presupune din start ca sunt valide (not a string or something else)
  -> comorile le citim manual de la tastatura, nu folosim un file aparte pentru ele
  -> se presupune ca nu introducem string-uri mai lungi decat macro-ul respectiv pentru fiecare (userName and clueText)

2. TREASURE_HUB PHASE 2:
   -> folosesc un fisier text prin intermediul caruia fac legatura intre procesul parinte si procesul copil
