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

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             screen_width, screen_height);

    SDL_Event event;
    bool running = true;
    bool useLocktexture = false;

    unsigned int frames = 0;
    Uint64 start = SDL_GetPerformanceCounter();

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

    unsigned char *raw_buffer = (unsigned char *)calloc(
        screen_width * screen_height * 4, sizeof(unsigned char));

    std::vector<Matrix<float>> canny_buffers(
        3, Matrix<float>(screen_height, screen_width, 0));

    Quantizer q;
    std::vector<Color> palette;
    int palette_number = 100;

    int count;
    bool generate_palette = false;
    bool dark_borders = false;

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
                generate_palette = !generate_palette;
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
                    std::cout << "color palette: " << palette.size()
                              << std::endl;
                }
            }
            if (SDL_KEYDOWN == event.type
                && SDL_SCANCODE_B == event.key.keysym.scancode)
            {
                dark_borders = !dark_borders;
            }
        }

        // preprocess

        // Grayscale
        to_grayscale(raw_buffer, canny_buffers[0]);

        edge_detection(canny_buffers);

        auto &border_mask = canny_buffers[0];

        // Apply color quant
        if (generate_palette)
            apply_palette(raw_buffer, q, palette);
        // if (generate_palette)
        //     apply_palette_debug(raw_buffer, q, palette, screen_width / 2);
        if (dark_borders)
            set_dark_borders(raw_buffer, border_mask);

        // fill_buffer(pixels, output);

        // SDL again

        if (useLocktexture)
        {
            unsigned char *lockedPixels = nullptr;
            int pitch = 0;
            SDL_LockTexture(texture, NULL,
                            reinterpret_cast<void **>(&lockedPixels), &pitch);
            std::memcpy(lockedPixels, raw_buffer,
                        screen_width * screen_height * 3);
            SDL_UnlockTexture(texture);
        }
        else
        {
            SDL_UpdateTexture(texture, NULL, raw_buffer, screen_width * 4);
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
    free(raw_buffer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
