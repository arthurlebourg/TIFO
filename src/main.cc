#include <SDL2/SDL.h>
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <set>
// #include <tbb/tbb.h> // To disable multi-threading
#include <thread>
#include <vector>

#include "buffer_utils.hh"
#include "canny.hh"
#include "filters.hh"
#include "kernels.hh"
#include "octree.hh"

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    // tbb::task_scheduler_init t_init(1); // To disable multi-threading

    SDL_Window *window = SDL_CreateWindow("TIFO", SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED, screen_width,
                                          screen_height, SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    /*std::cout << "Renderer name: " << info.name << std::endl;
    std::cout << "Texture formats: " << std::endl;
    for (Uint32 i = 0; i < info.num_texture_formats; i++)
    {
        std::cout << SDL_GetPixelFormatName(info.texture_formats[i])
                  << std::endl;
    }*/

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             screen_width, screen_height);

    SDL_Event event;
    bool running = true;

    unsigned int frames = 0;
    Uint64 start = SDL_GetPerformanceCounter();

    FILE *pipein;
    if (argc == 1)
    {
        pipein = popen("ffmpeg -loglevel error -stream_loop -1 -i /dev/video0  "
                       "-f image2pipe "
                       "-vcodec rawvideo "
                       "-pix_fmt rgba -r 30 -s 1280x720 -",
                       "r");
    }
    else
    {
        auto command =
            std::string("ffmpeg -loglevel error -stream_loop -1 -i ");
        command.append(argv[1]);
        command.append(" -f image2pipe "
                       "-vcodec rawvideo "
                       "-pix_fmt rgba -r 25 "
                       "-s 1280x720 -");
        pipein = popen(command.c_str(), "r");
    }

    unsigned char *raw_buffer = (unsigned char *)calloc(
        screen_width * screen_height * 4, sizeof(unsigned char));

    std::vector<Matrix<float>> canny_buffers(
        3, Matrix<float>(screen_height, screen_width, 0));

    Matrix<Color> bil_filter_buffer(screen_height, screen_width, Color());

    Matrix<Color> pixels_matrix(screen_height, screen_width, Color());

    Quantizer q;
    std::vector<Color> palette;
    int palette_number = 40;

    int count;
    bool color_quantization = false;
    bool palette_init = false;
    bool generate_palette = false;
    bool dark_borders = false;
    bool border_dilation = true;
    bool edges_only = false;
    bool pixelate = false;

    Blur blur = Blur::GAUSS;
    float low_threshold_ratio = 0.030;
    float high_threshold_ratio = 0.150;

    auto square = square_kernel(2, 2);

    while (running)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // Read a frame from the input pipe into the buffer
        count = fread(raw_buffer, 1, screen_width * screen_height * 4, pipein);

        // If we didn't get a frame of video, we're probably at the end
        if (count != screen_width * screen_height * 4)
            break;

        while (SDL_PollEvent(&event))
        {
            if ((SDL_QUIT == event.type)
                || (SDL_KEYDOWN == event.type
                    && SDL_SCANCODE_ESCAPE == event.key.keysym.scancode))
            {
                running = false;
                break;
            }

            if (SDL_KEYDOWN == event.type)
            {
                auto state = SDL_GetKeyboardState(NULL);
                if (state[SDL_SCANCODE_C])
                {
                    color_quantization = palette_init && !color_quantization;
                }
                if (state[SDL_SCANCODE_P])
                {
                    generate_palette = !generate_palette;
                }
                if (state[SDL_SCANCODE_B])
                {
                    edges_only = false;
                    dark_borders = !dark_borders;
                }
                if (state[SDL_SCANCODE_E])
                {
                    dark_borders = false;
                    edges_only = !edges_only;
                }
                if (state[SDL_SCANCODE_D])
                {
                    border_dilation = dark_borders && !border_dilation;
                }
                if (state[SDL_SCANCODE_N])
                {
                    pixelate = !pixelate;
                }
                if (dark_borders || edges_only)
                {
                    if (state[SDL_SCANCODE_RIGHT])
                    {
                        ++blur;
                        std::cout << "Selected canny blur: " << blur
                                  << std::endl;
                    }
                    else if (state[SDL_SCANCODE_LEFT])
                    {
                        --blur;
                        std::cout << "Selected canny blur: " << blur
                                  << std::endl;
                    }
                    else if (state[SDL_SCANCODE_UP])
                    {
                        if (state[SDL_SCANCODE_L])
                            low_threshold_ratio += 0.01;
                        else if (state[SDL_SCANCODE_H])
                            high_threshold_ratio += 0.01;

                        std::cout << "Set threshold ratios to: "
                                  << low_threshold_ratio << ", "
                                  << high_threshold_ratio << std::endl;
                    }
                    else if (state[SDL_SCANCODE_DOWN])
                    {
                        if (state[SDL_SCANCODE_L])
                            low_threshold_ratio -= 0.01;
                        else if (state[SDL_SCANCODE_H])
                            high_threshold_ratio -= 0.01;

                        std::cout << "Set threshold ratios to: "
                                  << low_threshold_ratio << ", "
                                  << high_threshold_ratio << std::endl;
                    }
                }
            }
        }

        if (generate_palette)
        {
            q = Quantizer();

            std::cout << "generating new color palette" << std::endl;

            for (size_t i = 0; i < screen_height * screen_width; i++)
            {
                auto color = get_pixel(raw_buffer, i * 4);
                q.add_color(color);
            }

            palette = q.make_palette(palette_number);
            std::cout << "color palette: " << palette.size() << std::endl;

            palette_init = true;
            generate_palette = false;
        }

        // Preprocess

        if (color_quantization)
        {
            apply_palette(raw_buffer, q, palette);
            boost_luminance(raw_buffer);
            //  apply_palette_debug(raw_buffer, q, palette, screen_width /
            //  2);
        }

        if (dark_borders)
        {
            to_grayscale(raw_buffer, canny_buffers[0]);
            edge_detection(canny_buffers, blur, low_threshold_ratio,
                           high_threshold_ratio);

            if (border_dilation)
            {
                // Dilation to thicken edges, WIP
                canny_buffers[0].morph(square, true, canny_buffers[1]);
                canny_buffers[0].swap(canny_buffers[1]);
            }

            set_dark_borders(raw_buffer, canny_buffers[0]);
        }

        if (edges_only)
        {
            to_grayscale(raw_buffer, canny_buffers[0]);
            edge_detection(canny_buffers, blur, low_threshold_ratio,
                           high_threshold_ratio);
            // remap_to_rgb(canny_edge_buffers[0]);
            fill_buffer(raw_buffer, canny_buffers[0]);
        }

        if (pixelate)
        {
            pixelate_buffer(raw_buffer, 10);
        }

        // SDL again
        SDL_UpdateTexture(texture, NULL, raw_buffer, screen_width * 4);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        frames++;
        const Uint64 end = SDL_GetPerformanceCounter();
        const static Uint64 freq = SDL_GetPerformanceFrequency();
        const double seconds = (end - start) / static_cast<double>(freq);
        if (seconds > 2.0)
        {
            std::cout << frames << " frames in " << std::setprecision(1)
                      << std::fixed << seconds
                      << " seconds = " << std::setprecision(1) << std::fixed
                      << frames / seconds << " FPS (" << std::setprecision(3)
                      << std::fixed << (seconds * 1000.0) / frames
                      << " ms/frame)" << std::endl;
            start = end;
            frames = 0;
        }
    }

    // Flush and close input and output pipes
    fflush(pipein);
    pclose(pipein);
    free(raw_buffer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
