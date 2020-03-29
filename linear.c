

/*
 *      Linear.c
 *      By Jason Stewart
 * 
 *      A Linear Regression Modeling Program
 * 
 *      AP Computer Science Principles Create Task Submission
 * 
 *      Program characteristics:
 *          -Inputs coordinate data from user
 *          -Plots and displays coordinate data
 *          -Calculates linear regression model (y = mx + b)
 *          -Calculates Correlation Coefficient (r)
 *          -Calculates Coefficient of Determination (r^2)
 * 
 */

/*
 *      TODO:
 *          -Quality Assurance (QA) tests
 *          -Edge Cases
 * 
 *      NOTES:
 *          -Plot data stored on Linked List
 *          -5 Windows, all declared globally
 *          -Nothing is displayed on STDSCR, only on windows
 *          -LINES and COLS are constants provided by ncurses and relate to STDSCR
 *          -LOGFILE for debugging
 *          -Messages are displayed on data window
 *          -Color() changes all existing characters in a window
 *          -No Memory Leaks
 * 
 * 
 */


//Libraries  
#include <ctype.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


//Function Prototypes
bool startup(void);
void createWin(void);
void plotPoint(int x, int y);
void getPoints(void);
void getConstraints(void);
int  getUser(void);
void calcRegression(void);
void displayData(float a, float b, float r, float r2);
void logMove(float n);
void wipe(WINDOW *w);
void color(int key, int fg, int bg);
void createHeader(void);


//Names of our windows
WINDOW *graph;
WINDOW *ui;
WINDOW *display;
WINDOW *data;
WINDOW *header;


//window keys
enum windows { STDSCR = 1, GRAPH, UI, DISPLAY, DATA, HEADER };


//linked list to store all of our points
typedef struct node {

    int x, y;
    struct node *next;

} node;


//function prototype
void unload(struct node* head);


//struct wrapper for our global variables
struct {

    //size of our graph
    int max_y, max_x;
    int min_y, min_x;

    //number of points
    int num_p;

    node *head;

} g;


int main(int argc, char *argv[]) {

    //begin ncurses
    if (!startup()) {
        fprintf(stderr, "ERROR SETTING UP NCURSES\n");
        return 1;
    }

    refresh();

    createWin();

    getConstraints();

    getPoints();

    calcRegression();

    unload(g.head);

    //wait for user to exit the program
    while (getch() != 'q') { }

    //shutdown
    endwin();
    return 0; 

}


//begin ncurses
bool startup(void) {

    if (initscr() == NULL) return false;

    if (noecho() == ERR || raw() == ERR || keypad(stdscr, true) == ERR) {
        endwin();
        return false;
    }

    //create Colors
    if (has_colors()) start_color();

    //Minimum terminal size needed for program
    if (LINES < 50 || COLS < 149) {
        endwin();
        fprintf(stderr, "ERROR: TERMINAL SIZE TOO SMALL\n");
        return false;
    }

    return true;

}


//Create and display all of our windows
void createWin(void) {

    //make the graph
    graph = newwin(LINES/2, COLS/2, LINES/4, COLS/8);
    wipe(graph);

    //make the ui
    ui = newwin(LINES/6, COLS/2, LINES - LINES/6, COLS/8);
    wipe(ui);

    //make display
    display = newwin(LINES/2, COLS/4, LINES/4, COLS/1.4);
    wipe(display);

    //make display for the data
    data = newwin(LINES/6, COLS/4, LINES - LINES/6, COLS/1.4);
    wipe(data);

    //make header for decoration and title
    header = newwin(LINES/6, COLS/2, LINES/20, COLS/4);
    wipe(header);

    createHeader();

    return;

}


