#import "../include/testlayer.h"

Testlayer::~Testlayer()
{
};

void Testlayer::OnAttach() {
    std::cout << "Attach" << std::endl;
};

void Testlayer::OnDetach() {
    std::cout << "Detatch" << std::endl;
};

void Testlayer::OnUpdate() {
    std::cout << "Update" << std::endl;
};

void Testlayer::OnImGuiRender() {
    std::cout << "On render" << std::endl;
};

void Testlayer::OnEvent(Event& event) {
    std::cout << event.getName() << std::endl;
    std::cout << "In the testlayer" << std::endl;
};
