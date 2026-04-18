#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

extern "C" {
//#include"./SDL2-2.0.10/include/SDL.h"dobra
#include <SDL.h>
//#include"./SDL2-2.0.10/include/SDL_main.h"
#include <SDL_main.h>
}

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480

#define GRID_ELEMENTS 20                                                                                //ilosc kratek w siatce
#define GRID_SIZE 400                                                                                   //wielkosc siatki
#define CELL_SIZE GRID_SIZE/GRID_ELEMENTS                                                               //wielkosc jednej kratki w siatce
#define GAP 8                                                                                           //przerwa potrzebna do rysowania niektorych elementow
#define CENTERED_BORDER_X ((SCREEN_WIDTH / 2) - (GRID_SIZE / 2))                                        //obliczanie punktu x startowego rysowania granic
#define CENTERED_BORDER_Y ((SCREEN_HEIGHT / 2) - (GRID_SIZE / 2))                                       //obliczanie punktu y startowego rysowania granic
#define REQUIREMENTS_STARTING_HEIGHT ((GRID_SIZE / 2) - (GRID_SIZE / 4) + 2 * GAP)                      //poczatkowa wysokosc rysowania spelnionych wymagan

#define STARTING_DIRECTION rand()%4                                                                     //poczatkowy kierunek ruchu weza
#define STARTING_SPEED 0.12                                                                             //poczatkowa predkosc weza
#define STARTING_LENGTH 1                                                                               //poczatkowa dlugosc weza
#define STARTING_POSITION ((GRID_ELEMENTS / 2) + ((GRID_ELEMENTS / 2) - 1))                             //poczatkowa pozycja weza

#define TIME_FOR_SPEEDUP 5.0                                                                            //interwal czasu miedzy przyspieszeniem gry
#define SPEEDUP_FACTOR 0.97                                                                             //mnoznik przyspieszenia czasu
#define MAX_SPEED 0.04                                                                                  //maksymalna predkosc

#define CHANSE_FOR_REDDOT 200000000                                                                     //szansa na wygnrowanie bonusu

#define BLUEDOT_SIZE 3                                                                                  //wielkosc niebieskiej kropki (im wiecej tym mniejsza kropka)
#define BONUS_SIZE 2.5                                                                                  //wielkosc bonusu (im wiecej tym mniejsza kropka)
#define BONUS_LIFETIME 5.0                                                                              //czas na zdobycie bonusu
#define BONUS_SLOWDOWN 1.2                                                                              //mnoznik zwolnienia czasu gry poprzez bonus
#define BONUS_SHORTEN 2                                                                                 //ilosc zmniejszenia unitow weza poprzez bonus
#define BONUS_TIMER_WIDTH 300                                                                           //szerokosc timera bonusu
#define BONUS_TIMER_HEIGHT 20                                                                           //wysokosc timera bonusu
#define BLUEDOT_POINTS 6                                                                                //punkty za zdobycie niebieskiej kropki
#define REDDOT_POINTS rand()%50+50                                                                      //punkty za zdobycie bonusu

#define BEST_SCORES_FILE "best_scores.txt"                                                              //nazwa pliku z najlepszymi wynikami
#define NUMER_OF_SCORES 3                                                                               //ilosc zapisywanych najlepszych wynikow
#define MAX_NAME_LENGTH 20                                                                              //maksymalna dlugosc nazwy gracza

