#include "../include/batch.h"

void Batch::render()
{
    std::cout << "Drawing batch" << std::endl;
    std::cout << shader->GetProgram() << std::endl;
    shader->Enable();
    shader->setup();
    for (unsigned int i = 0; i < drawables->size(); i++)
    {
        drawables->at(i)->drawChildren(*shader);
    }
    shader->cleanup();
}
