#include <Node3D.hpp>

class CustomGameNode : public Node3D {

    public:
        void onEnter() override {
            // Sample enter
            UUID = 27;
        }

        void onExit() override {
            // Sample Exit
            UUID = 31;
        }
};
