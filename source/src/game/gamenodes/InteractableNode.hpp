#ifndef ENGINE_INTERACTABLE
#define ENGINE_INTERACTABLE

#include "UpdateNode3D.hpp"

/// Interface class for nodes the player can interact with
class InteractableNode : public UpdateNode3D
{
public:
    virtual void select() {}
    virtual void deselect() {}
    virtual void interact() = 0;
};
#endif