//get constraints for our graph
void getConstraints(void) {
    
    wclear(ui);
    box(ui, 0, 0);

    //Get Max Y constraint
    mvwaddstr(ui, 1, 1, "Enter max y constraint: ");
    wrefresh(ui);

    g.max_y = getUser();

    wclear(ui);
    box(ui, 0, 0);

    //Get Max X constraint
    mvwaddstr(ui, 1, 1, "Enter max x constraint: ");
    wrefresh(ui);

    g.max_x = getUser();

    //Get min Y constraint
    while (true) {

        wclear(ui);
        box(ui, 0, 0);

        mvwaddstr(ui, 1, 1, "Enter min y constraint: ");
        wrefresh(ui);

        g.min_y = getUser();

        color(DATA, COLOR_WHITE, COLOR_BLACK);
        wipe(data);

        //make sure constraint is valid
        if (g.min_y >= g.max_y) {

            color(DATA, 160, COLOR_BLACK);
            
            //print Error Message
            mvwaddstr(data, 1, 1, "Error: Minimum Y constraint cannot");
            mvwaddstr(data, 2, 1, "be larger than maximum Y constraint");
            mvwprintw(data, 3, 1, "Maximum Y constraint is: %d", g.max_y);
            wrefresh(data);

        } else {
            break;
        }
    } 

    //Get min X constraint
    while (true) {

        wclear(ui);
        box(ui, 0, 0);

        mvwaddstr(ui, 1, 1, "Enter min x constraint: ");
        wrefresh(ui);

        g.min_x = getUser();
        //make sure contraint is valid

        color(DATA, COLOR_WHITE, COLOR_BLACK);
        wipe(data);

        //make sure constraint is valid
        if (g.min_x >= g.max_x) {

            color(DATA, 160, COLOR_BLACK);
            
            //print Error Message
            mvwaddstr(data, 1, 1, "Error: Minimum X constraint cannot");
            mvwaddstr(data, 2, 1, "be larger than maximum X constraint");
            mvwprintw(data, 3, 1, "Maximum X constraint is: %d", g.max_x);
            wrefresh(data);

        } else {
            break;
        }
    }
    

    //print y axis
    float y_axis = ((LINES/2) - ((float) (0) - (g.min_y)) / (g.max_y - g.min_y) * (LINES/2));

    for (int i = 0; i < (COLS/2); i++) {
        mvwaddch(graph, (int) y_axis, i, '-');
    }

    //print x axis
    float x_axis = (((float) (0)) - g.min_x)/(g.max_x - g.min_x) * (COLS/2);;

    for (int i = 0; i < (COLS/2); i++) {
        mvwaddch(graph, i, (int) x_axis, '|');
    }

    wrefresh(graph);

    return;


}


//get a number from a user using ncurses
int getUser(void) {

    bool is_neg = false;
    int ch;
    int x = 0, y = 0, total = 0;

    getyx(ui, y, x);

    do {

        ch = getch();

        //exit if q
        if (ch == 'q') {
            endwin();
            exit(0);
        }

        //get numbers
        if (ch >= '0' && ch <= '9') {
            total *= 10;
            total -= (is_neg ? ch - '0' : (ch - '0') * -1);
        //get negative
        } else if (ch == '-') {
            is_neg = !is_neg;
            total *= -1;
        //get Backspace
        } else if (ch == 127 || ch == 8) {
            total /= 10;
        }

        //display number
        mvwprintw(ui, y, x, "%d ", total);
        wrefresh(ui);

    //end once we press ENTER
    } while (ch != 10);

    return total;

}


