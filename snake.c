#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

#define NUM_COLS 30
#define NUM_ROWS 10
#define GAME_OVER 1
#define FOOD_EATEN 2

struct termios orig_termios;
// This function set the terminal back to normal
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
// opening
void start(){
    printf("Snake\nMove with wsad\nPress Enter to continue...");
    getchar();
}
// This function helps with the keyboard
void enable_raw_mode(){
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;   // non aspettare caratteri
    raw.c_cc[VTIME] = 1;  // timeout 100ms
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

}

typedef struct snake {
    int index;
    struct snake *node;
} snake;

void print_grid(char *grid){
    for (int i = 0; i<NUM_COLS*NUM_ROWS; i++){
        printf("%c", grid[i]);
        if ((i + 1) % NUM_COLS == 0) {
            printf("\n");
        }
    }
}

void set_grid (char *grid){
    for (int i = 0; i < NUM_COLS * NUM_ROWS; i++){
        if (i < NUM_COLS || i >= NUM_COLS * (NUM_ROWS - 1)){
            grid[i] = '#';
        }
        else if (i % NUM_COLS == 0 || (i + 1) % NUM_COLS == 0){
            grid[i] = '#';
        }
        else {
            grid[i] = ' ';
        }
    }
}

snake *init_snake(char *grid, int index, char symbol){
    snake *head = malloc(sizeof(snake));
    if (head == NULL) {printf("errore"); exit(EXIT_FAILURE);} //check malloc errors
    head -> index = index;
    head -> node = NULL;
    grid[index] = symbol;
    return head;
}

int move_snake(snake *head, char *grid, char direction, int *tail_pos, int *punteggio){
    int is_head = 1;
    int old_head = head->index;
    int mangiato = 0;
    while (head != NULL){
        int temp=head->index;
        if (is_head == 1) {
            if (direction == 'd') {head->index++;}
            else if (direction == 'a') {head->index--;}
            else if(direction == 'w') {head->index -= NUM_COLS;}
            else if(direction == 's') {head->index += NUM_COLS;}
            if (grid[head->index] != ' ' && grid[head->index] != '*'){ return GAME_OVER;} 
            else if (grid[head->index] == '*'){mangiato = 1; (*punteggio)++;}
            is_head = 0;
            grid[head->index] = '@'; 
        } else {
            head->index = old_head;
            grid[head->index] = 'O';       
        }
        old_head = temp;
        head = head->node;
    }
    if (mangiato) {grid[old_head] = 'O'; *tail_pos = old_head; return FOOD_EATEN; } // keeps tail position to grow the snake

    grid[old_head] = ' ';
    return 0;
}
// free after malloc
void freeSnake(snake *head){
    while (head != NULL){
    snake *next = head->node;  
    free(head);                
    head = next;               
}
}

int main(){
    start();
    int punteggio = 0;
    printf("\033[2J\033[H"); //clear screen
    fflush(stdout);
    enable_raw_mode();
    char grid[NUM_COLS*NUM_ROWS];
    set_grid(grid);
    // set the initial snake of three nodes
    snake *snake_head = init_snake(grid, (NUM_ROWS / 2) * NUM_COLS + (NUM_COLS / 2), '@');
    snake_head->node = init_snake(grid, (NUM_ROWS / 2) * NUM_COLS + (NUM_COLS / 2) -1 , 'O');
    snake_head->node->node = init_snake(grid, (NUM_ROWS / 2) * NUM_COLS + (NUM_COLS / 2) -2, 'O');
    
    char direzione = 'd';
    int pos = 0;
    int tail_pos = 0; 
    
    srand(time(NULL)); //change seed every time, works with time.h
    while (grid[pos] != ' '){pos = rand() % (NUM_COLS * NUM_ROWS);}
    grid[pos] = '*';
    while(direzione != 'q'){
        char ch;

        int n = read(STDIN_FILENO, &ch, 1);
        int tasto = (n == 1) ? ch : -1;
        if (tasto != -1){
            if (!((tasto == 'a' && direzione == 'd') ||
          (tasto == 'd' && direzione == 'a') ||
          (tasto == 'w' && direzione == 's') ||
          (tasto == 's' && direzione == 'w'))){
            direzione = tasto;}
        }
        printf("\033[H");
        fflush(stdout);
        int status_check = move_snake(snake_head, grid, direzione, &tail_pos, &punteggio); //& is for changing the global value
        if (status_check == GAME_OVER){
            printf("\033[H");
            fflush(stdout);
            printf("\033[%d;1H\033[2K", NUM_ROWS + 2);
            printf("GAME OVER\nScore %d\n", punteggio);
            freeSnake(snake_head);
            exit(0);
        } else if (status_check == FOOD_EATEN){
            int pos = 0;
            while (grid[pos] != ' '){pos = rand() % (NUM_COLS * NUM_ROWS);}
            grid[pos] = '*';
            snake *temp = snake_head;
            while (temp->node != NULL){
                temp = temp->node;
            }
            temp->node = init_snake(grid, tail_pos, 'O');
        }
        print_grid(grid);
        printf("\033[1;1HScore: %d", punteggio);
        fflush(stdout);
        int delay = 150000 - punteggio * 10000;
        if (delay < 30000) delay = 30000;
        usleep(delay);

        }
    freeSnake(snake_head);
    return 0;
}
