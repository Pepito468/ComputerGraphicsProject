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

    /**
     * Generates a texture of the given dimensions of a uniform rectangle of the given color
     * */
    static TextureData generateTexture(int width = 256, int height = 256, int r_in = 150, int g_in = 55, int b_in = 38, int a_in = 255) {
        TextureData texture;
        texture.width = width;
        texture.height = height;
        texture.pixels.resize(width * height * 4);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                setPixel(texture, x, y, static_cast<uint8_t>(r_in), static_cast<uint8_t>(g_in), static_cast<uint8_t>(b_in), static_cast<uint8_t>(a_in));
            }
        }

        return texture;
    }

    /**
     * Generates a texture of the given dimensions of a rectangle of a given color with some noise
     * */
    static TextureData generateTextureWithNoise(int width = 256, int height = 256, int r_in = 150, int g_in = 55, int b_in = 38, int a_in = 255, float constNoise = 0.85f, float varNoise = 0.15f, float xNoise = 0.11f, float yNoise = 0.07f) {
        TextureData texture;
        texture.width = width;
        texture.height = height;
        texture.pixels.resize(width * height * 4);
        float noise;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                noise = constNoise + varNoise * std::sin(x * xNoise) * std::sin(y * yNoise);

                uint8_t r = static_cast<uint8_t>(r_in * noise);
                uint8_t g = static_cast<uint8_t>(g_in * noise);
                uint8_t b = static_cast<uint8_t>(b_in * noise);

                setPixel(texture, x, y, r, g, b, static_cast<uint8_t>(a_in));
            }
        }

        return texture;
    }

    static TextureData generateMenuBackgroundTint(int width, int height) {
        return generateTexture(width, height, 255, 255, 255, 100);
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
