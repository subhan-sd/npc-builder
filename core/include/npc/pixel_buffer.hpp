#pragma once
#include "types.hpp"
#include <vector>
#include <algorithm>

namespace npc {

struct PixelBuffer {
    int width{0};
    int height{0};
    std::vector<ColorRGBA> pixels;

    PixelBuffer() = default;
    PixelBuffer(int w, int h, ColorRGBA fill = {0, 0, 0, 0})
        : width(w), height(h), pixels(w * h, fill) {}

    void resize(int w, int h, ColorRGBA fill = {0, 0, 0, 0}) {
        width = w;
        height = h;
        pixels.assign(w * h, fill);
    }

    void set_pixel(int x, int y, ColorRGBA color) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        if (color.a == 0) return;
        // ponytail: naive alpha composite (src over), full blending if opacity layering needed
        if (color.a == 255) {
            pixels[y * width + x] = color;
        } else {
            auto& dst = pixels[y * width + x];
            float a = color.a / 255.0f;
            dst.r = static_cast<uint8_t>(color.r * a + dst.r * (1.0f - a));
            dst.g = static_cast<uint8_t>(color.g * a + dst.g * (1.0f - a));
            dst.b = static_cast<uint8_t>(color.b * a + dst.b * (1.0f - a));
            dst.a = std::max(dst.a, color.a);
        }
    }

    ColorRGBA get_pixel(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return {0, 0, 0, 0};
        return pixels[y * width + x];
    }

    void fill_rect(int x, int y, int w, int h, ColorRGBA color) {
        for (int dy = 0; dy < h; ++dy) {
            for (int dx = 0; dx < w; ++dx) {
                set_pixel(x + dx, y + dy, color);
            }
        }
    }

    void draw_hline(int x, int y, int len, ColorRGBA color) {
        for (int dx = 0; dx < len; ++dx) {
            set_pixel(x + dx, y, color);
        }
    }

    void draw_span(int y, int x_start, int x_end, ColorRGBA fill_color, ColorRGBA left_edge, ColorRGBA right_edge) {
        if (x_end < x_start) return;
        if (x_start == x_end) {
            set_pixel(x_start, y, left_edge);
            return;
        }
        set_pixel(x_start, y, left_edge);
        for (int x = x_start + 1; x < x_end; ++x) {
            set_pixel(x, y, fill_color);
        }
        set_pixel(x_end, y, right_edge);
    }
};

} // namespace npc
