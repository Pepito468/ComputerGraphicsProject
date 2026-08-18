#include <cmath>
#include <cstdint>
#include <vector>

class ProceduralTextures {
public:
    struct TextureData {
        int width;
        int height;
        std::vector<uint8_t> pixels; // RGBA8
    };

    static TextureData generateUITexture(int width = 256, int height = 256) {
        TextureData texture;
        texture.width = width;
        texture.height = height;
        texture.pixels.resize(width * height * 4);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Simple procedural variation
                float noise = 0.85f + 0.15f * std::sin(x * 0.11f) * std::sin(y * 0.07f);

                uint8_t r = static_cast<uint8_t>(150 * noise);
                uint8_t g = static_cast<uint8_t>(55 * noise);
                uint8_t b = static_cast<uint8_t>(38 * noise);

                setPixel(texture, x, y, r, g, b);
            }
        }

        return texture;
    }

private:
    static void setPixel(TextureData &texture, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        int index = 4 * (y * texture.width + x);

        texture.pixels[index + 0] = r;
        texture.pixels[index + 1] = g;
        texture.pixels[index + 2] = b;
        texture.pixels[index + 3] = a;
    }
};
