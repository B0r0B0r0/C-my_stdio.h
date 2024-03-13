# Structura SO_FILE
 * Structura contine urmatoarele:
   * fd - file descriptorul fisierului deschis
   * cursor - un index care indica pozitia currenta in buffer
   * errorFound - flag care este setat daca s-a intalnit o eroare
   * toBeWritten - flag care imi arata daca ce este in buffer trebuie scris in fisier (Folosit in special pentru fisierele deschise cu append)
   * bytesRead - numar care imi tine minte cati octeti am citit din fisier
   * eof - flag care este setat daca am ajuns la sfarsitul fisierului
   * fcursor - index care imi indica pozitia relativa in fisier
   * openedPipe - int care imi tine minte capatul deschis de la pipe (Creat la popen)
   * buffer - un vector de caractere pe care realizez operatiile de citire/scriere
   * permision - vector de maxim 2 caractere, tin minte permisiunile de deschidere a fisierului
   * pid - pidul procesului copil creat

# so_fopen
 * Aloc memorie pentru structura, initializez toate variabilele si deschid fisierul cu permisiunile trimise ca parametru

# so_fclose
 * apelez 'so_fflush' in cazul in care mai era ceva de sris din buffer, dupa care egalez toate variabilele cu 0 si dezaloc memoria

# so_fileno
 * returnez file descriptorul fisierului deschis

# so_fseek
 * apelez 'so_fflush' in cazul in care mai era ceva de scris din buffer, dupa care mut cursorul in functie de parametrii trimisi

# so_fseek
 * returnez variabile fcursor, care imi indica pozitia relativa in fisierul deschis

# so_fflush
 * Verific daca fisierul a fost deschis cu append, in acest caz mut cursorul la sfarsitul fisierului.
 * Verific daca fisierul are drepturi de scriere, dupa care verific daca flagul toBeWritten este setat, in acest caz, realizez scrierea in fisier.
 * Golesc tot bufferul si setez variabilele cursor si 'bytesRead' pe 0

# so_fgetc
 * Verific daca fisierul are drepturi de citire.
 * Incerc sa citesc din fisier un bufferSize (In acest caz, 4096 de octeti)
 * Fac verificari in functie de valoarea de return a apelului de sistem read.
 * Verific daca cursorul este la finalul buffer-ului (adica la valoarea bytesRead) si apelez 'so_fflush' daca este cazul.
 * Returnez valoarea din 'buffer' de la pozitia 'cursor'

# so_fputc
 * Verific daca fisierul are drepturi de scriere
 * Verific daca se afla valori in buffer
 * Verific daca bufferul este plin, caz in care apelez 'so_fflush'
 * Scriu in 'buffer' pe pozitia 'cursor' valoarea transmisa ca parametru
 * Incrementez cursor si fcursor si setez flagul toBeWritten pe true

# so_feof
 * Returnez flagul 'eof'

# so_ferror
 * Returnez flagul 'errorFound'

# so_fread
 * Fac o incluziune de for-uri. Primul incepe de la 0 si se opreste la 'nmemb' si al doilea incepe de la 0 si se opreste la 'size'. Pentru fiecare iteratie din for-ul inclus, incerc citirea unui octet (apeland functia 'so_fgetc'), verific valoarea de return si scriu in buffer transmis ca parametru.
 * returnez, atat la finalul functiei cat si in caz de eroare, numarul de elemente citite din fisier.

# so_fwrite
 * Fac o incluziune de for-uri. Primul incepe de la 0 si se opreste la 'nmemb' si al doilea incepe de la 0 si se opreste la 'size'. Pentru fiecare iteratie din for-ul inclus, incerc scrierea unui octet (apeland functia 'so_fputc'), dupa care verific valoarea de return.
 * returnez, atat la finalul functiei cat si in caz de eroare, numarul de elemente scrise in fisier.

# so_popen
 * Verific parametrii
 * Creez pipe-ul
 * Aloc memorie si initializez toate variabilele unei structuri 'SO_FILE'
 * Verific parametrul type
   * Daca type este 'r', fac fork, inchid capetele nefolosite ale pipe-ului (atat in copil cat si in parinte), dupa care fac standard output-ul copilului sa fie capatul deschis al pipe-ului si execut comanda.
   * Daca type este 'w', fac fork, inchid capetele nefolosite ale pipe-ului (atat in copil cat si in parinte), dupa care fac standard input-ul copilului sa fie capatul deschis al pipe-ului si execut comanda.

# so_pclose
 * Apelez functia 'so_fflush' in cazul in care a ramas ceva nescris.
 * Inchid ultimul capat deschis al pipe-ului
 * Astept terminarea procesului copil utilizand apelul de sistem 'waitpid'
 * Verific valoarea de return a apelului de sistem si o returnez
