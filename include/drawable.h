#ifndef DRAWABLE
#define DRAWABLE

#include <iostream>
#include <vector>
class Shader;

class IDrawable {

public:
    virtual void draw(Shader &shader) = 0;
    void addChild(IDrawable* child)
    {
        children.push_back(child);
    }
    void drawChildren(Shader &shader)
    {
        draw(shader);
        std::cout << "Done drawing main.." << std::endl;
        if (!children.empty())
        {
            for (unsigned int i = 0; i < children.size(); i++)
            {
                children[i]->drawChildren(shader);
                std::cout << "Done drawing child number " << i << std::endl;
            }
        }
    }

    std::vector<IDrawable*> children;
};

#endif
