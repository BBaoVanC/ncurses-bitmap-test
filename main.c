#include <ncurses.h>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

static double *get_pixel(double *pixels, uint32_t size, uint32_t x, uint32_t y) {
    uint32_t bitmap_index = (size * y) + x;
    return &pixels[bitmap_index];
}
// TODO: check for off-by-one errors
// also: do you really trust my floating point comparisons? or is it just magic?
//
// remember: x and y positions in here are real native coordinates on display
static void draw_logo_to_bitmap(double *pixels, uint32_t size) {
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
        double center_y = top_radius;
        for (uint32_t x = 0; x < size; x++) {
            // circle
            if (((x - center_x) * (x - center_x)) + ((y - center_y) * (y - center_y)) <= (top_radius * top_radius)) {
                *get_pixel(pixels, size, x, y) = 1;
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
        double transition_pos = y - transition_begin_y;
        // how far through the transition we are
        double transition_factor = transition_pos / (transition_end_y - transition_begin_y);
        double radius = radius_start + (transition_factor * transition_radius_diff);

        double x_start = center_x - radius;
        double x_end = center_x + radius;
        for (uint32_t x = x_start + 1; x < x_end; x++) { // TODO: why does this need to be + 1, probably because < instead of <=? since otherwise off by one on left side only
            *get_pixel(pixels, size, x, y) = 1;
        }
    }

    // BOTTOM CIRCLE
    for (uint32_t y = transition_end_y; y < size; y++) {
        // TODO: rename? this is local not global
        double center_y = size - bottom_radius;
        for (uint32_t x = 0; x < size; x++) {
            if (((x - center_x) * (x - center_x)) + ((y - center_y) * (y - center_y)) <= (bottom_radius * bottom_radius)) {
                *get_pixel(pixels, size, x, y) = 1;
            }
        }
    }
}
// in_size must be multiple of out_size or else something bad probably happens idk
static void downsample_bitmap(double *in_pixels, uint32_t in_size, double *out_pixels, uint32_t out_size) {
    // this is integer division because in_size will always be a multiple of out_size, resulting in a whole number
    uint32_t sample_size = in_size / out_size;
    for (uint32_t out_x = 0; out_x < out_size; out_x++) {
        for (uint32_t out_y = 0; out_y < out_size; out_y++) {
            uint32_t sample_start_x = out_x * sample_size;
            uint32_t sample_start_y = out_y * sample_size;
            uint32_t sample_limit_x = sample_start_x + sample_size;
            uint32_t sample_limit_y = sample_start_y + sample_size;

            double sample_value = 0;
            for (uint32_t in_x = sample_start_x; in_x < sample_limit_x; in_x++) {
                for (uint32_t in_y = sample_start_y; in_y < sample_limit_y; in_y++) {
                    sample_value += *get_pixel(in_pixels, in_size, in_x, in_y);
                }
            }

            double sample_count = sample_size * sample_size;
            sample_value /= sample_count;

            *get_pixel(out_pixels, out_size, out_x, out_y) = sample_value;
        }
    }
}
int main() {
    initscr();
    start_color();
    curs_set(0);

    // init_pair(0, COLOR_WHITE, COLOR_BLACK);
    // init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(0, COLOR_WHITE, 0);
    init_pair(1, COLOR_WHITE, 8);
    init_pair(2, COLOR_BLACK, 7);
    init_pair(3, COLOR_BLACK, 15);

    uint32_t size = 64;
    int ch;
    while (TRUE) {
        erase();

        // supersample bitmap
        uint32_t super_size = size * 2;
        double *super_pixels = calloc(super_size * super_size, sizeof(double));
        draw_logo_to_bitmap(super_pixels, super_size);

        // actual output bitmap
        double *pixels = calloc(size * size, sizeof(double));
        downsample_bitmap(super_pixels, super_size, pixels, size);

        /*
        for (uint32_t x = 0; x < super_size; x++) {
            for (uint32_t y = 0; y < super_size; y++) {
                double color = *get_pixel(super_pixels, super_size, x, y);
                int color_pair = color * 100 / 25;
                if (color_pair > 3) {
                    color_pair = 3;
                }
                attron(COLOR_PAIR(color_pair));
                //mvprintw(y, x * 2, "%2.0f", color * 100);
                mvprintw(y, x * 2, "  ");
                attroff(COLOR_PAIR(color_pair));
            }
        }
        */

        for (uint32_t x = 0; x < size; x++) {
            for (uint32_t y = 0; y < size; y++) {
                double color = *get_pixel(pixels, size, x, y);
                int color_pair = color * 100 / 25;
                if (color_pair > 3) {
                    color_pair = 3;
                }
                attron(COLOR_PAIR(color_pair));
                mvprintw(y, x * 2, "%2.0f", color * 100);
                //mvprintw(y, x * 2, "  ");
                attroff(COLOR_PAIR(color_pair));
            }
        }

        free(super_pixels);
        free(pixels);

        mvprintw(0, 0, "%i", size);
        refresh();

        ch = getch();
        switch (ch) {
            case KEY_RESIZE:
                break; // don't do anything
            case '=':
            case '+':
                size++;
                break;
            case '-':
                size--;
                break;
            default:
                goto end;
        }
    }
end:
    endwin();

    return 0;
}
