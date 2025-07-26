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
};

void Testlayer::OnImGuiRender() {
};

void Testlayer::OnEvent(Event& event) {
};
