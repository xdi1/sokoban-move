#pragma once
#include "engine.h"
#include "game_context.h"
#include "input_system.h"

using std::chrono::steady_clock;

namespace animation {
    inline steady_clock::time_point last_frame_time;
    inline steady_clock::time_point end_time;


    inline f64 get_t(const steady_clock::time_point now) {
        if (now < last_frame_time) return 0.0;
        if (now >= end_time) return 1.0;

        const steady_clock::duration t = now - last_frame_time;
        if (t < end_time - last_frame_time) {
            return std::chrono::duration<f64>(t).count() / std::chrono::duration<f64>(end_time - last_frame_time).count();
        }

        return 1.0;
    }
};

struct sokoban {
    std::queue<i32> directions; /* 0 left, 1 up, 2 right, 3 down */
    f64 total_distance;

    i32 x {};
    i32 y {};

    f32 draw_x {};
    f32 draw_y {};

    i32 animation_time { 150 }; /*in ms*/
    i32 num_step { 1 };

    f64 brightness { 1.0 };

    static void init() {
        animation::last_frame_time = steady_clock::now();
    }

    static f64 walk_brightness(const f64 step_remaining, const f64 step_size = 100.0) {
        const f64 min_brightness = 0.45;
        const f64 p = std::clamp(1.0 - step_remaining / step_size, 0.0, 1.0);
        const f64 wave = std::abs(2.0 * p - 1.0);
        return min_brightness + (1.0 - min_brightness) * wave;
    };

    void move(const f64 distance, const i32 direction) {
        if (direction == 0) {
            draw_x -= (f32)distance;
        }
        else if (direction == 1) {
            draw_y -= (f32)distance;
        }
        else if (direction == 2) {
            draw_x += (f32)distance;
        }
        else if (direction == 3) {
            draw_y += (f32)distance;
        }
    }

    void tick() {
        // Input handling
        if (input::key_down_no_mod('1')) {
            if (animation_time >= 50) {
                animation_time -= 50;
            }
        }
        if (input::key_down_no_mod('2')) {
            if (animation_time <= 950) {
                animation_time += 50;
            }
        }

        if (input::key_down_no_mod('3')) {
            if (num_step >= 2) {
                num_step -= 1;
            }
        }
        if (input::key_down_no_mod('4')) {
            if (num_step <= 9) {
                num_step += 1;
            }
        }

        if (input::key_down_no_mod(VK_F1)) {
            animation::end_time = steady_clock::now() + std::chrono::milliseconds(animation_time);

            for (const size_t i : range(num_step)) {
                directions.push(0);
                total_distance += 100;
                x -= 100;
            }

            for (const size_t i : range(num_step)) {
                directions.push(1);
                total_distance += 100;
                y -= 100;
            }

            for (const size_t i : range(num_step)) {
                directions.push(2);
                total_distance += 100;
                x += 100;
            }

            for (const size_t i : range(num_step)) {
                directions.push(3);
                total_distance += 100;
                y += 100;
            }
        }

        if (input::key_down_no_mod(VK_LEFT)) {
            animation::end_time = steady_clock::now() + std::chrono::milliseconds(animation_time);

            for (const size_t i : range(num_step)) {
                directions.push(0);
                total_distance += 100;
                x -= 100;
            }
        }
        if (input::key_down_no_mod(VK_UP)) {
            animation::end_time = steady_clock::now() + std::chrono::milliseconds(animation_time);

            for (const size_t i : range(num_step)) {
                directions.push(1);
                total_distance += 100;
                y -= 100;
            }
        }
        if (input::key_down_no_mod(VK_RIGHT)) {
            animation::end_time = steady_clock::now() + std::chrono::milliseconds(animation_time);

            for (const size_t i : range(num_step)) {
                directions.push(2);
                total_distance += 100;
                x += 100;
            }
        }
        if (input::key_down_no_mod(VK_DOWN)) {
            animation::end_time = steady_clock::now() + std::chrono::milliseconds(animation_time);

            for (const size_t i : range(num_step)) {
                directions.push(3);
                total_distance += 100;
                y += 100;
            }
        }

        char buffer[256];
        sprintf_s<256>(buffer, "Animation time: %d  (1 / 2 to decrease / increase)", animation_time);
        game_context::push_text_draw(buffer, { 100, 100 }, DirectX::Colors::White);

        sprintf_s<256>(buffer, "Steps per input: %d  (simulate multiple input in one frame, 3 / 4 to decrease / increase)", num_step);
        game_context::push_text_draw(buffer, { 100, 150 }, DirectX::Colors::White);

        sprintf_s<256>(buffer, "F1: Do a circular motion  (scaled by steps per input)", num_step);
        game_context::push_text_draw(buffer, { 100, 200 }, DirectX::Colors::White);

        sprintf_s<256>(buffer, "x: %d\ny: %d\ndraw_x: %f\ndraw_y: %f\nnum_inputs_queued: %d", x, y, draw_x, draw_y, directions.size());
        game_context::push_text_draw(buffer, { (f32)window_context::width - 500, 200 }, DirectX::Colors::White);

        // Animate
        const f64 t = animation::get_t(steady_clock::now());
        animation::last_frame_time = steady_clock::now();

        if (!directions.empty()) {
            f64 delta_distance = std::lerp(0, total_distance, t);

            for (;;) {
                const i32 current_step_direction = directions.front();
                const f64 current_step_distance = total_distance - 100 * (directions.size() - 1);
                brightness = walk_brightness(current_step_distance, 100.0);

                if (delta_distance >= current_step_distance) {
                    delta_distance -= current_step_distance;
                    move(current_step_distance, current_step_direction);

                    total_distance -= current_step_distance;
                    directions.pop();
                    if (directions.empty()) {
                        /* manually set value to avoid floating point error */
                        total_distance = 0;
                        draw_x = x;
                        draw_y = y;
                        brightness = 1.0;
                        break;
                    };
                }
                else {
                    total_distance -= delta_distance;
                    move(delta_distance, current_step_direction);
                    break;
                }
            }
        }
        
        // Draw
        const size_t sprite_start = game_context::_sprite_instance._size;
        sprite_instance *instance = game_context::_sprite_instance.push_back({});
        instance->_position = { draw_x, draw_y };
        instance->_size = { 100, 100 };
        instance->_depth = 0.999f;
        instance->_color = { 0.3f * (f32)brightness, 0.6f * (f32)brightness, 0.3f * (f32)brightness, 1.0f };

        game_context::_sprite_instanced_draw_opaque.push_back({ {}, sprite_start, 1, false });
    }
};
