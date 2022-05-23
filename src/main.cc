#include <SDL2/SDL.h>
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <set>
// #include <tbb/tbb.h> // To disable multi-threading
#include <SDL2/SDL_ttf.h>
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

    // Text
    TTF_Init();
    // this opens a font style and sets a size
    TTF_Font *Sans = TTF_OpenFont("FreeSans.ttf", 24);
    if (Sans == NULL)
    {
        fprintf(stderr, "error: font not found\n");
        exit(EXIT_FAILURE);
    }

    // this is the color in rgb format,
    // maxing out all would give you the color white,
    // and it will be your text's color
    SDL_Color White = { 255, 255, 255, 255 };

    // as TTF_RenderText_Solid could only be used on
    // SDL_Surface then you have to create the surface first
    SDL_Surface *surfaceMessage =
        TTF_RenderText_Solid(Sans, "Keyboard Shortcuts:", White);

    // now you can convert it into a texture
    SDL_Texture *Message =
        SDL_CreateTextureFromSurface(renderer, surfaceMessage);

    SDL_Rect Message_rect; // create a rect
    Message_rect.x = 0; // controls the rect's x coordinate
    Message_rect.y = 0; // controls the rect's y coordinte
    Message_rect.w = 500; // controls the width of the rect
    Message_rect.h = 100; // controls the height of the rect

    // (0,0) is on the top left of the window/screen,
    // think a rect as the text's box,
    // that way it would be very simple to understand

    // Now since it's a texture, you have to put RenderCopy
    // in your game loop area, the area where the whole code executes

    // you put the renderer's name first, the Message,
    // the crop size (you can ignore this if you don't want
    // to dabble with cropping), and the rect which is the size
    // and coordinate of your texture
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
    SDL_RenderPresent(renderer);

    std::cout << "wait" << std::endl;
    SDL_Delay(3000);
    std::cout << "go" << std::endl;

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

    Matrix<RGB> bil_filter_buffer(screen_height, screen_width, RGB());

    Matrix<RGB> pixels_matrix(screen_height, screen_width, RGB());

    Quantizer q;
    std::vector<RGB> palette;
    tbb::concurrent_vector<size_t> palette_lightness_cumul_histo;
    int palette_number = 40;

    int count;
    bool color_quantization = false;
    bool palette_init = false;
    bool generate_palette = false;
    bool dark_borders = false;
    bool border_dilation = true;
    bool edges_only = false;
    bool pixelate = false;

    bool saturation_boost = true;
    bool contrast_cor = false;

    bool render_shortcuts = false;

    Blur blur = Blur::GAUSS;
    float low_threshold_ratio = 0.030;
    float high_threshold_ratio = 0.150;
    float saturation_value = 1.5;

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
                if (state[SDL_SCANCODE_SPACE])
                {
                    render_shortcuts = !render_shortcuts;
                    std::cout << "Show shortcuts: "
                              << (render_shortcuts ? "enabled" : "disabled")
                              << std::endl;
                }
                if (state[SDL_SCANCODE_C])
                {
                    color_quantization = palette_init && !color_quantization;
                    std::cout << "Color quantization: "
                              << (color_quantization ? "enabled" : "disabled")
                              << std::endl;
                }
                if (state[SDL_SCANCODE_P])
                {
                    generate_palette = !generate_palette;
                }
                if (state[SDL_SCANCODE_B])
                {
                    edges_only = false;
                    dark_borders = !dark_borders;
                    std::cout << "Border darkening: "
                              << (dark_borders ? "enabled" : "disabled")
                              << std::endl;
                }
                if (state[SDL_SCANCODE_E])
                {
                    dark_borders = false;
                    edges_only = !edges_only;
                }
                if (state[SDL_SCANCODE_N])
                {
                    pixelate = !pixelate;
                }
                if (state[SDL_SCANCODE_X])
                {
                    // contrast_cor = color_quantization && !contrast_cor;
                    contrast_cor = !contrast_cor;
                    std::cout << "Contrast correction: "
                              << (contrast_cor ? "enabled" : "disabled")
                              << std::endl;
                }
                if (state[SDL_SCANCODE_S])
                {
                    saturation_boost = color_quantization && !saturation_boost;
                    std::cout << "Saturation boost: "
                              << (saturation_boost ? "enabled" : "disabled")
                              << std::endl;
                }
                if (dark_borders || edges_only)
                {
                    if (state[SDL_SCANCODE_D])
                    {
                        border_dilation = !border_dilation;
                        std::cout << "Border dilation: "
                                  << (border_dilation ? "enabled" : "disabled")
                                  << std::endl;
                    }

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
                if (saturation_boost)
                {
                    if (state[SDL_SCANCODE_UP] && !state[SDL_SCANCODE_H]
                        && !state[SDL_SCANCODE_L])
                    {
                        saturation_value += 0.1;
                        std::cout
                            << "Set saturation boost to: " << saturation_value
                            << std::endl;
                    }
                    else if (state[SDL_SCANCODE_DOWN] && !state[SDL_SCANCODE_H]
                             && !state[SDL_SCANCODE_L])
                    {
                        saturation_value -= 0.1;
                        std::cout
                            << "Set saturation boost to: " << saturation_value
                            << std::endl;
                    }
                }
            }
        }
        if (!render_shortcuts)
        {
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

                palette_lightness_cumul_histo =
                    q.get_lightness_cumulative_histogram();

                palette_init = true;
                generate_palette = false;
            }

            // Preprocess

            // Compute edges BEFORE color pre-processing
            if (dark_borders)
            {
                to_grayscale(raw_buffer, canny_buffers[0]);
                edge_detection(canny_buffers, blur, low_threshold_ratio,
                               high_threshold_ratio);

                if (border_dilation)
                {
                    thicken_edges(canny_buffers[0], canny_buffers[2],
                                  canny_buffers[1]);
                    canny_buffers[1].swap(canny_buffers[0]);
                }
            }
            else if (edges_only)
            {
                to_grayscale(raw_buffer, canny_buffers[0]);
                edge_detection(canny_buffers, blur, low_threshold_ratio,
                               high_threshold_ratio);
                // remap_to_rgb(canny_edge_buffers[0]);
                if (border_dilation)
                {
                    thicken_edges(canny_buffers[0], canny_buffers[2],
                                  canny_buffers[1]);
                    canny_buffers[1].swap(canny_buffers[0]);
                }
            }

            if (color_quantization)
            {
                // if (contrast_cor) // From raw buffer
                // {
                //     auto c_histo =
                //     compute_lightness_cumul_histogram(raw_buffer);
                //     contrast_correction(raw_buffer, c_histo);
                // }

                apply_palette(raw_buffer, q, palette);

                if (contrast_cor) // From palette
                {
                    contrast_correction(raw_buffer,
                                        palette_lightness_cumul_histo);
                }

                if (saturation_boost)
                {
                    saturation_modification(raw_buffer, saturation_value);
                }

                //  apply_palette_debug(raw_buffer, q, palette, screen_width /
                //  2);
            }

            // Apply edges AFTER color pre-processing
            if (dark_borders)
            {
                set_dark_borders(raw_buffer, canny_buffers[0]);
            }
            else if (edges_only)
            {
                fill_buffer(raw_buffer, canny_buffers[0]);
            }

            if (pixelate)
            {
                pixelate_buffer(raw_buffer, 10);
            }
        }

        // SDL again
        SDL_UpdateTexture(texture, NULL, raw_buffer, screen_width * 4);
        if (render_shortcuts)
            SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
        else
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
