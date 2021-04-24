# Inšpirácie na riešenie komunikácie klient-server

## server

Napíšte program (server), ktorý vytvorý lokálny soket na ktorom bude počúvať
a čo prijme, to vypíše.

toto nie je skutocny server, len cast  
caka na spojenie a prijme spravu, neposiela vsak ziadnu odpoved

```c
int main(int ac, char **av, char **en) {
  int s, ns;
  char buff[64];                // buffer pre prijatu spravu
  struct sockaddr_un ad;        // adresa pre soket
  
  memset(&ad, 0, sizeof(ad));
  ad.sun_family = AF_LOCAL;
  strcpy(ad.sun_path, "./sck"); // adresa = meno soketu (rovnake ako pouziva klient)
  s = socket(PF_LOCAL, SOCK_STREAM, 0);
  if (s == -1)
  {
    perror("socket: ");
    exit(2);
  }
  
  unlink("./sck");                              // ak by tam uz taky bol
  bind(s, (struct sockaddr*)&ad, sizeof(ad));   // zviazat soket s lokalnou adresou
                                                // s je hlavny soket, len pocuva
  listen(s, 5);                      // pocuvat, najviac 5 spojeni naraz (v rade)
  
  ns = accept(s, NULL, NULL);        // prijat jedno spojenie (z max 5 cakajucich)
                                     // ns je novy soket pre konkretne spojenie s konkretnym klientom
  read(ns, &buff, sizeof(buff));     // blokuje citanie (cakanie na klienta)
  
  printf("Client sent: %s\n", buff); // vypisat co prislo
  close(ns);                         // tu to nie je nevyhnutne, kedze proces aj tak konci
  close(s);
}
```

---

## client

Napíšte program (klient), ktorý vytvorí spojenie so serverom a pošle mu reťazec.

```c
int main(int ac, char **av, char **en) {
  int s;
  char msg[64] = "hello world!";
  struct sockaddr_un ad;           // adresa pre soket
  
  memset(&ad, 0, sizeof(ad));
  ad.sun_family = AF_LOCAL;
  strcpy(ad.sun_path, "./sck");    // adresa = meno soketu (rovnake ako pouziva server)
  s = socket(PF_LOCAL, SOCK_STREAM, 0);
  if (s == -1)
  {
    perror("socket: ");
    exit(2);
  }
  
  printf("running as client\n");
  connect(s, (struct sockaddr*)&ad, sizeof(ad));   // pripojenie na server
  write(s, msg, strlen(msg)+1);                    // poslanie spravy
  close(s);
}
```

---

## 3 server

Upravte server tak, aby každý prijatý reťazec poslal klientovi naspäť,
pričom malé písmená zmení na veľké.

```c
int main(int ac, char **av, char **en) {
  int i, s, ns, r;
  fd_set rs;
  char buff[64], msg[]="server ready\n";
  struct sockaddr_un ad;
  
  memset(&ad, 0, sizeof(ad));
  ad.sun_family = AF_LOCAL;
  strcpy(ad.sun_path, "./sck");
  s = socket(PF_LOCAL, SOCK_STREAM, 0);

  if (s == -1) {
    perror("socket: ");
    exit(2);
  }
  
  unlink("./sck");
  bind(s, (struct sockaddr*)&ad, sizeof(ad));
  listen(s, 5);
  
  // obsluzime len jedneho klienta
  ns = accept(s, NULL, NULL);

  // toto je nieco ako uvitaci banner servera
  // bude to fungovat vdaka tomu, ze klient je schopny citatat aj zo servera,
  // aj z terminalu sucasne (v lubovolnom poradi)
  // strcpy(buff, "Hello from server\nSend a string and I'll send you back the upper case...\n");
  // write(ns, buff, strlen(buff)+1);

  // blokuju citanie, cakanie na poziadavku od klienta
  while((r=read(ns, buff, 64)) > 0) {
    buff[r]=0;          // za poslednym prijatym znakom
    printf("received: %d bytes, string: %s\n", r, buff);
    for (i=0; i<r; i++) buff[i]=toupper(buff[i]);
    printf("sending back: %s\n", buff);
    write(ns, buff, r); // zaslanie odpovede
  }

  perror("read");     // ak klient skonci (uzavrie soket), nemusi ist o chybu
  
  close(ns);
  close(s);
}
```

---

## 4  client

Upravte klienta tak, aby po zadaní reťazca tento odoslal na server,
pričom vždy keď prijme reťazec zo servera, vypíše ho.

```c
int main(int ac, char **av, char **en) {
  int s, r;
  fd_set rs;  // deskriptory pre select()
  char msg[64] = "hello world!";
  struct sockaddr_un ad;
  
  memset(&ad, 0, sizeof(ad));
  ad.sun_family = AF_LOCAL;
  strcpy(ad.sun_path, "./sck");
  s = socket(PF_LOCAL, SOCK_STREAM, 0);
  if (s == -1)
  {
    perror("socket: ");
    exit(2);
  }
  
  printf("running as client\n");
  connect(s, (struct sockaddr*)&ad, sizeof(ad)); // pripojenie na server
  FD_ZERO(&rs);
  FD_SET(0, &rs);
  FD_SET(s, &rs);
  // toto umoznuje klientovi cakat na vstup z terminalu (stdin) alebo zo soketu
  // co je prave pripravene, to sa obsluzi (nezalezi na poradi v akom to pride)
  while(select(s+1, &rs, NULL, NULL, NULL) > 0)
  {
    if (FD_ISSET(0, &rs))  // je to deskriptor 0 = stdin?
    {
      r=read(0, msg, 64);  // precitaj zo stdin (terminal)
                           //if (msg[r-1]=='\n') msg[r-1]=0;
      write(s, msg, r);    // posli serveru (cez soket s)
    }
    if (FD_ISSET(s, &rs))  // je to deskriptor s - soket spojenia na server?
    {
      r=read(s, msg, 1);   // precitaj zo soketu (od servera)
      write(1, msg, r);    // zapis na deskriptor 1 = stdout (terminal)
    }
    FD_ZERO(&rs);          // connect() mnoziny meni, takze ich treba znova nastavit
    FD_SET(0, &rs);
    FD_SET(s, &rs);
  }
  perror("select");        // ak server skonci, nemusi ist o chybu
  close(s);
}
```