enum {                                                                                                  //kierunki ruchu weza
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

struct dot {                                                                                            //struktura oblugi kropek oraz innych elementow
    int x, y;
};

struct scores {                                                                                         //struktura do przechowywania najlepszych wynikow
    char name[MAX_NAME_LENGTH];
    int score;
};

typedef struct element {                                                                                //struktura obslugujaca weza
    int x, y, direction;
    struct element* nextPos;
} Snake;

typedef struct {                                                                                        //struktura obslugujaca kolory
    int czarny, zielony, czerwony, niebieski, szary, ciemno_zielony, bialy;
} Colors;


Snake* init_snake() {                                                                                   //funkcja inizcjalizujaca weza
    Snake* start = (Snake*)malloc(sizeof(Snake));
    if (start == NULL) {
        fprintf(stderr, "Blad alokacji pamieci dla weza!\n");
        return NULL;
    }
    else {
        start->nextPos = NULL;
        start->direction = STARTING_DIRECTION;
        start->x = rand() % STARTING_POSITION;
        start->y = rand() % STARTING_POSITION;

        return start;                                                                                   //zwrocenie wskaznika do glowy weza
    }
}

void add_segment(Snake** last) {                                                                        //dodawanie nowego segmentu weza
    Snake* new_segment = (Snake*)malloc(sizeof(Snake));
    struct dot move = { 0 };
    if (new_segment != NULL) {
        switch ((*last)->direction) {
        case UP:
            move.y = 1;
            break;
        case DOWN:
            move.y = -1;
            break;
        case RIGHT:
            move.x = -1;
            break;
        case LEFT:
            move.x = 1;
            break;
        }

        new_segment->direction = (*last)->direction;
        new_segment->nextPos = NULL;
        new_segment->x = (*last)->x + move.x;                                                           //w zaleznosci od kierunku weza dodajemy segment w odpowiednim miejscu
        new_segment->y = (*last)->y + move.y;

        (*last)->nextPos = new_segment;
        (*last) = new_segment;
    }
    else {
        fprintf(stderr, "Blad alokacji pamieci dla zwiekszania weza!\n");
    }
}

void delete_segment(Snake** first, Snake** last) {
    if ((*first)->nextPos == *last) {                                                                       //sprawdzenie czy waz ma wiecej niz jeden segment
        fprintf(stderr, "Waz nie moze byc skrocony, poniewaz sklada sie z jednego elementu!\n");
        return;
    }

    Snake* current = *first;
    while (current->nextPos != *last) {                                                                    //znajdowanie przedostatniego segmenetu
        current = current->nextPos;
    }

    free(*last);                                                                                        //usuniecie ostatniego segmentu
    *last = current;
    (*last)->nextPos = NULL;
}

void move_snake_directions(Snake* new_head) {                                                           //funkcja obslugujaca kierunki ruchu weza
    switch (new_head->direction) {
    case UP:
        new_head->y--;
        break;
    case DOWN:
        new_head->y++;
        break;
    case RIGHT:
        new_head->x++;
        break;
    case LEFT:
        new_head->x--;
        break;
    }
}

void copying_segment(Snake* new_head, Snake** first) {                                                   //funkcja przekazujaca jedna strukture segmentu weza do drugiej
    new_head->x = (*first)->x;
    new_head->y = (*first)->y;
    new_head->direction = (*first)->direction;
}

void move_snake(Snake** first, Snake** last) {                                                           //glowna funkcjaod obslugi ruchu weza po ekranie
    if (*first == NULL || *last == NULL) {
        fprintf(stderr, "Error: Waz nie ma elementow!\n");
        return;
    }

    Snake* new_head = (Snake*)malloc(sizeof(Snake));                                                    //tworzenie nowej glowy na podstawie obecnej pozycji i kierunku
    if (new_head == NULL) {
        fprintf(stderr, "Error: Blad alokacji pamieci!\n");
        return;
    }
    else {
        copying_segment(new_head, first);
        move_snake_directions(new_head);

        new_head->nextPos = *first;                                                                         //nowa glowa jako poczatek listy
        *first = new_head;

        Snake* current = *first;
        while (current->nextPos != *last) {
            current = current->nextPos;
        }
        free(*last);                                                                                    //usuniecie ogona w celu zachowania dlugosci weza
        current->nextPos = NULL;
        *last = current;
    }
}

int isPos_valid(Snake** first, dot* Dot) {
    for (Snake* current = *first; current != NULL; current = current->nextPos) {                           //sprawdzenie czy jej pozycja nie koliduje z pozycja weza
        if (current->x == Dot->x && current->y == Dot->y) {
            return 1;
        }
    }
    return 0;
}

void generate_blueDot(dot* blueDot, Snake** first) {                                                    //funkcja generujaca niebieska kropke
    do {
        blueDot->x = rand() % GRID_ELEMENTS;
        blueDot->y = rand() % GRID_ELEMENTS;
    } while (isPos_valid(first, blueDot));                                                             //sprawdzenie kolizjiz wezem
}

void generate_bonus(dot* bonus, dot* blueDot, Snake** first) {
    do {                                                                                                //sprawdzenie kolizji
        bonus->x = rand() % GRID_ELEMENTS;
        bonus->y = rand() % GRID_ELEMENTS;
    } while ((bonus->x == blueDot->x && bonus->y == blueDot->y) || isPos_valid(first, bonus));
}

void check_blueDot(dot* blueDot, int* points, Snake** first, Snake** last) {                            //obluga zdjedzenia niebieskiej kropki
    if ((*first)->x == blueDot->x && (*first)->y == blueDot->y) {
        *points += BLUEDOT_POINTS;
        add_segment(last);
        generate_blueDot(blueDot, first);
    }
}

int get_snake_length(Snake** first) {                                                                   //funkcja pomocnicza zwracajaca obecna dlugosc weza
    int length = 0;
    Snake* current = *first;
    while (current != NULL) {
        length++;
        current = current->nextPos;
    }
    return length;
}

void check_bonus(dot bonus, double* snake_update_interval, bool* bonus_active, int* points, Snake** first, Snake** last) {
    if ((*first)->x == bonus.x && (*first)->y == bonus.y) {                                             //funkcja obslugujaca zjedzenie czerwonej kropki
        *bonus_active = false;
        *points += REDDOT_POINTS;

        int snake_length = get_snake_length(first);

        if (rand() % 2 == 0) {                                                                          //polowa szansy na zmniejszenie dlugosci lub zwolnienie czasu
            if (snake_length - 1 >= BONUS_SHORTEN) {
                for (int i = 0; i < BONUS_SHORTEN; i++) {
                    delete_segment(first, last);
                }
            }
            else {
                *snake_update_interval *= BONUS_SLOWDOWN;
            }
        }
        else {
            *snake_update_interval *= BONUS_SLOWDOWN;
        }
    }
}

void free_snake(Snake* element) {                                                                       //rekurencyjne wyczyszczenie pamieci weza
    if (element != NULL) {
        free_snake(element->nextPos);
        free(element);                                                                                  //zwolnienie bierzacego elementu
    }
}

void reset_game(Snake** first, Snake** last, dot* blueDot, double* worldTime, double* snake_update_interval, bool* bonus_active, int* points) {
    free_snake(*first);
    *last = NULL;
    *first = NULL;

    *first = init_snake();
    if (*first != NULL) {
        *last = *first;                                                                                 //glowa i ogon poczatkowo staniowa ten sam element
    }
    else {
        fprintf(stderr, "Nie udalo sie zainicjalizowac weza\n");
    }
    if (STARTING_LENGTH > 0) {
        for (int i = 0; i < STARTING_LENGTH; i++) {
            add_segment(last);
        }
    }

    generate_blueDot(blueDot, first);
    (*bonus_active) = false;                                                                            //reset wartosci zmiennych
    *worldTime = 0.0;
    *snake_update_interval = STARTING_SPEED;
    *points = 0;
}

void check_border(Snake** first) {                                                                      //obsluga zderzenia z krawedziami gry
    if ((*first)->direction == RIGHT && (*first)->x >= GRID_ELEMENTS - 1) {
        if ((*first)->y < GRID_ELEMENTS - 1) {
            (*first)->direction = DOWN;                                                                 //waz moze poruszyc sie w swoje prawo
        }
        else {
            (*first)->direction = UP;                                                                   //brak mozliwosci ruchu w prawa strone weza, wiec skreca w swoje lewo
        }
    }
    else if ((*first)->direction == LEFT && (*first)->x <= 0) {
        if ((*first)->y > 0) {
            (*first)->direction = UP;
        }
        else {
            (*first)->direction = DOWN;
        }
    }
    else if ((*first)->direction == DOWN && (*first)->y >= GRID_ELEMENTS - 1) {
        if ((*first)->x > 0) {
            (*first)->direction = LEFT;
        }
        else {
            (*first)->direction = RIGHT;
        }
    }
    else if ((*first)->direction == UP && (*first)->y <= 0) {
        if ((*first)->x < GRID_ELEMENTS - 1) {
            (*first)->direction = RIGHT;
        }
        else {
            (*first)->direction = LEFT;
        }
    }
}

// narysowanie napisu txt na powierzchni screen, zaczynajac od punktu (x, y)
// charset to bitmapa 128x128 zawierajaca znaki
void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset) {
    int px, py, c;
    SDL_Rect s, d;
    s.w = 8;
    s.h = 8;
    d.w = 8;
    d.h = 8;
    while (*text) {
        c = *text & 255;
        px = (c % 16) * 8;
        py = (c / 16) * 8;
        s.x = px;
        s.y = py;
        d.x = x;
        d.y = y;
        SDL_BlitSurface(charset, &s, screen, &d);
        x += 8;
        text++;
    };
};

// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt srodka obrazka sprite na ekranie
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
    SDL_Rect dest;
    dest.x = x - sprite->w / 2;
    dest.y = y - sprite->h / 2;
    dest.w = sprite->w;
    dest.h = sprite->h;
    SDL_BlitSurface(sprite, NULL, screen, &dest);
};

// rysowanie pojedynczego pixela
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
    int bpp = surface->format->BytesPerPixel;
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
    *(Uint32*)p = color;
};

// rysowanie linii o dlugosci l w pionie (gdy dx = 0, dy = 1) 
// badz poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
    for (int i = 0; i < l; i++) {
        DrawPixel(screen, x, y, color);
        x += dx;
        y += dy;
    };
};

// rysowanie prostokata o dlugosci bokow l i 
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
    DrawLine(screen, x, y, k, 0, 1, outlineColor);
    DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
    DrawLine(screen, x, y, l, 1, 0, outlineColor);
    DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
    for (int i = y + 1; i < y + k - 1; i++)
        DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

void DrawCircle(SDL_Surface* screen, int x0, int y0, int radius, int kolor) {                           //funkcja obslugujaca rysowanie kola potrzebnego do generowania niebieskiej i czerwonej kropki
    int y = 0;
    int decision = 1 - radius;                                                                          //parametr decyzyjny

    while (radius >= y) {
        DrawLine(screen, x0 - radius, y0 + y, 2 * radius + 1, 1, 0, kolor);                             //rysowanie poziomych linii miedzy punktami ktore tworza okrag wykorzystujac symetrie
        DrawLine(screen, x0 - radius, y0 - y, 2 * radius + 1, 1, 0, kolor);
        DrawLine(screen, x0 - y, y0 + radius, 2 * y + 1, 1, 0, kolor);
        DrawLine(screen, x0 - y, y0 - radius, 2 * y + 1, 1, 0, kolor);                                  //petla dziala do momentu gdy y nie osiagnie wartosci wiekszej niz promien
        //w kazdej iteracji petli algorytm rysuje cztery symetryczne punkty na okregu 
        y++;                                                                                            //sa one rozlozone w czterech cwiartkach okregu
        if (decision <= 0) {
            decision += 2 * y + 1;                                                                      //przesuniecie punktu o dwa pixele do gory na osi y
        }
        else {
            radius--;
            decision += 2 * (y - radius) + 1;                                                           //jesli wartosc decision jest wieksza niz 0 nalezy przesunac punkt w poziomie na osi x aby dopasowac okrag do ksztaltu
        }
    }
}

