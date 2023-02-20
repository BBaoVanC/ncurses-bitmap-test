#include <ncurses.h>
#include <stdint.h>

static void set_pixel(const uint32_t x, const size_t y) {
    attron(COLOR_PAIR(1));
    mvprintw(y, x * 2, "a ");
    attroff(COLOR_PAIR(1));
}
static void set_pixel_off(const uint32_t x, const size_t y) {
    attron(COLOR_PAIR(0));
    mvprintw(y, x * 2, "a ");
    attroff(COLOR_PAIR(0));
}
// TODO: check for off-by-one errors
// also: do you really trust my floating point comparisons? or is it just magic?
//
// remember: x and y positions in here are real native coordinates on display
static void draw(const uint32_t size) {
    // top_radius/size = 8/128
    // This allows it to be scaled to any `size`,
    // while using position measurements based on pixels of 128x128 palera1n logo PNG
    const double top_radius = (size * 8 / 128);
    const double bottom_radius = (size * 36 / 128);

    // global center x TODO: rename?
    const double center_x = size / 2;

    // TOP CIRCLE
    for (uint32_t y = 0; y < top_radius; y++) {
        // the y coordinate of centerpoint of circle
        // TODO: rename? this is local not global
        const double center_y = top_radius;
        for (uint32_t x = 0; x < size; x++) {
            // circle
            if (((x - center_x) * (x - center_x)) + ((y - center_y) * (y - center_y)) <= (top_radius * top_radius)) {
                set_pixel(x, y);
            }
        }
    }

    // TRANSITION
    const double radius_start = top_radius;
    const double radius_end = bottom_radius;

    const double transition_begin_y = top_radius;
    const double transition_end_y = size - bottom_radius;
    // difference in radii
    const double transition_radius_diff = radius_end - radius_start;
    // TODO: if size < 128, this might overwrite the same line multiple times, but who cares
    for (uint32_t y = transition_begin_y; y < transition_end_y; y++) {
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
        for (uint32_t x = x_start + 1; x < x_end; x++) { // TODO: why does this need to be + 1, probably because < instead of <=? since otherwise off by one on left side only
            set_pixel(x, y);
        }
    }

    // BOTTOM CIRCLE
    for (uint32_t y = transition_end_y; y < size; y++) {
        // TODO: rename? this is local not global
        const double center_y = size - bottom_radius;
        for (uint32_t x = 0; x < size; x++) {
            if (((x - center_x) * (x - center_x)) + ((y - center_y) * (y - center_y)) <= (bottom_radius * bottom_radius)) {
                set_pixel(x, y);
            }
        }
    }
}
int main() {
    initscr();
    start_color();
    curs_set(0);

    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_BLACK, COLOR_WHITE);

    uint32_t max = 64;
    int ch;
    while (TRUE) {
        erase();
        draw(max);
        mvprintw(0, 0, "%i", max);
        refresh();
        ch = getch();
        switch (ch) {
            case KEY_RESIZE:
                break; // don't do anything
            case '=':
            case '+':
                max++;
                break;
            case '-':
                max--;
                break;
            default:
                goto end;
        }
    }
end:
    endwin();

    return 0;
}