//Get plot data from user and add them to the linked list
void getPoints(void) {

    g.head = NULL;
    g.num_p = 0;

    while(true) {

        //See if the User wants to enter another point
        if (g.num_p >= 2) {

            //Ask the User
            mvwaddstr(ui, 1, 1, "Press N or ENTER to add new Coordinate");
            mvwaddstr(ui, 3, 1, "Press any other character to calculate");
            wrefresh(ui);

            //get user Input
            int ch = getch();
            if (ch == 'q') {
                endwin();
                exit(0);
            } else if (ch != 'n' && ch != 10) {
                wipe(ui);
                break;
            }
        }

        g.num_p++;

        //create a new node
        node *temp = (node*)malloc(sizeof(node));

        //make sure new node worked
        if (temp == NULL) {
            unload(g.head);
            endwin();
            fprintf(stderr, "MEMORY ERROR");
            exit(1);
        }

        //get x coordinate
        while (true) {
            wclear(ui);
            box(ui, 0, 0);

            mvwaddstr(ui, 1, 1, "Enter x coordinate of point: ");
            wrefresh(ui);

            temp->x = getUser();

            color(DATA, COLOR_WHITE, COLOR_BLACK);
            wipe(data);

            //make sure constraint is valid
            if (temp->x < g.min_x || temp->x > g.max_x) {

                color(DATA, 160, COLOR_BLACK);
            
                //print Error Message
                mvwaddstr(data, 1, 1, "Error: X coordinate must be between");
                mvwaddstr(data, 2, 1, "minimum and maximum X constraints");
                mvwprintw(data, 3, 1, "Minimum X constraint is: %d", g.min_x);
                mvwprintw(data, 4, 1, "Maximum X constraint is: %d", g.max_x);
                wrefresh(data);

            } else {
                break;
            }

        } 

        color(DISPLAY, 46, COLOR_BLACK);

        //put our value in our display
        mvwprintw(display, g.num_p + 1, 1, "%d, ", temp->x);
        wrefresh(display);


        //get y coordinate
        while (true) {
            wclear(ui);
            box(ui, 0, 0);

            mvwaddstr(ui, 1, 1, "Enter y coordinate of point: ");
            wrefresh(ui);

            temp->y = getUser();

            color(DATA, COLOR_WHITE, COLOR_BLACK);
            wipe(data);

            //make sure constraint is valid
            if (temp->y < g.min_y || temp->y > g.max_y) {

                color(DATA, 160, COLOR_BLACK);
            
                //print Error Message
                mvwaddstr(data, 1, 1, "Error: Y coordinate must be between");
                mvwaddstr(data, 2, 1, "minimum and maximum Y constraints");
                mvwprintw(data, 3, 1, "Minimum Y constraint is: %d", g.min_y);
                mvwprintw(data, 4, 1, "Maximum Y constraint is: %d", g.max_y);
                wrefresh(data);

            } else {
                break;
            }
        }

        color(DISPLAY, 46, COLOR_BLACK);

        //put our value in our display
        wprintw(display, "%d", temp->y);
        wrefresh(display);

        plotPoint(temp->x, temp->y);

        //join our new node
        temp->next = g.head;

        g.head = temp;

    }

    return;
}


//put a point on the graph
void plotPoint(int x, int y) {

    //determine positioning of the point on the graph
    float x_pos = (((float) (x)) - g.min_x)/(g.max_x - g.min_x) * (COLS/2);
    float y_pos = ((LINES/2) - ((float) (y) - (g.min_y)) / (g.max_y - g.min_y) * (LINES/2));

    color(GRAPH, 45, COLOR_BLACK);

    //add the character
    mvwaddch(graph, y_pos, x_pos, '+');
    wrefresh(graph);

    return;

}


//Recursively delete the linked list
void unload(struct node* head) {

    if (head == NULL) return;

    unload(head->next);

    free(head);
}


//calculate linear regression model and R values
void calcRegression(void) {

    float sum_x = 0, sum_x2 = 0, sum_y = 0, sum_y2 = 0, sum_xy = 0;

    //calculate sums of all values
    node *curr = g.head;
    while (curr != NULL) {
        sum_x += curr->x;
        sum_x2 += (curr->x * curr->x);
        sum_y += curr->y;
        sum_y2 += (curr->y * curr->y);
        sum_xy += (curr->x * curr-> y);

        //move onto the next node
        curr = curr->next;
    }

    //calculate slope
    float a = (g.num_p * sum_xy - sum_x * sum_y) / (g.num_p * sum_x2 - sum_x * sum_x);

    //calculate y intercept
    float b = (sum_y - a * sum_x) / g.num_p;

    //calculate pearson correlation coefficient
    float r = (float) (g.num_p * sum_xy - sum_x * sum_y);
    r /= sqrt((g.num_p * sum_x2 - sum_x * sum_x) * (g.num_p * sum_y2 - sum_y * sum_y));

    if (isnan(r) && a == 0) r = 1;

    //calculate coefficient of determination
    float r2 = r * r;

    displayData(a, b, r, r2);

    return;

}


