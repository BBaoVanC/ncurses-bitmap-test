#include <ncurses.h>
#include <stddef.h>

static void set_pixel(const size_t x, const size_t y, const bool color) {
    attron(COLOR_PAIR(color));
    mvprintw(y, x * 2, "a ");
    attroff(COLOR_PAIR(color));
}
// TODO: check for off-by-one errors
static void draw(const size_t size) {
    // top_radius/size = 8/128
    // This allows it to be scaled to any `size`,
    // while using position measurements based on pixels of 128x128 palera1n logo PNG
    const double top_radius = (size * 8 / 128);
    const double bottom_radius = (size * 36 / 128);

    // global center x TODO: rename?
    const double center_x = size / 2;

    // TOP CIRCLE
    for (size_t y = 0; y < top_radius; y++) {
        // the y coordinate of centerpoint of circle
        // TODO: rename? this is local not global
        const double center_y = top_radius;
        for (size_t x = 0; x < size; x++) {
            // circle
            if (((x - center_x) * (x - center_x)) + ((y - center_y) * (y - center_y)) <= (top_radius * top_radius)) {
                set_pixel(x, y, true);
            }
        }
    }

    // TRANSITION
    const double radius_start = top_radius;
    const double radius_end = bottom_radius;

    const double transition_begin_y = top_radius;
    const double transition_end_y = size - top_radius - bottom_radius;
    // difference in radii
    const double transition_radius_diff = radius_end - radius_start;
    // TODO: if size < 128, this might overwrite the same line multiple times, but who cares
    for (size_t y = transition_begin_y; y < transition_end_y; y++) {
        // the y value through the transition; remember that y is not starting at 0
        const double transition_pos = y - transition_begin_y;
        // how far through the transition we are
        const double transition_factor = transition_pos / (transition_end_y - transition_begin_y);
        //mvprintw(y, 0, "%f", transition_length);
        //continue;
        // could be const but that's confusing if you read it in English
        const double radius = radius_start + (transition_factor * transition_radius_diff);

        const double x_start = center_x - radius;
        const double x_end = center_x + radius;
        for (size_t x = x_start; x < x_end; x++) {
            set_pixel(x, y, true);
        }
    }

    // BOTTOM CIRCLE
    for (size_t y = transition_end_y; y < size; y++) {
        // TODO: rename? this is local not global
        const double center_y = size - bottom_radius;
        for (size_t x = 0; x < size; x++) {
            if (((x - center_x) * (x - center_x)) + ((y - center_y) * (y - center_y)) <= (bottom_radius * bottom_radius)) {
                set_pixel(x, y, true);
            }
        }
    }

    return;
}
int main() {
    initscr();
    start_color();
    curs_set(0);

    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_BLACK, COLOR_WHITE);

    static const size_t max = 128;
    int ch;
    while (TRUE) {
        erase();
        draw(max);
        refresh();
        ch = getch();
        if (ch == KEY_RESIZE) {
            continue;
        }
        break;
    }
    endwin();

    return 0;
}
