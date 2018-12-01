#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define HALL_ROWS 15
#define HALL_COLUMNS 20
#define CINEMA_FILE "cinema.txt"
#define FINISH_OPTION "finish"
#define BACK_OPTION "back"
#define RESET_OPTION "reset"
#define RESERVE_OPTION "reserve"

typedef char * string;
// =====================================================================================
//									MY STRUCTS
// A session of a film that shows all free and reserved seats
typedef struct {
	char time[20];
	short seats[HALL_ROWS][HALL_COLUMNS];
} TSession;

// A film showed in the cinema theater, 
// has its name and sessions of that film
typedef struct {
	char name[50];
	TSession* sessions;
	short sessions_cnt;
	short price;
} TFilm;

// Represents the cinema theater that has its films, 
// films have their sessions and sessions have their seats
typedef struct {
	TFilm* films;
	short films_cnt;
} TCinemaTheater;
// Defines states of the application
typedef enum {
	OPTIONS,
	CANCELLING,
	MENU,
	SESSIONS,
	HALL
} TState;
// Represents a simple seat coordinate
typedef struct {
	short row;
	short column;
} TSeat;
// Defines user's reservation info
typedef struct {
	char film_name[50];
	char film_time[20];
	TSeat* reserved_seats;
	short seat_cnt;
} TReservation;
// =====================================================================================
//									OPERATING WITH STRUCTS
TCinemaTheater* initCinemaTheater() {
	TCinemaTheater* cinema_theater = (TCinemaTheater*) malloc(sizeof(TCinemaTheater));
	cinema_theater->films_cnt = 0;
	cinema_theater->films = NULL;
	return cinema_theater;
}
TCinemaTheater* addFilm(TCinemaTheater* cinema_theater, TFilm* film) {
	cinema_theater->films_cnt++;
	// make more space for another film
	cinema_theater->films = (TFilm*) realloc(cinema_theater->films, cinema_theater->films_cnt * sizeof(TFilm));
	TFilm* f = &(cinema_theater->films[cinema_theater->films_cnt - 1]);
	// copy the film
	strcpy(f->name, film->name);
	f->sessions = film->sessions;
	f->sessions_cnt = film->sessions_cnt;
	f->price = film->price;
	free(film);
	return cinema_theater;
}

TFilm* initFilm(const string s, short price) {
	TFilm* film = (TFilm*) malloc(sizeof(TFilm));
	strcpy(film->name, s);
	film->price = price;
	film->sessions = NULL;
	film->sessions_cnt = 0;
	return film;
}
TFilm* addSession(TFilm* film, TSession* session) {
	film->sessions_cnt++;
	// make more space for another session
	film->sessions = (TSession*) realloc(film->sessions, film->sessions_cnt * sizeof(TSession));
	TSession* s = &(film->sessions[film->sessions_cnt - 1]);
	// copy the session
	strcpy(s->time, session->time);
	short i = 0, j = 0;
	for (i = 0; i < HALL_ROWS; i++) {
		for (j = 0; j < HALL_COLUMNS; j++) {
			s->seats[i][j] = session->seats[i][j];
		}
	}
	free(session);
	return film;
}

// early declaration
TSession* readSessionFromFile(const string file_name);
TSession* initSession(const string film_name, const string time) {
	TSession* session = (TSession*) malloc(sizeof(TSession));
	strcpy(session->time, time);

	short i = 0, j = 0;
	char file_name[100];
	// build the name of the file
	strcpy(file_name, "");
	strcat(file_name, film_name);
	strcat(file_name, "_");
	strcat(file_name, time);
	strcat(file_name, ".txt");
	// try to read it
	TSession* redSession = readSessionFromFile(file_name);
	for (i = 0; i < HALL_ROWS; i++) {
		for (j = 0; j < HALL_COLUMNS; j++) {
			// if redSession is defined, copy the values, otherwise make it false
			session->seats[i][j] = redSession ? redSession->seats[i][j] : FALSE;
		}
	}
	free(redSession);
	return session;
}

void cleanCinemaTheater(TCinemaTheater* cinema_theater) {
	if (!cinema_theater) return;
	int i = 0;
	for (i = 0; i < cinema_theater->films_cnt; i++) {
		TFilm* f = &(cinema_theater->films[i]);
		free(f->sessions);
	}
	free(cinema_theater->films);
	free(cinema_theater);
}