void draw_blueDot(SDL_Surface* screen, dot* blueDot, dot* border, int niebieski) {
    struct dot center;
    center.x = (border->x + blueDot->x * CELL_SIZE) + CELL_SIZE / 2;                                    //obliczanie srodka okregu
    center.y = (border->y + blueDot->y * CELL_SIZE) + CELL_SIZE / 2;

    DrawCircle(screen, center.x, center.y, CELL_SIZE / BLUEDOT_SIZE, niebieski);
}

void draw_bonus(SDL_Surface* screen, dot* bonus, dot* border, int czerwony) {
    struct dot center;
    center.x = (border->x + bonus->x * CELL_SIZE) + CELL_SIZE / 2;                                      //obliczanie srodka okregu
    center.y = (border->y + bonus->y * CELL_SIZE) + CELL_SIZE / 2;

    DrawCircle(screen, center.x, center.y, CELL_SIZE / BONUS_SIZE, czerwony);
}

void draw_snake(SDL_Surface* screen, Snake** first, dot* border, int zielony, int ciemno_zielony) {
    struct dot segment;                                                                                 //funkcja rysujaca glowe i cialo weza
    for (Snake* current = *first; current != NULL; current = current->nextPos) {                           //petla przechodzaca przez kazdy element az do NULL w celu narysowania calego weza 
        segment.x = border->x + current->x * CELL_SIZE;
        segment.y = border->y + current->y * CELL_SIZE;
        DrawRectangle(screen, segment.x, segment.y, CELL_SIZE, CELL_SIZE, ciemno_zielony, zielony);
    }
}

void draw_border(SDL_Surface* screen, dot* border, int kolor) {                                         //rysowanie granic gry
    DrawLine(screen, border->x, border->y, GRID_SIZE, 1, 0, kolor);
    DrawLine(screen, border->x, border->y + GRID_SIZE - 1, GRID_SIZE, 1, 0, kolor);
    DrawLine(screen, border->x, border->y, GRID_SIZE, 0, 1, kolor);
    DrawLine(screen, border->x + GRID_SIZE - 1, border->y, GRID_SIZE, 0, 1, kolor);
}

void draw_requirements(SDL_Surface* screen, SDL_Surface* charset) {                                     //funkcja rysujaca spelnione w projekcie wymagania
    const char* requirements[] = {
            "MANDATORY:",
            "1. Graphic design of the game",
            "2. Basic movement",
            "3. Snake hitting itself.",
            "4. Listed requirements",
            "", "",                                                                                     //spelnione wymagania:
            "OPTIONAL:",
            "A. Lengthening of the snake",
            "B. Speedup",
            "C. Red dot bonus",
            "D. Points",
            "F. Best scores",
    };
    int height = REQUIREMENTS_STARTING_HEIGHT;

    for (int i = 0; i < sizeof(requirements) / sizeof(requirements[0]); i++) {                          //wyswietlanie spelnionych wymagan na ekranie na odpowiedniej wysokosci
        DrawString(screen, screen->w / 2 - strlen(requirements[i]) * 8 / 2, height, requirements[i], charset);
        height += 20;
    }
}

void check_speedup(double worldTime, double* snake_update_interval, double* last_speedup_time) {
    if (worldTime - *last_speedup_time >= TIME_FOR_SPEEDUP) {                                           //funkcja obslugujaca zwalnianie oraz przyspieszanie czasu rozgrywki
        *snake_update_interval *= SPEEDUP_FACTOR;
        *last_speedup_time = worldTime;
    }
    if (*snake_update_interval < MAX_SPEED) {                                                           //brak mozliwosci zbyt szybkiego ruchu
        *snake_update_interval = MAX_SPEED;
    }
    if (*snake_update_interval > STARTING_SPEED) {                                                      //brak mozliwosci zbyt wolnego ruchu
        *snake_update_interval = STARTING_SPEED;
    }
}

void draw_timer(SDL_Surface* screen, Colors* colors, bool bonus_active, double* bonus_time_left) {
    double current_time = (*bonus_time_left) / BONUS_LIFETIME;                                          //obliczanie dlugosci paska bonusu
    int current_width = (int)(BONUS_TIMER_WIDTH * current_time);

    int x = (SCREEN_WIDTH - BONUS_TIMER_WIDTH) / 2;
    int y = SCREEN_HEIGHT - BONUS_TIMER_HEIGHT - GAP;

    DrawRectangle(screen, x - 1, y - 1, BONUS_TIMER_WIDTH + 1, BONUS_TIMER_HEIGHT + 1, colors->bialy, colors->szary);   //rysowanie obramowania paska bonusu
    DrawRectangle(screen, x, y, current_width, BONUS_TIMER_HEIGHT - 1, colors->czerwony, colors->czerwony);             //rysowanie paska bonusu
}

