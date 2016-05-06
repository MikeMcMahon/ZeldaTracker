#ifndef GAME_MATH_H
#define GAME_MATH_H


/********************************************//**
 * @brief
 * Determines if the points x/y collides with or is within the boundaries of the given x2/y2/h/w rectangle
 * @param x int
 * @param y int
 * @param x2 int
 * @param y2 int
 * @param w int
 * @param h int
 * @return int
 * 1 if collides, 0 if not
 ***********************************************/
int point_collides(int x, int y, int x2, int y2, int w, int h) {
    if (x >= x2 && x <= x2 + w)
        if (y >= y2 && y <= y2 + h)
            return 1;

    return 0;
}
#endif // GAME_MATH_H