TReservation* initReservation(const string name, const string time) {
	TReservation* res = (TReservation*) malloc(sizeof(TReservation));
	strcpy(res->film_name, name);
	strcpy(res->film_time, time);
	res->seat_cnt = 0;
	res->reserved_seats = NULL;
	return res;
}
TReservation* addReservedSeat(TReservation* res, short row, short column) {
	res->seat_cnt++;
	res->reserved_seats = (TSeat*) realloc(res->reserved_seats, res->seat_cnt * sizeof(TSeat));
	TSeat* seat = &res->reserved_seats[res->seat_cnt - 1];
	seat->row = row;
	seat->column = column;
	return res;
}
void cleanReservation(TReservation* res) {
	free(res->reserved_seats);
	free(res);
}
// =====================================================================================
//									WORKING WITH FILES
TCinemaTheater* readCinemaTheaterFromFile(const string file_name) {
	FILE* cinema_file = fopen(file_name, "r");
	if (!cinema_file) {
		printf("Couldn't find such file\n");
		return NULL;
	}

	char c;
	// Read the beginning of the file
	if (fscanf(cinema_file, " %c", &c) != 1 || c != '{') {
		fclose(cinema_file);
		printf("Couldn't read the beginning of the file\n");
		return NULL;
	}

	char s[100], s1[20];
	TCinemaTheater* cinema_theater = initCinemaTheater();
	// Read each film from a "film: {sessions};" line
	while (fscanf(cinema_file, " %[^:]%*c", s)) {
		// If it's the end, finish reading
		if (!strcmp(s, "}")) break;

		TFilm* film = initFilm(s, 150);
		// Read each session of that film
		while (fscanf(cinema_file, " %[^,;]%c", s1, &c)) {
			if ((c != ',' && c != ';') || (strlen(s1) > 6)) {
				printf("Error reading film sessions\n");
				cleanCinemaTheater(cinema_theater);
				fclose(cinema_file);
				return NULL;
			}

			TSession* session = initSession(film->name, s1);
			film = addSession(film, session);
			if (c == ';') break;
		}

		cinema_theater = addFilm(cinema_theater, film);
	}

	fclose(cinema_file);
	return cinema_theater;
}

TSession* readSessionFromFile(const string file_name) {
	FILE* session_file = fopen(file_name, "r");
	if (!session_file) return NULL;

	TSession* session = (TSession*) malloc(sizeof(TSession));
	short i, j;
	for (i = 0; i < HALL_ROWS; i++) {
		for (j = 0; j < HALL_COLUMNS; j++) {
			if (fscanf(session_file, "%hd", &(session->seats[i][j])) != 1
				|| session->seats[i][j] < FALSE || session->seats[i][j] > TRUE)
			{
				printf("Error reading session file\n");
				free(session);
				fclose(session_file);
				return NULL;
			}
		}
	}
	return session;
}

void saveSessionToFile(const string film_name, TSession* session) {
	char file_name[100];
	// build the name of the file
	strcpy(file_name, "");
	strcat(file_name, film_name);
	strcat(file_name, "_");
	strcat(file_name, session->time);
	strcat(file_name, ".txt");

	// open and clean it or make a new file
	FILE* session_file = fopen(file_name, "w+");
	short i, j;
	for (i = 0; i < HALL_ROWS; i++) {
		for (j = 0; j < HALL_COLUMNS; j++) {
			fprintf(session_file, "%d ", session->seats[i][j]);
		}
		fprintf(session_file, "\n");
	}
	fclose(session_file);
}
// =====================================================================================
//									MODEL FUNCTIONS
int cancelReservation(TCinemaTheater* cinema, TReservation* res) {
	int i;
	TFilm* film = NULL;
	TSession* session = NULL;
	// find the film
	for (i = 0; i < cinema->films_cnt; i++) {
		TFilm* f = &cinema->films[i];
		if (!strcmp(f->name, res->film_name)) film = f; 
	}
	if (!film) {
		printf("Couldn't find the film\n");
		return -1;
	}
	// find the session
	for (i = 0; i < film->sessions_cnt; i++) {
		TSession* s = &film->sessions[i];
		if (!strcmp(s->time, res->film_time)) session = s;
	}
	if (!session) {
		printf("Couldn't find the session\n");
		return -1;
	}
	// check that all the seats are actually reserved
	for (i = 0; i < res->seat_cnt; i++) {
		TSeat* s = &res->reserved_seats[i];
		if (session->seats[s->row][s->column] == FALSE) {
			printf("Wrong reservation code\n");
			return -1;
		}
	}
	// cancel the reservation
	for (i = 0; i < res->seat_cnt; i++) {
		TSeat* s = &res->reserved_seats[i];
		session->seats[s->row][s->column] = FALSE;
	}
	printf("Your reservation was successfully canceled\n");
	return 0;
}

