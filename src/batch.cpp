#include "../include/batch.h"
#include "../include/preprocessor.h"

void Batch::render()
{
    shader->Enable();
    shader->setup();
    for (unsigned int i = 0; i < drawables.size(); i++)
    {
        Info("Writing batch entry " << i);
        drawables.at(i)->draw(*shader);
    }
    shader->cleanup();
}
