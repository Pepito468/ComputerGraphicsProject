#include <node.hpp>
class GameNode : public Node {

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
