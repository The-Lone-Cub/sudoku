#ifndef DIFFICULTY_SETTINGS_H
#define DIFFICULTY_SETTINGS_H

#include <SDL2/SDL.h>

struct Slide
{
    SDL_Rect slider;
    float value;
    bool isDragging;
};

class DifficultySettings
{
public:
    static Slide *getDifficultySlider() { return &difficultySlider; }

private:
    static Slide difficultySlider;
};

#endif // DIFFICULTY_SETTINGS_H