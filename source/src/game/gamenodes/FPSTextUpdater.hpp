#ifndef ENGINE_FPSTEXTUPDATER
#define ENGINE_FPSTEXTUPDATER

#include "Engine.hpp"
#include "Text2D.hpp"
#include "UpdateNode3D.hpp"

class FPSTextUpdater : public UpdateNode3D
{
    Text2D* fpsText;
    Text2D* timeText = nullptr;

public:

    FPSTextUpdater()
    {
        fpsText = new Text2D("", {-1, -1}, "SS", false, true, true, TAL_LEFT, TRH_LEFT, TRV_TOP);
        timeText = new Text2D("", {1, 1}, "SS", false, true, true, TAL_LEFT, TRH_RIGHT, TRV_BOTTOM);
        Node::adopt(fpsText);
        Node::adopt(timeText);
    }
    explicit FPSTextUpdater(Text2D* fpsText)
    {
        this->fpsText = fpsText;
        Node::adopt(fpsText);
    }

    void update() override
    {
        if (!fpsText)
        {
            warning("FPS Text set to NULL", true);
            return;
        }

        static float elapsedT = 1.0f;
        static float count = 0;
        elapsedT += Engine::getDeltaTime();
        count++;

        if (Engine::isPauseMenuOpen())
            return;

        static float totalTimeElaped = 0.0f;
        totalTimeElaped += Engine::getDeltaTime();

        if (elapsedT >= 1.0f) {
            fpsText->text = std::format("FPS: {}", count / elapsedT);
            elapsedT = 0.0f;
            count = 0;
            if (timeText)
                timeText->text = std::format("Time: {}", totalTimeElaped);
        }
    }
};
#endif
