#include <SDL2/SDL.h>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include "color.hh"

const size_t screen_width = 640;
const size_t screen_height = 480;

const int max_threads = std::thread::hardware_concurrency();

Color get_pixel(unsigned char *pixels, size_t offset)
{
    return Color(pixels[offset + 0], pixels[offset + 1], pixels[offset + 2],
                 pixels[offset + 3]);
}

void set_pixel(unsigned char *pixels, size_t offset, Color col)
{
    pixels[offset + 0] = col.blue(); // b
    pixels[offset + 1] = col.green(); // g
    pixels[offset + 2] = col.red(); // r
    pixels[offset + 3] = col.a(); // a
}

size_t get_offset(size_t x, size_t y)
{
    return (y * 4) * screen_width + (x * 4);
}

void fill_buffer(size_t miny, size_t maxy, unsigned char *pixels)
{
    for (size_t y = miny + 1; y < maxy; y++)
    {
        for (size_t x = 1; x < screen_width; x++)
        {
            size_t offset = get_offset(x, y);
            Color old_pixel = get_pixel(pixels, offset);
            set_pixel(pixels, offset, old_pixel);
        }
    }
}

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *window = SDL_CreateWindow("TIFO", SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED, screen_width,
                                          screen_height, SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    std::cout << "Renderer name: " << info.name << std::endl;
    std::cout << "Texture formats: " << std::endl;
    for (Uint32 i = 0; i < info.num_texture_formats; i++)
    {
        std::cout << SDL_GetPixelFormatName(info.texture_formats[i])
                  << std::endl;
    }

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

    FILE *pipein = popen("ffmpeg -i /dev/video0 -f image2pipe -vcodec rawvideo "
                         "-pix_fmt rgba  -framerate 25 -video_size 640x480 -",
                         "r");

    int count;
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
        }

        // Not SDL

        // Read a frame from the input pipe into the buffer
        count = fread(pixels, 1, screen_width * screen_height * 4, pipein);

        // If we didn't get a frame of video, we're probably at the end
        if (count != screen_width * screen_height * 4)
            break;

        // Process frame
        double y_num = screen_height / max_threads;
        std::vector<std::thread> threads;
        for (int i = 0; i < max_threads - 1; i++)
        {
            threads.push_back(
                std::thread(fill_buffer, i * y_num, (i + 1) * y_num, pixels));
        }
        fill_buffer((max_threads - 1) * y_num, max_threads * y_num, pixels);
        for (int i = 0; i < max_threads - 1; i++)
        {
            threads[i].join();
        }

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