//prints the data to the display
void displayData(float a, float b, float r, float r2) {

    color(DATA, 45, COLOR_BLACK);

    //print equation
    mvwaddstr(data, 1, 1, "Line of Best Fit-");
    mvwprintw(data, 2, 1, "Y = %0.4f x + %0.4f", a, b);

    //print pearson correlation coefficient
    mvwaddstr(data, 3, 1, "Pearson Correlation Coefficient-");
    mvwprintw(data, 4, 1, "%0.8f", r);

    //print Coefficient of determination
    mvwaddstr(data, 5, 1, "Coefficient of Determination-");
    mvwprintw(data, 6, 1, "%0.8f", r2);

    wrefresh(data);

    //Graph line of best fit
    float y_pos, x_pos;
    
    //compare relative pixels to actual pixels
    float scale = (abs(g.max_x) + abs(g.min_x)) / ((float) (COLS/2));


    //graph for every X
    for (float i = g.min_x; i <= g.max_x; i += scale) {


        //calculate positions
        x_pos = (i - g.min_x)/(g.max_x - g.min_x) * (COLS/2);
        y_pos = a * i + b;
        y_pos =  ((LINES/2) - (y_pos - (g.min_y)) / (g.max_y - g.min_y) * (LINES/2));

        color(GRAPH, 45, COLOR_BLACK);

        mvwaddch(graph, y_pos, x_pos, '*');
        
    }

    //hide cursor
    curs_set(0);

    wrefresh(graph);

    return;
}


//log anything for debugging
void logMove(float n) {

    // create log
    FILE *fp = fopen("log.txt", "a");
    if (fp == NULL) return;

    //print number
    fprintf(fp, "%0.2f\n", n);

    fclose(fp);

    return;
}


//sequencing for completely clearing a window
void wipe(WINDOW *w) {
    wclear(w);
    box(w, 0, 0);
    wrefresh(w);

    return;
}


//Key is linked to window and color pair assigned to that window
//Foreground and Background are desired colors
void color(int key, int fg, int bg) {


    if (!has_colors()) return;


    WINDOW *w;


    //link up the keys with the correct window
    switch (key) {

        case STDSCR:
            w = stdscr;
            break;

        case GRAPH:
            w = graph;
            break;

        case DATA:
            w = data;
            break;

        case UI:
            w = ui;
            break;

        case DISPLAY:
            w = display;
            break;

        case HEADER:
            w = header;
            break;

    }
    
    init_pair(key, fg, bg);

    //begin or end desired attribute
    wattrset(w, COLOR_PAIR(key));

    return;

}


//populates the header window
void createHeader(void) {

    int center;
    char *str;

    //nice
    color(HEADER, 69, COLOR_BLACK);

    wattron(header, A_BOLD);

    str = "Linear.c";
    center = ((COLS/2) - strlen(str)) / 2;
    mvwaddstr(header, 2, center, str);

    wattroff(header, A_BOLD);

    str = "By Jason Stewart @ 2020";
    center = ((COLS/2) - strlen(str)) / 2;
    mvwaddstr(header, 4, center, str);

    str = "AP Computer Science Principles Create Task Submission";
    center = ((COLS/2) - strlen(str)) / 2;
    mvwaddstr(header, 5, center, str);

    str = "A Linear Regression Modeling Program";
    center = ((COLS/2) - strlen(str)) / 2;
    mvwaddstr(header, 6, center, str);

    str = "QUIT PROGRAM: Q";
    center = ((COLS/2) - strlen(str)) / 2;
    mvwaddstr(header, LINES/6 - 2, 1, str);

    wrefresh(header);

    return;

}