string makeReservationCode(TReservation* res) {
	int i;
	string code = (string) malloc(100 * sizeof(char));
	strcpy(code, "");
	strcat(code, res->film_name);
	strcat(code, ";");
	strcat(code, res->film_time);
	strcat(code, ";");
	for (i = 0; i < res->seat_cnt; i++) {
		TSeat* seat = &res->reserved_seats[i];
		char s[10];
		sprintf(s, "<%hd, %hd>", seat->row, seat->column);
		strcat(code, s);
	}
	return code;
}

TReservation* convertFromCode(const string code) {
	char name[50], time[20];
	int len, len_sum;
	if (sscanf(code, "%[^;]%*c%n", name, &len) != 1) {
		printf("Error converting name\n");
		return NULL;
	}
	len_sum = len;
	if (sscanf(code + len_sum, "%[^;]%*c%n", time, &len) != 1) {
		printf("Error converting time\n");
		return NULL;
	}
	len_sum += len;
	
	TReservation* res = initReservation(name, time);
	short column, row;
	while (sscanf(code + len_sum, "<%hd, %hd>%n", &row, &column, &len) == 2) {
		len_sum += len;
		res = addReservedSeat(res, row, column);
	}
	if (!res->reserved_seats) {
		printf("Error converting seats\n");
		cleanReservation(res);
		return NULL;
	}
	return res;
}
// =====================================================================================
//									WORKING WITH STD I/O
void printCinemaTheater(TCinemaTheater* cinema) {
	int i, j;
	for (i = 0; i < cinema->films_cnt; i++) {
		TFilm* f = &(cinema->films[i]);
		printf("%s: ", f->name);
		for (j = 0; j < f->sessions_cnt; j++) {
			printf("%s,", f->sessions[j].time);
		}
		printf("\n");
	}
}

void printOptions() {
	printf("-------------------------------------------------------------------------\n");
	printf("|                    0) Making a reservation\n");
	printf("|                    1) Cancelling a reservation\n");
	printf("|\n");
	printf("|  Type 'finish' to exit the application\n");
	printf("-------------------------------------------------------------------------\n");
}
int askForOption() {
	int choice;
	char s[100];
	printf("Choose an option (enter a number): ");
	while (scanf(" %d", &choice) != 1 
		|| choice < 0 
		|| choice >= 2)
	{
		// flush the buffer to prevent infinite loop
		scanf("%[^\n]", s);
		// check if user wants another options
		if (!strcmp(s, FINISH_OPTION)) return -1;
		// if you didn't scan the right digit that mathches the range
		// print a warning and start scanning again
		printf("You can enter only a number from the range!\n");
		printf("Choose an option (enter a number): ");
	}
	return choice;
}

void printCancelling() {
	printf("-------------------------------------------------------------------------\n");
	printf("|                   Enter your reservation code\n");
	printf("|\n");
	printf("|  Type 'finish' to exit the application\n");
	printf("| or 'back' to return to the previous menu\n");
	printf("-------------------------------------------------------------------------\n");
}
int askForCode(TReservation** res) {
	char s[100];
	TReservation* r = NULL;
	while (!r) 
	{
		printf("Type your reservation code: ");
		scanf(" %[^\n]", s);
		// check if user wants another options
		if (!strcmp(s, FINISH_OPTION)) return -1;
		else if (!strcmp(s, BACK_OPTION)) return -2;
		// try to convert
		r = convertFromCode(s);
		// if you didn't scan right string, start scanning again
	}
	*res = r;
	return 0;
}