void update_screen(SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Surface* screen) {                  //funkcja oswiezajaca elementy na ekranie
    SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, scrtex, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void load_score(struct scores* scores) {                                                                //funkcja wczytujaca najlepsze wyniki
    FILE* file = fopen(BEST_SCORES_FILE, "r");
    if (file == NULL) {
        for (int i = 0; i < NUMER_OF_SCORES; i++) {
            strcpy(scores[i].name, "---");                                                              //zwraca '---' jesli plik nie istnieje lub nie ma zapisanych wynikow ani punktow
            scores[i].score = 0;
        }
        return;
    }
    else {
        for (int i = 0; i < NUMER_OF_SCORES; i++) {
            if (fscanf(file, "%s %d", scores[i].name, &scores[i].score) != 2) {
                strcpy(scores[i].name, "---");
                scores[i].score = 0;
            }
        }
    }
    fclose(file);
}

void save_score(struct scores* scores) {                                                                //funkcja obslugujaca zapis najlepszych wynikow do pliku
    FILE* file = fopen(BEST_SCORES_FILE, "w");
    if (file == NULL) {
        printf("Error saving high scores!");
        return;
    }
    for (int i = 0; i < NUMER_OF_SCORES; i++) {
        fprintf(file, "%s %d\n", scores[i].name, scores[i].score);
    }
    fclose(file);
}

void enter_name(SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Surface* screen, SDL_Surface* charset, char* name, int position) {
    SDL_Event event1;                                                                                   //funkcja obslugujaca wpisywanie imienia przez gracza ktory osiagnal topowy wynik
    char prompt[128];
    char text[128];
    int name_length = 0;
    bool done = false;
    name[0] = '\0';

    while (!done) {
        while (SDL_PollEvent(&event1)) {
            if (event1.type == SDL_KEYDOWN) {
                if (event1.key.keysym.sym == SDLK_RETURN) {                                             //enter zatwierdza wprowadzenie
                    done = true;
                }
                else if (event1.key.keysym.sym == SDLK_BACKSPACE && name_length > 0) {                  //usuwanie znakow poprzez backspace 
                    name_length--;
                    name[name_length] = '\0';
                }
            }
            else if (event1.type == SDL_TEXTINPUT) {                                                    //obsluguje wpisywanie tekstu
                if (name_length < MAX_NAME_LENGTH - 1) {
                    strcat(name, event1.text.text);
                    name_length++;
                }
            }
        }
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));                                //czyszczenie ekranu

        sprintf(text, "Congratulations! Your score is top %d. worldwide!", position + 1);
        DrawString(screen, SCREEN_WIDTH / 2 - strlen(text) * 8 / 2, (SCREEN_HEIGHT / 2) - (SCREEN_HEIGHT / 4), text, charset);

        sprintf(prompt, "Enter your name to continue");
        DrawString(screen, SCREEN_WIDTH / 2 - strlen(prompt) * 8 / 2, (SCREEN_HEIGHT / 2) - (SCREEN_HEIGHT / 4.5) + GAP, prompt, charset);

        DrawString(screen, SCREEN_WIDTH / 2 - name_length * 8 / 2, SCREEN_HEIGHT / 2, name, charset);   //wyswietlanie aktualnego stanu wpisywanego imienia
        update_screen(scrtex, renderer, screen);
    }
}

void update_score(SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Surface* screen, SDL_Surface* charset, struct scores* scores, int new_score) {
    int position = -1;
    char name[MAX_NAME_LENGTH];
    for (int i = 0; i < NUMER_OF_SCORES; i++) {                                                         //znalezienie pozycji dla nowego wyniku
        if (new_score > scores[i].score) {
            position = i;
            break;
        }
    }
    if (position == -1) {
        return;                                                                                         //brak miejsca dla tego wyniku
    }
    for (int i = NUMER_OF_SCORES - 1; i > position; i--) {                                              //przesuniecie wynikow jesli nowy jest lepszy niz poprzednie
        scores[i] = scores[i - 1];
    }

    enter_name(scrtex, renderer, screen, charset, name, position);                                      //oblsluga wpisywania nazwy gracza
    strcpy(scores[position].name, name);                                                                //dodanie nowego wyniku
    scores[position].score = new_score;
}

void display_score(struct scores* scores, SDL_Surface* screen, SDL_Surface* charset, int* points) {
    char prompt[128];                                                                                   //funkcja obslugujaca wyswietlanie tablicy najlepszych wynikow
    char text[128];
    char text2[128];
    char text3[128];
    int height = 20;

    sprintf(text2, "'ESC' - quit       'N' - start new game");
    DrawString(screen, screen->w / 2 - strlen(text2) * 8 / 2, GAP, text2, charset);

    sprintf(text, "TOP 3 SCORES:");
    DrawString(screen, SCREEN_WIDTH / 2 - strlen(text) * 8 / 2, (SCREEN_HEIGHT / 2) - (SCREEN_HEIGHT / 5), text, charset);

    sprintf(text3, "Your score: %d", *points);
    DrawString(screen, SCREEN_WIDTH / 2 - strlen(text3) * 8 / 2, (SCREEN_HEIGHT / 2) - (SCREEN_HEIGHT / 3) + 2 * GAP, text3, charset);

    for (int i = 0; i < NUMER_OF_SCORES; i++) {                                                         //petla wyswietlajaca najlepsze wyniki
        sprintf(prompt, "%d. %s: %d", i + 1, scores[i].name, scores[i].score);
        DrawString(screen, SCREEN_WIDTH / 2 - strlen(prompt) * 8 / 2, (SCREEN_HEIGHT / 2) - (SCREEN_HEIGHT / 6) + 2 * height, prompt, charset);
        height += 20;
    }
}

