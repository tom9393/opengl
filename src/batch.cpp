#include "../include/batch.h"

void Batch::render()
{
    shader->Enable();
    shader->setup();
    shader->draw(*drawables);
    shader->cleanup();
}
