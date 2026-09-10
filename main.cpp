// src/main.cpp (example usage)
#include "Domain/IRenderable.hpp"
#include <memory>
#include <iostream>

using namespace InputProcessor::Domain;

struct ConcreteRenderable : IRenderable {
    void Render(Transform) override {}
};

struct LoggingDecorator : IRenderableDecorator {
    LoggingDecorator(std::shared_ptr<IRenderable> r) : IRenderableDecorator(r) {}
    void Render(Transform t) override {
        // delegate
        mRenderable->Render(t);
    }
    // helper for debug demo
    std::shared_ptr<IRenderable> GetWrapped() const { return mRenderable; }
};

int main() {
    auto r = std::make_shared<ConcreteRenderable>();
    auto d = std::make_shared<LoggingDecorator>(r);

    // Put a breakpoint on the next line:
    std::cout << "Created objects\n";

    // Optional: print raw addresses too
    std::cout << "r raw: " << static_cast<void*>(r.get()) << "\n";

    std::cout << "d raw: " << static_cast<void*>(d.get()) << "\n";

    std::cout << "wrapped raw: " << static_cast<void*>(d->GetWrapped().get()) << "\n";

    return 0;
}