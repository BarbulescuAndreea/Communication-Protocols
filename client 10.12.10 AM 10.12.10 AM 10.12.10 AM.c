#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, calloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include <stdbool.h>

//construieste payload-ul de tip user si parola
char* parolaSiUser(char user[100], char parola[100]){
    char* payload = (char*)malloc(sizeof(char) *250);
	sprintf(payload, "{\"username\": \"%s\", \"password\": \"%s\"}", user, parola);
    return payload;
}

void prepare(char user[100], char parola[100]){
    memset(user, 0, 100);
    memset(parola, 0, 100);
}

void prepareBuffer(char buffer[300]){
	memset(buffer, 0, 300);
}

void prepareId(char id[10]){
	memset(id, 0, 10);
}

char *extractCookie(char *response){
	//iau linia de cookie din raspunsul serverului
	char *cookie = strstr(response, "Set-Cookie");
	if(cookie == NULL){
		return NULL;
	}
	// elimin string-ul "Set-Cookie"
	cookie+=12;

	// scot enter de la final
	char *finalCookie;
	finalCookie = strtok(cookie,"\r\n");

	return finalCookie;
}

int toNumber(char caractere[10]){
	return atoi(caractere);
}

// operatiile efectuate in cazul comenzii de register
void registerCommand(char *raspuns, char *mesaj, int sockid){
    // se deschide conex. la server
    char user[100];
    char parola[100];
    char **payload = (char**)malloc(sizeof(char*));
    prepare(user, parola);

    // citesc de la stdin user-ul
    printf("username=");
	scanf("%s", user);

    // citesc de la stdin parola
    printf("password=");
	scanf("%s", parola);

    // am creat formatul potritiv pentru user si parola
    payload[0] = parolaSiUser(user, parola);

    // trimit post request serverului cu mesajul meu
    mesaj = compute_post_request("34.241.4.235","/api/v1/tema/auth/register","application/json",payload, 1, NULL, 0);
    send_to_server(sockid, mesaj);
    // se primeste raspunsul de la server pe socket
    raspuns = receive_from_server(sockid);

    // afisez raspunsul serverului pentru a se vedea toate informatiile 
    puts(raspuns);

    // inchid conexiunea
    close_connection(sockid);
}

//login
char* loginCommand(char* raspuns, char* mesaj, int sockid){
	// se deschide conexiunea cu serverul meu
	char user[100], parola[100];
	char **payload = (char**)malloc(sizeof(char*));
	prepare(user, parola);

	// user
	printf("username=");
	scanf("%s", user);

	// parola
	printf("password=");
	scanf("%s", parola);

	//creez payload
	payload[0] = parolaSiUser(user, parola);

	//creez mesaj si trimit
	mesaj = compute_post_request("34.241.4.235", "/api/v1/tema/auth/login",
		"application/json", payload, 1, NULL, 0);
	send_to_server(sockid, mesaj);
	raspuns = receive_from_server(sockid);
	//strcat(raspuns, "\0");

	// afisez daca s a putut conecta "200 OK BUN VENIT", altfel, se afiseaza eroarea
	if (strstr(raspuns, "error") != NULL) {
		printf("You can't login!\n");
		printf("The error: \n");
        printf("%s\n", strstr(raspuns, "{\""));
		// SEG FAULT DE CE??
    }else{
        printf("200 OK - BUN VENIT!\n");
    }

	//extragere cookie tinut minte de server pentru user-ul "x"
	char *COOKIE = extractCookie(raspuns);
	if(COOKIE != NULL){
		printf("Your cookie: %s\n", COOKIE);
		return COOKIE;
	}
	
	// inchid conexiunea cu serverul 
	close_connection(sockid);

	return NULL;

}

char* extractToken(char* raspuns){
	char* token = strstr(raspuns, "{\"token\":\"");
	if(token == NULL){
		return NULL;
	}

	// elimin textul "token" din fata token-ului
	token+=10;
	token = strtok(token, "}");
	
	// setez sfarsitul token-ului
	token[strlen(token) - 1] = '\0';
	return token;
}

	char* createURL(char* caleInitiala, char* id){
	char *caleFinala = (char*)malloc(sizeof(char) * 200);

    strcpy(caleFinala, caleInitiala);
    return strcat(caleFinala, id);
}

int isNumberFromString(char id[10]){
	for(int i = 0; id[i]!= '\0'; i++){
		if (isdigit(id[i]) == 0){
			return 0;
		}
	}
	return 1;
}