int check_collision(SDL_Window* window, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Surface* screen, SDL_Surface* charset, dot* blueDot, int* quit, double* worldTime, double* snake_update_interval, bool* bonus_active,
    dot* border, int* points, Snake** first, struct scores* high_scores, Colors* colors) {
    SDL_Event event2;                                                                                   //funkcja obslugujaca kolizje glowy weza z jego cialem
    Snake* current = (*first)->nextPos;
    bool goback = false;

    while (current != NULL) {
        if ((*first)->x == current->x && (*first)->y == current->y) {
            SDL_FillRect(screen, NULL, colors->czarny);                                                 //wyswitlenie i cala obsluga najlepszych wynikow
            load_score(high_scores);
            if (*points > high_scores[NUMER_OF_SCORES - 1].score) {
                update_score(scrtex, renderer, screen, charset, high_scores, *points);
                save_score(high_scores);
            }

            SDL_FillRect(screen, NULL, colors->czarny);
            display_score(high_scores, screen, charset, points);
            update_screen(scrtex, renderer, screen);

            while (!goback) {                                                                           //mozliwosc wyjscia lub rozpoczenia nowej gry
                while (SDL_PollEvent(&event2)) {
                    if (event2.type == SDL_QUIT) {                                                      //wyjscie z programu
                        goback = true;
                        *quit = 1;
                        break;
                    }
                    else if (event2.type == SDL_KEYDOWN) {
                        if (event2.key.keysym.sym == SDLK_ESCAPE) {                                     //wyjscie z programu
                            goback = true;
                            *quit = 1;
                            break;
                        }
                        if (event2.key.keysym.sym == SDLK_n) {                                          //rozpoczecie nowej gry
                            goback = true;
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        current = current->nextPos;
    }
    return 0;
}

void draw_info(SDL_Surface* screen, SDL_Surface* charset, char* text, int* points, Snake** first, Snake** last, dot* blueDot, double* worldTime, double* snake_update_interval, bool* bonus_active, double fps) {
    sprintf(text, "'ESC' - quit   'R' - show requirements   'N' - new game");                           //wyswietlani glowych informacji na temat gry; liczby punktow, czasu, fps itd...
    DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, GAP, text, charset);

    if (*points >= 18446744073709551615) {
        printf("You gained too many points");
        reset_game(first, last, blueDot, worldTime, snake_update_interval, bonus_active, points);
    }

    char points_text[50];
    sprintf(points_text, "Points: %d   ", *points);
    sprintf(text, "Time: %.1lf s   FPS: %.0lf ", *worldTime, fps);
    char combined_text[178];
    sprintf(combined_text, "%s%s", points_text, text);

    DrawString(screen, screen->w / 2 - strlen(combined_text) * 8 / 2, 3 * GAP, combined_text, charset);
}

void init_colors(SDL_Surface* screen, Colors* colors) {                                                 //funkcja obslugujaca wyswietlanie odpowiednich kolorow umieszczonych w strukturze
    colors->czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    colors->zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
    colors->czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
    colors->niebieski = SDL_MapRGB(screen->format, 0x00, 0x00, 0xFF);
    colors->szary = SDL_MapRGB(screen->format, 0x55, 0x55, 0x55);
    colors->ciemno_zielony = SDL_MapRGB(screen->format, 0x00, 0x80, 0x00);
    colors->bialy = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
}

void main_functions(SDL_Surface* screen, SDL_Surface* charset, SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* scrtex, char* text, Snake** first, Snake** last, double* worldTime, double* snake_update_interval,
    bool* bonus_active, dot* border, dot* blueDot, dot* bonus, double* bonus_time_left, double* delta, double* snake_update_timer, int* points, double* last_speedup_time, bool* show_info, int* quit, int* t1, Colors* colors, scores* high_scores) {
    //funkcja obslugujaca wszystkie glowne podfunkcje gry takie jak: rysowanie wszystkich elementow oraz sprawdzanie kolizji miedzy nimi
    draw_border(screen, border, colors->szary);
    draw_snake(screen, first, border, colors->zielony, colors->ciemno_zielony);
    draw_blueDot(screen, blueDot, border, colors->niebieski);

    if (rand() % CHANSE_FOR_REDDOT == 0 && !(*bonus_active)) {                                          //generowanie czerwoneej kropki (bonusu)
        generate_bonus(bonus, blueDot, first);
        *bonus_active = true;
        *bonus_time_left = BONUS_LIFETIME;
    }

    if (bonus_active != NULL && *bonus_active) {                                                        //wyswietlanie bonusu jesli jest aktywny
        *bonus_time_left -= *delta;
        draw_bonus(screen, bonus, border, colors->czerwony);
        check_bonus(*bonus, snake_update_interval, bonus_active, points, first, last);
        draw_timer(screen, colors, bonus_active, bonus_time_left);

        sprintf(text, "Lifetime of the bonus");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT - (BONUS_TIMER_HEIGHT)-GAP / 4, text, charset);
        if (*bonus_time_left <= 0) {
            (*bonus_active) = false;
        }
    }

    if (*snake_update_timer >= *snake_update_interval) {                                                //obsluga predkosci gry i odpowiednie wyswietlanie elementow
        *snake_update_timer -= *snake_update_interval;
        check_border(first);
        move_snake(first, last);
        check_blueDot(blueDot, points, first, last);
        if (check_collision(window, scrtex, renderer, screen, charset, blueDot, quit, worldTime, snake_update_interval, bonus_active, border, points, first, high_scores, colors)) {
            reset_game(first, last, blueDot, worldTime, snake_update_interval, bonus_active, points);
            *t1 = SDL_GetTicks();
        }
    }

    if (*show_info) {                                                                                   //przelacznik do pokazania spelnionych wymagan
        draw_requirements(screen, charset);
    }

    check_speedup(*worldTime, snake_update_interval, last_speedup_time);
    update_screen(scrtex, renderer, screen);                                                            //aktualizacja ekranu
}

void handle_events(SDL_Event* event, Snake** first, Snake** last, bool* show_info, int* quit, dot* blueDot, double* worldTime, double* snake_update_interval, bool* bonus_active, int* points) {
    while (SDL_PollEvent(event)) {                                                                      //glowna funkcja do obslugi kliknietych przez gracza przyciskow
        switch (event->type) {
        case SDL_QUIT:
            *quit = 1;                                                                                  //wyjscie z gry
            break;
        case SDL_KEYUP:
            break;
        case SDL_KEYDOWN:
            switch (event->key.keysym.sym) {
            case SDLK_ESCAPE:
                *quit = 1;                                                                              //wyjscie z gry
                break;
            case SDLK_UP:
                if ((*first)->direction != DOWN) {                                                       //ruch do gory
                    (*first)->direction = UP;
                }
                break;
            case SDLK_DOWN:
                if ((*first)->direction != UP) {                                                         //ruch w dol
                    (*first)->direction = DOWN;
                }
                break;
            case SDLK_LEFT:
                if ((*first)->direction != RIGHT) {                                                      //ruch w lewo
                    (*first)->direction = LEFT;
                }
                break;
            case SDLK_RIGHT:
                if ((*first)->direction != LEFT) {                                                       //ruch w prawo
                    (*first)->direction = RIGHT;
                }
                break;
            case SDLK_n:                                                                                //wczytanie nowej gry
                reset_game(first, last, blueDot, worldTime, snake_update_interval, bonus_active, points);
                break;
            case SDLK_r:                                                                                //przelacznik od pokazywania spelnionych wymagan
                *show_info = !(*show_info);
                break;
            }
            break;
        }
    }
}

void free_game(SDL_Surface* screen, SDL_Surface* charset, SDL_Surface* decoration, SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* scrtex) {
    SDL_FreeSurface(charset);                                                                           //funkcja powodujaca zwolnienie pamieci zajmowanej przez elementy SDL
    SDL_FreeSurface(screen);
    SDL_FreeSurface(decoration);
    SDL_DestroyTexture(scrtex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void init_game(SDL_Surface* screen, SDL_Surface* charset, Snake** first, Snake** last, Colors* colors, dot* border, dot* blueDot, int* t1) {
    SDL_SetColorKey(charset, true, 0x000000);                                                           //funkcja wczytujaca niektore podstawowe elementy potrzebne do zaladowania rozgrywki
    init_colors(screen, colors);

    border->x = CENTERED_BORDER_X;                                                                      //wysrodkowanie planszy do gry
    border->y = CENTERED_BORDER_Y;

    *first = init_snake();
    if (*first != NULL) {
        *last = *first;                                                                                 //glowa i ogon poczatkowo stanowia ten sam element
    }
    else {
        fprintf(stderr, "Nie udalo sie zainicjalizowac weza!\n");
    }
    if (STARTING_LENGTH > 0) {
        for (int i = 0; i < STARTING_LENGTH; i++) {
            add_segment(last);
        }
    }
    generate_blueDot(blueDot, first);
    *t1 = SDL_GetTicks();
}

void DrawSnakeDecorations(SDL_Surface* screen, SDL_Surface* decoration) {
    SDL_Rect left, right;                                                                               //funkcja rysujaca bitmapy zawierajace obrazy snake'ow

    left.w = decoration->w;                                                                             //obsluga lewej dekoracji
    left.h = decoration->h;
    left.x = GAP;
    left.y = (SCREEN_HEIGHT - left.h) / 2;

    right.w = left.w;                                                                                   //obsluga prawej dekoracji
    right.h = left.h;
    right.x = SCREEN_WIDTH - left.w - GAP;
    right.y = (SCREEN_HEIGHT - left.h) / 2;

    SDL_BlitSurface(decoration, NULL, screen, &left);                                                   //wyswietlanie bitmap na ekranie gry
    SDL_BlitSurface(decoration, NULL, screen, &right);
}

void game_loop(SDL_Surface* screen, SDL_Surface* charset, SDL_Surface* decoration, SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* scrtex, char* text, Snake** first, Snake** last, SDL_Event* event, double* worldTime,
    double* snake_update_interval, bool* bonus_active, dot* border, dot* blueDot, dot* bonus, double* bonus_time_left, double* delta, double* snake_update_timer, int* points, double* last_speedup_time,
    bool* show_info, int* quit, int* t1, Colors* colors, scores* high_scores, int* t2, double* fpsTimer, double* fps, int* frames) {
    while (!(*quit)) {                                                                                                                                                                                                                                       //glowna petla gry oblugujaca wiekszosc zdarzen na ekranie
        *t2 = SDL_GetTicks();
        *delta = (*t2 - *t1) * 0.001;                                                                                                                                                                                                   //wyswietlanie czasu
        *t1 = *t2;

        *worldTime += *delta;
        *snake_update_timer += *delta;                                                                                                                                                                                                  //obsluga predkosci gry

        SDL_FillRect(screen, NULL, colors->czarny);

        *fpsTimer += *delta;
        if (*fpsTimer > 0.5) {                                                                                                                                                                                                                   //obliczanie liczby klatek na sekunde
            *fps = *frames * 2;
            *frames = 0;
            *fpsTimer -= 0.5;
        };
        DrawSnakeDecorations(screen, decoration);                                                       //rysowanie bitmapy
        handle_events(event, first, last, show_info, quit, blueDot, worldTime, snake_update_interval, bonus_active, points);
        (*frames)++;                                                                                                                                                                                                                                    //obsluga podstawowych zdarzen przez klikanie klawiszy
        draw_info(screen, charset, text, points, first, last, blueDot, worldTime, snake_update_interval, bonus_active, *fps);
        //rysowanie postawowych informacji o rozgrywce
        main_functions(screen, charset, window, renderer, scrtex, text, first, last, worldTime, snake_update_interval, bonus_active, border, blueDot, bonus, bonus_time_left, delta, snake_update_timer, points,
            last_speedup_time, show_info, quit, t1, colors, high_scores);                               //funkcja zawierajaca wszystkie podfunkcje potrzebne do generowania obrazu gry
    }
}

int sdl_basic() {                                                                                       //ustawianie podstawowych wartosci biblioteki SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
}

int check_surface(SDL_Surface* surface, SDL_Surface* screen, SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* scrtex) {
    if (surface == NULL) {                                                                              //sprawdzanie poprawnosci wczytania bitmapy
        printf("SDL_LoadBMP(surface.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;
    }
}

int check_rc(int* rc) {                                                                                 //sprawdzenie poprawnosci rc
    if (*rc != 0) {
        SDL_Quit();
        printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
        return 1;
    };
}



#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
    srand(time(NULL));
    int t1 = 0, t2 = 0, quit = 0, frames = 0, rc = 0, points = 0;                                       //deklaracja wszystkich podstawowych zmiennych potrzebnych do inicjalizacji gry
    double delta = 0, worldTime = 0, fpsTimer = 0, fps = 0, snake_update_timer = 0.0, last_speedup_time = 0.0, bonus_time_left = 0.0, snake_update_interval = STARTING_SPEED;
    bool show_info = false, bonus_active = false;
    char text[128];
    SDL_Event event;
    SDL_Surface* screen, * charset, * decoration;
    SDL_Texture* scrtex;
    SDL_Window* window;
    SDL_Renderer* renderer;

    struct dot blueDot;
    struct dot bonus;
    struct dot border;

    struct scores high_scores[NUMER_OF_SCORES];

    Snake* first = NULL;
    Snake* last = NULL;
    Colors colors;

    sdl_basic();
    //rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);                // tryb pe?noekranowy
    rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    check_rc(&rc);

    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_SetWindowTitle(window, "Snake Project Pawel Hiszpanski 203455");
    screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    charset = SDL_LoadBMP("assets/cs8x8.bmp");                                                                       //utworzenie bitmapy do wyswietlania napisow
    check_surface(charset, screen, window, renderer, scrtex);
    decoration = SDL_LoadBMP("assets/snake.bmp");                                                                    //utworzenie bitmapy z obrazkami weza
    check_surface(decoration, screen, window, renderer, scrtex);

    init_game(screen, charset, &first, &last, &colors, &border, &blueDot, &t1);                                  //funkcja inizjalicujaca gre
    game_loop(screen, charset, decoration, window, renderer, scrtex, text, &first, &last, &event, &worldTime, &snake_update_interval, &bonus_active, &border, &blueDot, &bonus, &bonus_time_left, &delta, &snake_update_timer, &points,
        &last_speedup_time, &show_info, &quit, &t1, &colors, high_scores, &t2, &fpsTimer, &fps, &frames);       //funkcja wprowadzajaca cala glowna petle gry

    free_snake(first);

    free_game(screen, charset, decoration, window, renderer, scrtex);                                           //zwolnienie pamieci zajmowanej przez podstawowe elementy biblioteki SDL
    SDL_Quit();
    return 0;
};