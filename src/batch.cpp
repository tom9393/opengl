#include "../include/batch.h"

void Batch::render()
{
    shader->Enable();
    shader->setup();
    for (unsigned int i = 0; i < drawables.size(); i++)
    {
        drawables.at(i)->draw(*shader);
    }
    shader->cleanup();
}
