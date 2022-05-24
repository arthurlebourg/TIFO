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

void get_text_and_rect(SDL_Renderer *renderer, int x, int y, const char *text,
                       TTF_Font *font, SDL_Texture **texture, SDL_Rect *rect)
{
    int text_width;
    int text_height;
    SDL_Surface *surface;
    SDL_Color textColor = { 255, 255, 255, 0 };

    surface = TTF_RenderText_Solid(font, text, textColor);
    *texture = SDL_CreateTextureFromSurface(renderer, surface);
    text_width = surface->w;
    text_height = surface->h;
    SDL_FreeSurface(surface);
    rect->x = x;
    rect->y = y;
    rect->w = text_width;
    rect->h = text_height;
}

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
    SDL_Rect rect1, rect2, rect3, rect4, rect5, rect6, rect7, rect8, rect9,
        rect10;
    SDL_Texture *texture1, *texture2, *texture3, *texture4, *texture5,
        *texture6, *texture7, *texture8, *texture9, *texture10;
    // this opens a font style and sets a size
    TTF_Font *Sans = TTF_OpenFont("FreeSans.ttf", 24);
    if (Sans == NULL)
    {
        fprintf(stderr, "error: font not found\n");
        exit(EXIT_FAILURE);
    }

    get_text_and_rect(renderer, 0, 0, "Different keyboard shortcuts:", Sans,
                      &texture1, &rect1);
    get_text_and_rect(renderer, 0, rect1.y + rect1.h * 2,
                      "E : Display contours", Sans, &texture2, &rect2);
    get_text_and_rect(renderer, 0, rect2.y + rect2.h,
                      "B : Apply border darkening", Sans, &texture3, &rect3);
    get_text_and_rect(renderer, 0, rect3.y + rect3.h,
                      "D : Apply border dilation/thickening", Sans, &texture4,
                      &rect4);
    get_text_and_rect(renderer, 0, rect4.y + rect4.h,
                      "RIGHT and LEFT arrows to select blur function", Sans,
                      &texture5, &rect5);
    get_text_and_rect(renderer, 0, rect5.y + rect5.h,
                      "L / H + UP / DOWN to update low/high thresholds", Sans,
                      &texture6, &rect6);

    get_text_and_rect(renderer, 0, rect6.y + rect6.h * 2,
                      "P : compute color palette", Sans, &texture7, &rect7);
    get_text_and_rect(renderer, 0, rect7.y + rect7.h, "apply color palette",
                      Sans, &texture8, &rect8);
    get_text_and_rect(renderer, 0, rect8.y + rect8.h,
                      "UP / DOWN arrows to update the saturation value", Sans,
                      &texture9, &rect9);
    get_text_and_rect(renderer, 0, rect9.y + rect9.h * 2,
                      "N to activate pixel filter", Sans, &texture10, &rect10);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture1, NULL, &rect1);
    SDL_RenderCopy(renderer, texture2, NULL, &rect2);
    SDL_RenderCopy(renderer, texture3, NULL, &rect3);
    SDL_RenderCopy(renderer, texture4, NULL, &rect4);
    SDL_RenderCopy(renderer, texture5, NULL, &rect5);
    SDL_RenderCopy(renderer, texture6, NULL, &rect6);
    SDL_RenderCopy(renderer, texture7, NULL, &rect7);
    SDL_RenderCopy(renderer, texture8, NULL, &rect8);
    SDL_RenderCopy(renderer, texture9, NULL, &rect9);
    SDL_RenderCopy(renderer, texture10, NULL, &rect10);
    SDL_RenderPresent(renderer);

    // SDL_Delay(3000);

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
    unsigned char *saved_frame_buffer = (unsigned char *)calloc(
        screen_width * screen_height * 4, sizeof(unsigned char));

    std::vector<Matrix<float>> canny_buffers(
        3, Matrix<float>(screen_height, screen_width, 0));

    Matrix<RGB> bil_filter_buffer(screen_height, screen_width, RGB());

    Matrix<RGB> pixels_matrix(screen_height, screen_width, RGB());

    Quantizer q;
    std::vector<RGB> palette;
    tbb::concurrent_vector<size_t> palette_lightness_cumul_histo;
    int palette_number = 100;

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

    bool freeze_frame = false;
    bool frame_saved = false;
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
        if (!freeze_frame)
        {
            count =
                fread(raw_buffer, 1, screen_width * screen_height * 4, pipein);

            // If we didn't get a frame of video, we're probably at the end
            if (count != screen_width * screen_height * 4)
                break;
        }
        else
        {
            if (!frame_saved)
            {
                count = fread(saved_frame_buffer, 1,
                              screen_width * screen_height * 4, pipein);

                // If we didn't get a frame of video, we're probably at the end
                if (count != screen_width * screen_height * 4)
                    break;
                frame_saved = true;
            }
            memcpy(raw_buffer, saved_frame_buffer,
                   screen_width * screen_height * 4);
        }

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
                if (state[SDL_SCANCODE_F])
                {
                    freeze_frame = !freeze_frame;
                    frame_saved = false;
                    std::cout << "Freeze frame: "
                              << (freeze_frame ? "enabled" : "disabled")
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
                if (contrast_cor) // From raw buffer
                {
                    auto c_histo =
                        compute_lightness_cumul_histogram(raw_buffer);
                    contrast_correction(raw_buffer, c_histo);
                }

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
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        if (render_shortcuts)
        {
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderCopy(renderer, texture1, NULL, &rect1);
            SDL_RenderCopy(renderer, texture2, NULL, &rect2);
            SDL_RenderCopy(renderer, texture3, NULL, &rect3);
            SDL_RenderCopy(renderer, texture4, NULL, &rect4);
            SDL_RenderCopy(renderer, texture5, NULL, &rect5);
            SDL_RenderCopy(renderer, texture6, NULL, &rect6);
            SDL_RenderCopy(renderer, texture7, NULL, &rect7);
            SDL_RenderCopy(renderer, texture8, NULL, &rect8);
            SDL_RenderCopy(renderer, texture9, NULL, &rect9);
            SDL_RenderCopy(renderer, texture10, NULL, &rect10);
        }
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
    free(saved_frame_buffer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