void printFilmMenu(TCinemaTheater* cinema) {
	int i;
	printf("-------------------------------------------------------------------------\n");
	for (i = 0; i < cinema->films_cnt; i++) {
		printf("|            %d) %s\n", i, cinema->films[i].name);
	}
	printf("|\n");
	printf("|  Type 'finish' to exit the application\n");
	printf("| or 'back' to return to the previous menu\n");
	printf("-------------------------------------------------------------------------\n");
}
int askForFilm(TCinemaTheater* cinema) {
	int choice;
	char s[100];
	printf("Choose a film (enter a number): ");
	while (scanf(" %d", &choice) != 1 
		|| choice < 0 
		|| choice >= cinema->films_cnt)
	{
		// flush the buffer to prevent infinite loop
		scanf("%[^\n]", s);
		// check if user wants another options
		if (!strcmp(s, FINISH_OPTION)) return -1;
		else if (!strcmp(s, BACK_OPTION)) return -2;
		// if you didn't scan the right digit that mathches the range
		// print a warning and start scanning again
		printf("You can enter only a number from the range!\n");
		printf("Choose a film (enter a number): ");
	}
	return choice;
}

void printFilmSessions(TFilm* film) {
	int i;
	printf("-------------------------------------------------------------------------\n");
	printf("|                     %s:\n", film->name);
	for (i = 0; i < film->sessions_cnt; i++) {
		printf("|                 %d) %s\n", i, film->sessions[i].time);
	}
	printf("|\n");
	printf("| Type 'finish' to exit the application\n");
	printf("| or 'back' to return to the previous menu\n");
	printf("-------------------------------------------------------------------------\n");
}
int askForSession(TFilm* film) {
	int choice;
	char s[100];
	printf("Choose the time (enter a number): ");
	while (scanf("%d", &choice) != 1
		|| choice < 0
		|| choice >= film->sessions_cnt)
	{
		// flush the buffer to prevent infinite loop
		scanf("%[^\n]", s);
		// check if user wants another options
		if (!strcmp(s, FINISH_OPTION)) return -1;
		else if (!strcmp(s, BACK_OPTION)) return -2;
		// if you didn't scan the right digit that mathches the range
		// print a warning and start scanning again
		printf("You can enter only a number from the range!\n");
		printf("Choose the time (enter a number): ");
	}
	return choice;
}

void printHall(const string film_name, TSession* session) {
	int i, j;
	printf("---------------------------------------------------------------------------------------------\n");
	printf("|                     %s %s:\n", film_name, session->time);
	printf("|       \n");
	for (i = 0; i < HALL_ROWS; i++) {
		printf("|       ");
		for (j = 0; j < HALL_COLUMNS; j++) {
			if (session->seats[i][j]) {
				printf("{%d} ", j);
			} else {
				printf("|%d| ", j);
			}
		}
		printf("      row: %d\n", i);
	}
	printf("|\n");
	printf("|       |..| - free seat, {..} - reserved seat\n");
	printf("|\n");
	printf("|\n");
	printf("| Type 'finish' to exit the application,\n");
	printf("| 'back' to return to the previous menu,\n");
	printf("| 'reset' to reset the actual reservation,\n");
	printf("| 'reserve' to reserve all chosen seats and get a reservation code\n");
	printf("---------------------------------------------------------------------------------------------\n");
}
int askForSeat(short * row, short * column) {
	int i, j;
	char s[100];
	printf("Choose the seat (enter \"row:column\"):");
	while (scanf(" %d:%d", &i, &j) != 2
		|| (i < 0 || i >= HALL_ROWS) || (j < 0 || j >= HALL_COLUMNS))
	{
		// flush the buffer to prevent infinite loop
		scanf("%[^\n]", s);
		// check if user wants another options
		if (!strcmp(s, FINISH_OPTION)) return -1;
		else if (!strcmp(s, BACK_OPTION)) return -2;
		else if (!strcmp(s, RESERVE_OPTION)) return 1;
		else if (!strcmp(s, RESET_OPTION)) return 2;
		// if you didn't scan the right input that mathes the range
		// print a warning and start scanning again
		printf("You can enter only numbers from the range!\n");
		printf("Choose the seat (enter \"row:column\"):");
	}
	// pass the values to the output parameters
	*row = i;
	*column = j;
	return 0;
}