void deleteBook(char* mesaj, char* raspuns, char* cookie, char* token, int sockid, char** cookies){
	// introducere id de catre user
	char id[10];
	prepareId(id);
	printf("id=");
	scanf("%s", id);
	printf("\n");

	int result = isNumberFromString(id);
	if(result == 0){
		while(true){
			puts("We need a number for the id, please!\n");
			printf("id=");
			scanf("%s", id);
			printf("\n");
			if(isNumberFromString(id) == 1) {
				break;
			}
		}
	}
	cookies[0] = cookie;

	char* cale = createURL("/api/v1/tema/library/books/", id);
	mesaj = compute_delete_request("34.241.4.235", cale, NULL, NULL, 0, token);

    send_to_server(sockid, mesaj);
    raspuns = receive_from_server(sockid);

	// Eroare - daca id ul nu exista, se afiseaza eroare ca nu se poate sterge cartea
	if (strstr(raspuns, "error") != NULL) {
		printf("The book can't be deleted!\n");
		printf("The error: \n");
        printf("%s\n", "The id does not exist!");
    }else{
        printf("The book was deleted!");
    }
	close_connection(sockid);
}
//functie care cere informatii despre o carte

void getInformationAboutBook(char* mesaj, char* raspuns, char* cookie, char* token, int sockid, char** cookies){
	char *id = (char*) malloc(sizeof(char) * 5);
	prepareId(id);

	//afisez textul pentru id-ul ce va fi dat de user de la tastatura
	printf("id=");
	scanf("%s", id);
	//printf("\n");
	int result = isNumberFromString(id);

	if(result == 0){
		while(true){
			puts("We need a number, please!\n");
			printf("id=");
			scanf("%s", id);
			printf("\n");
			if(isNumberFromString(id) == 1) {
				break;
			}
		}
	}
	// adaug cookie-ul in vectorul de cookies
	cookies[0] = cookie;

	char* cale = createURL("/api/v1/tema/library/books/", id);
	mesaj = compute_get_request_token("34.241.4.235", cale, NULL, NULL, 0, token);
	send_to_server(sockid, mesaj);
    raspuns = receive_from_server(sockid);

	//erori
	if(strstr(raspuns, "error") != NULL) {
        printf("%s\n", strstr(raspuns, "{\""));
    }else{
		printf("The details about your book: \n");
		printf("%s\n", strstr(raspuns, "["));
    }

	//inchid conexiunea pe socket
	close_connection(sockid);

}

