#include "../include/batch.h"

void Batch::render()
{
    shader->setup();
    shader->draw(*drawables);
    shader->cleanup();
}