void printSeatsForReservation(TReservation* res) {
	printf("Chosen seats for reservation: ");
	TSeat* seats = res->reserved_seats;
	if (!seats) {
		printf("\n---------------------------------------------------------------------------------------------\n");
		return;
	}
	int i;
	for (i = 0; i < res->seat_cnt; i++) {
		TSeat* s = &res->reserved_seats[i];
		printf("<%d, %d> ", s->row, s->column);
	}
	printf("\n---------------------------------------------------------------------------------------------\n");
}
// =====================================================================================

int main(void) {

	TCinemaTheater* cinema = readCinemaTheaterFromFile(CINEMA_FILE);
	
	if (cinema) {
		//printCinemaTheater(cinema);
		//printf("\n\n\n");

		TFilm* film = NULL;
		TSession* session = NULL;
		TReservation* reservation = NULL;
		TState state = OPTIONS;
		int user_choice;

		// main loop
		while (TRUE) {
			switch(state) {
				case OPTIONS: {
					// print options menu
					printOptions();
					// ask for a choice
					user_choice = askForOption();
					switch (user_choice) {
						case 0: {
							// making a reservation
							state = MENU;
							break;
						}
						case 1: {
							// cancelling a reservation
							state = CANCELLING;
							break;
						}
						case -1: {
							// finish
							return 0;
						}
					}
					break;
				}
				case CANCELLING: {
					printCancelling();
					// ask user to enter the reservation code for cancelling
					TReservation* res;
					user_choice = askForCode(&res);
					switch (user_choice) {
						case 0: {
							// reservation decoded
							int result;
							result = cancelReservation(cinema, res);
							cleanReservation(res);
							// if reservation deleted, go to the next state
							if (!result) return 0;
							break;
						}
						case -1: {
							// finish
							return 0;
						}
						case -2: {
							// back
							state = OPTIONS;
						}
					}
					break;
				}
				case MENU: {
					// print the list of films
					printFilmMenu(cinema);
					// ask for a film from the list
					user_choice = askForFilm(cinema);
					switch (user_choice) {
						case -1: {
							// finish
							return 0;
						}
						case -2: {
							// back
							state = OPTIONS;
							break;
						}
						default: {
							// selected item in the menu
							state = SESSIONS;
							film = &cinema->films[user_choice];
							break;
						}
					}
					break;
				}
				case SESSIONS: {
					// print all sessions of that film
					printFilmSessions(film);
					// ask for a session of the film
					user_choice = askForSession(film);
					switch (user_choice) {
						case -1: {
							// finish
							return 0;
						}
						case -2: {
							// back
							state = MENU;
						}
						default: {
							state = HALL;
							session = &film->sessions[user_choice];
							break;
						}
					}
					break;
				}
				case HALL: {
					if (!reservation) reservation = initReservation(film->name, session->time);
					// print the hall for that session
					// and seats that are going to be reserved 
					printHall(film->name, session);
					printSeatsForReservation(reservation);
					// ask for a seats
					short seatRow, seatColumn;
					int res = askForSeat(&seatRow, &seatColumn);
					switch (res) {
						case 0: {
							// reserve the chosen seat
							if (session->seats[seatRow][seatColumn]) {
								printf("\n\n");
								printf("This seat is already reserved! Choose another\n");
							}
							else {
								session->seats[seatRow][seatColumn] = TRUE;
								reservation = addReservedSeat(reservation, seatRow, seatColumn);
							}
							break;
						}
						case -1: {
							// finish
							cleanReservation(reservation);
							return 0;
						}
						case -2: {
							// back
							state = SESSIONS;
							cleanReservation(reservation);
							reservation = NULL;
							break;
						}
						case 1: {
							// reserve
							// generate code and save to the file
							string s = makeReservationCode(reservation);
							printf("\n\n");
							printf("Your reservation code: \"%s\"\n", s);
							saveSessionToFile(film->name, session);
							cleanReservation(reservation);
							reservation = NULL;
							return 0;
						}
						case 2: {
							// reset
							int i;
							for (i = 0; i < reservation->seat_cnt; i++) {
								TSeat* s = &reservation->reserved_seats[i];
								session->seats[s->row][s->column] = FALSE;
							}
							free(reservation->reserved_seats);
							reservation->seat_cnt = 0;
							reservation->reserved_seats = NULL;
						}
					}
					break;
				}
			}
		}
	}

	cleanCinemaTheater(cinema);
	return 0;
}