int main(int argc, char *argv[]) {
	bool dejaLogat = false;
	bool accesBib = false;
	int sockid;
	char *mesaj = NULL, *response = NULL, *cookie = NULL, *token = NULL;
	char **cookies =(char**)malloc(sizeof(char*) * 2);
	char **bodyData = (char**) malloc (40* sizeof(char*));

	while(1){
		char  command[300];
		prepareBuffer(command);
		scanf("%s", command);
		
		//verific ce comanda primesc si apelez functia specifica comenzii
		if(!strcmp(command, "register")){
			// deschid conexiunea cu server-ul meu
			sockid = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
			registerCommand(response, mesaj, sockid);
		}
		if(!strcmp(command, "login")){
			if(dejaLogat == true){
				printf("You're already logged!\n");
				continue;
			}
			// deschid conexiunea cu server-ul meu
			sockid = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
			cookie = loginCommand(response, mesaj, sockid);
			dejaLogat = true;
		}
		if(!strcmp(command, "logout")){
			if(dejaLogat == false){
				printf("You should log-in first!\n");
			}else{
				//trimit cerere get la server cu cookie-ul user-ului ce trebuie deconectat
				// deschid conexiunea cu server-ul meu
				sockid = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
				cookies[0] = cookie;
				mesaj = compute_get_request("34.241.4.235", "/api/v1/tema/auth/logout", NULL, cookies, 1);
				send_to_server(sockid, mesaj);

				// primesc raspunsul serverului
				response = receive_from_server(sockid);
				printf("User delogged succesfully!\n");
				dejaLogat = false;
				cookie = NULL; // curatare cookie
				token = NULL; // clean token

				//inchidere conexiune pe socket dupa logout
				close_connection(sockid);
			}
		}
		if(!strcmp(command, "enter_library")){
			// in cazul in care nu sunt logat, nu pot accesa libraria, nu se permite accesul
			if(dejaLogat == false){
			 	printf("Access denied! You're not logged in!\n");
			}
			// deschid conexiunea cu server-ul meu
			sockid = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
			cookies[0] = cookie;

			mesaj = compute_get_request_token("34.241.4.235", "/api/v1/tema/library/access", NULL, cookies, 1, NULL);
			send_to_server(sockid, mesaj);
			response = receive_from_server(sockid);

			// obtin token-ul de care am nevoie pentru operatiile asupra cartilor
			token = extractToken(response);
			if(token == NULL){
				printf("You have no token!\n");
			}else{
				puts("You entered the library!:)");
				//accesBib devine true fiindca am intrat cu succes in biblioteca
				accesBib = true;
			}

			close_connection(sockid);
		}
		if(!strcmp(command, "get_books")){
			// daca utilizatorul nu este logat sau nu are acces in biblioteca, nu poate vedea cartile
			if(dejaLogat == false){
				printf("You can't see the books until you're logged!\n");
				continue;
			}
			if(accesBib == false){
				printf("You can't see the books until you have access in library!\n");
				continue;
			}
			// deschid conexiunea cu server-ul meu
			sockid = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

			mesaj = compute_get_request_token("34.241.4.235", "/api/v1/tema/library/books", NULL, NULL, 0, token);

			//trimit mesajul la server
            send_to_server(sockid, mesaj);

			// accesez raspunsul pentru a verifica daca am obtinut cartile sau in caz contrar, ce eroare am
            response = receive_from_server(sockid);

            printf("The books are: %s\n", strstr(response, "["));

			// inchid conexiunea pe socket dupa ce cer cartile
            close_connection(sockid);
		}
		if(!strcmp(command, "get_book")){
			// daca utilizatorul nu este logat sau nu are acces in biblioteca, nu poate cere informatii despre carte
			if(dejaLogat == false){
				printf("You can't have a book until you're logged!\n");
				continue;
			}
			if(accesBib == false){
				printf("You can't have a book until you have access in library!\n");
				continue;
			}
			// deschid conexiunea cu server-ul meu
			sockid = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
			getInformationAboutBook(mesaj, response, cookie, token, sockid, cookies);
		}
		if(!strcmp(command, "add_book")){

			int nr;
			if(accesBib == false){
				printf("You can't add a book until you have access in library!\n");
				continue;
			}
			// deschid conexiunea cu server-ul meu
			sockid = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
						
			char title[100];
			printf("title=");
			char ch;
			while ((ch = getchar()) != '\n' && ch != EOF);  
			fgets(title,100, stdin);
			strtok(title, "\n");

			char author[100];
			printf("author=");
			fgets(author,100, stdin);
			strtok(author, "\n");

			char genre[100];
			printf("genre="); 
			fgets(genre,100, stdin); 
			strtok(genre, "\n");

			char page_count[100];
			printf("page_count=");
			scanf("%s",page_count);
			while ((ch = getchar()) != '\n' && ch != EOF); 

			char publisher[100];
			printf("publisher=");
			fgets(publisher,100, stdin);
			strtok(publisher, "\n");

			// trebuie ca page_count sa fie numar
			if(isNumberFromString(page_count) == 0){
				while(true){
					puts("Page should be a number, enter one!\n");
					printf("page_count=");
					scanf("%s", page_count);
					printf("\n");
					if(isNumberFromString(page_count) == 1) {
						break;
					}
				}
			}
			nr = toNumber(page_count);

			char payload[1500];
			sprintf(payload, "{\"title\": \"%s\", \"author\": \"%s\", \"genre\":" " \"%s\",\"page_count\": %d, \"publisher\": \"%s\"}",
									title, author, genre, nr, publisher);
			cookies[0] = cookie;
			bodyData[0] = payload;

			// cere de tip post cu token catre server
			mesaj = compute_post_request_token("34.241.4.235", "/api/v1/tema/library/books", 
                    	"application/json", bodyData, 1, NULL, 0, token);
			send_to_server(sockid, mesaj);
            response = receive_from_server(sockid);

			puts("The book was added!");
			// ERORI ADAUGARE CARTE

			close_connection(sockid);
		}
		if(strcmp(command, "delete_book") == 0){
			if(accesBib == false){
				printf("You can't delete a book until you have access in library!\n");
				continue;
			}
			cookies[0] = cookie;
			// deschid conexiunea cu server-ul meu
			sockid = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

			deleteBook(mesaj, response, cookie, token, sockid, cookies);
		}
		if(!strcmp(command, "exit")){
			return 0;
		}
	}
	free(token);
	free(cookies[0]);
	free(cookies);
	return 0;
}
