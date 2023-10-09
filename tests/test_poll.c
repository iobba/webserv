#include <ncurses.h>
#include <cstdlib>
#include <csignal>

// Signal handler function
void sigintHandler(int signal) {
    endwin(); // Restore terminal settings
    std::exit(0); // Exit the program gracefully
}

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    printw("Press Ctrl-C to exit\n");
    refresh();

    // Register the SIGINT signal handler
    std::signal(SIGINT, sigintHandler);

    while (true) {
        int ch = getch();
        // Continue processing other keys if needed
    }

    endwin();
    return 0;
}
