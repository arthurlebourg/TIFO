#include <SDL2/SDL.h>
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <set>
#include <thread>
#include <vector>

#include "canny.hh"
#include "filters.hh"
#include "gauss.hh"
#include "octree.hh"

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

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

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             screen_width, screen_height);

    SDL_Event event;
    bool running = true;
    bool useLocktexture = false;

    unsigned int frames = 0;
    Uint64 start = SDL_GetPerformanceCounter();

    unsigned char *pixels = (unsigned char *)calloc(
        screen_width * screen_height * 4, sizeof(unsigned char));

    FILE *pipein;
    if (argc == 1)
    {
        pipein = popen("ffmpeg -loglevel error -stream_loop -1 -i /dev/video0  "
                       "-f image2pipe "
                       "-vcodec rawvideo "
                       "-pix_fmt rgba -framerate 25 -s 1280x720 -",
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

    std::vector<Matrix<float>> canny_buffers(
        3, Matrix<float>(screen_height, screen_width, 0));

    Quantizer q;
    std::vector<Color> palette;
    int palette_number = 256;

    int count;
    bool init = true;
    while (running)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        while (SDL_PollEvent(&event))
        {
            if ((SDL_QUIT == event.type)
                || (SDL_KEYDOWN == event.type
                    && SDL_SCANCODE_ESCAPE == event.key.keysym.scancode))
            {
                running = false;
                break;
            }
            if (SDL_KEYDOWN == event.type
                && SDL_SCANCODE_L == event.key.keysym.scancode)
            {
                useLocktexture = !useLocktexture;
                std::cout << "Using "
                          << (useLocktexture ? "SDL_LockTexture() + memcpy()"
                                             : "SDL_UpdateTexture()")
                          << std::endl;
            }
            if (SDL_KEYDOWN == event.type
                && SDL_SCANCODE_P == event.key.keysym.scancode)
            {
                q = Quantizer();
                palette.clear();
                std::cout << "generating new color palette" << std::endl;
                std::vector<Color> colors = unique_colors(pixels);
                std::cout << "colors: " << colors.size() << " | " << std::endl;

                for (auto i : colors)
                {
                    q.add_color(i);
                }

                palette = q.make_palette(palette_number);
                std::cout << "color palette: " << palette.size() << " | "
                          << std::endl;
                for (auto i : palette)
                {
                    std::cout << i << std::endl;
                }
            }
        }

        // Not SDL

        // Read a frame from the input pipe into the buffer
        count = fread(pixels, 1, screen_width * screen_height * 4, pipein);

        // If we didn't get a frame of video, we're probably at the end
        if (count != screen_width * screen_height * 4)
            break;

        if (init)
        {
            init = false;
            std::cout << "generating new color palette" << std::endl;
            std::vector<Color> colors = unique_colors(pixels);
            std::cout << "colors: " << colors.size() << " | " << std::endl;

            for (auto i : colors)
            {
                q.add_color(i);
            }

            palette = q.make_palette(palette_number);
            std::cout << "color palette: " << palette.size() << " | "
                      << std::endl;
            // for (auto i : palette)
            // {
            //     std::cout << i << std::endl;
            // }
        }

        // preprocess

        // Grayscale
        canny_buffers[0].set_values(to_grayscale(pixels));

        edge_detection(canny_buffers);

        auto &output = canny_buffers[0];

        remap_to_rgb(output);

        fill_buffer_palette(q, palette, pixels);
        // fill_buffer(pixels);
        fill_buffer_dark_borders(pixels, output);

        // fill_buffer(pixels, output);

        // SDL again

        if (useLocktexture)
        {
            unsigned char *lockedPixels = nullptr;
            int pitch = 0;
            SDL_LockTexture(texture, NULL,
                            reinterpret_cast<void **>(&lockedPixels), &pitch);
            std::memcpy(lockedPixels, pixels, screen_width * screen_height * 3);
            SDL_UnlockTexture(texture);
        }
        else
        {
            SDL_UpdateTexture(texture, NULL, pixels, screen_width * 4);
        }

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
    free(pixels);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
