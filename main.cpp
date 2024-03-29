#include <time.h>
#include <SDL2/SDL.h>

#include "macros.h"
#include "screen.h"

#if USE_SIMD == true
    #include "simd/vector.h"
    #include "simd/ray.h"
    #include "simd/camera.h"
    #include "simd/hitable.h"
    #include "simd/hitable_list.h"
    #include "simd/sphere.h"
    #include "simd/material.h"
    #include "simd/thread_pool.h"
#else
    #include "float/vector.h"
    #include "float/ray.h"
    #include "float/camera.h"
    #include "float/hitable.h"
    #include "float/hitable_list.h"
    #include "float/sphere.h"
    #include "float/material.h"
    #include "float/thread_pool.h"
#endif

// inline vec3 process_float(vec3 v1, vec3 v2) {
//     const __m128 XMM_POS_2_loc = _mm_set1_ps(2.0);
//     const __m128 XMM_NEG_1_loc = _mm_set1_ps(-1.0);
//     __m128 r;
//     vec3 p;
//     do {
//         // pick random point in unit cube
//         r = _mm_set_ps(drand48(), drand48(), drand48(), 0);
//         p = vec3(_mm_fmadd_ps(r, XMM_POS_2_loc, XMM_NEG_1_loc));  // 2x-1 ; scales [0,1] to [-1,1]
//     // reject while not in unit sphere; this happens when : sqrt(x^2 + y^2 + z^2) > 1
//     } while (p.squared_length() > 1);
//     return p;
// }
//
// static const __m128 XMM_POS_2 = _mm_set1_ps(2.0);
// static const __m128 XMM_NEG_1 = _mm_set1_ps(-1.0);
//
// inline vec3 process_simd(vec3 v1, vec3 v2) {
//     __m128 r;
//     vec3 p;
//     do {
//         // pick random point in unit cube
//         r = _mm_set_ps(drand48(), drand48(), drand48(), 0);
//         p = vec3(_mm_fmadd_ps(r, XMM_POS_2, XMM_NEG_1));  // 2x-1 ; scales [0,1] to [-1,1]
//     // reject while not in unit sphere; this happens when : sqrt(x^2 + y^2 + z^2) > 1
//     } while (p.squared_length() > 1);
//     return p;
// }
//
// int _main()
// {
//     clock_t init, start_float, stop_float, start_simd, stop_simd;
//
//     long n = 10;//INT16_MAX;
//     vec3 vecs_a[n];
//     vec3 vecs_b[n];
//     vec3 out_float[n];
//     vec3 out_simd[n];
//
//     init = clock();
//     for (long i = 0; i < n; i++)
//     {
//         vecs_a[i] = vec3(0,0,0);//random_in_unit_sphere();
//         vecs_b[i] = vec3(0,0,0);//random_in_unit_sphere();
//     }
//
//     start_float = clock();
//     for (long i = 0; i < n; i++)
//     {
//         out_float[i] = process_float(vecs_a[i], vecs_b[i]);
//     }
//     stop_float = clock();
//
//     start_simd = clock();
//     for (long i = 0; i < n; i++)
//     {
//         out_simd[i] = process_simd(vecs_a[i], vecs_b[i]);
//     }
//     stop_simd = clock();
//
//     // std::cout << "use_simd=" << USE_SIMD << std::endl;
//     // std::cout << "n=" << n << std::endl;
//
//     // double init_seconds = ((double)(start_float - init)) / CLOCKS_PER_SEC;
//     double float_seconds = ((double)(stop_float - start_float)) / CLOCKS_PER_SEC;
//     double simd_seconds = ((double)(stop_simd - start_simd)) / CLOCKS_PER_SEC;
//     std::cout << "locals took:  " << float_seconds << std::endl;
//     std::cout << "global took:   " << simd_seconds << std::endl;
//
//     return 0;
// }

// gross but functional
// displays output i.e.    23% done   [||||___________]
float displ_progress(int done, int total, int width)
{
    float perc_done = float(done) / total;
    char buf[width+1];
    for (int i = 0; i < width; i++)
        buf[i] = (i < width*perc_done)? '|' : '_';
    buf[width] = 0;
    printf(
        "\r%3d%% done  %s[%s]",
        int(perc_done * 100),
        ((perc_done < 0.1f)? "  " : ((perc_done == 1.0f)? "" : " ")),
        buf);
    return perc_done;
}


int main()
{
    clock_t setup_start, render_start, render_stop;
    setup_start = clock();

#if USE_SIMD == true
    std::cout << "Using SIMD vectors" << std::endl;
#else
    std::cout << "Using standard float vectors" << std::endl;
#endif

    int num_pixels = WINDOW_WIDTH * WINDOW_HEIGHT;

    Hitable *dList[9];
    dList[0] = new Sphere(vec3( 0,   100.6, -2  ), 100, new Diffuse(   vec3(0.3, 0.5, 0.7)                ));
    dList[1] = new Sphere(vec3( 0,     0,   -2  ), 0.5, new Diffuse(   vec3(0.8, 0.3, 0.3)                ));
    dList[2] = new Sphere(vec3( 2.6,  -1.4, -1.7), 0.7, new Metal(     vec3(0.7, 0.7, 0.7),   0.4         ));
    dList[3] = new Sphere(vec3( 1,     0,   -2  ), 0.4, new Metal(     vec3(0.3, 0.4, 0.9),   0.05        ));
    dList[4] = new Sphere(vec3(-0.3,   0.1, -1  ), 0.3, new Glass(     vec3(0.5, 1.0, 0.6),   0.9         ));
    dList[5] = new Sphere(vec3( 0,     0.2,  1  ), 0.3, new Glass(     vec3(0.8, 0.2, 0.3),   0.0         ));
    dList[6] = new Sphere(vec3(-1,    -0.3, -1.2), 0.2, new Emmissive( vec3(0.3, 0.2, 0.0),   9.0,  false ));
    dList[7] = new Sphere(vec3( 0.3,  -0.5, -1.1), 0.2, new Emmissive( vec3(0.0, 0.1, 0.9),  10.0,  false ));
    dList[8] = new Sphere(vec3( 0,    -5.0, -3  ), 2.0, new Emmissive( vec3(1.0, 1.0, 1.0),   1.0,  false ));
    Hitable *pWorld = new HitableList(dList, 9);

    // Hitable *dList[1];
    // dList[0] = new Sphere(vec3(0, 0, -2), 0.5, new Diffuse(vec3(0.8, 0.3, 0.3)));
    // Hitable *pWorld = new HitableList(dList, 1);

    Camera cam(
            vec3(-1,0,2),
            vec3(0,0,-1),
            vec3(0,1,0),
            70,
            (float)WINDOW_WIDTH/WINDOW_HEIGHT
        );

    ThreadPool pool(NUM_THREADS);
    uint32_t *pFrameBuffer = new uint32_t[num_pixels];
    threadInfo globalInfo {
        pWorld,
        &cam,
        pFrameBuffer
    };
    pool.init(&globalInfo);

    SDL_Event e;
    Screen screen(WINDOW_WIDTH, WINDOW_HEIGHT, 1);
    free(screen.pTextureBuffer);
    screen.pTextureBuffer = pFrameBuffer;
    //screen.show(); // first draw -- black screen

    render_start = clock();
    pool.start();
    printf("ThreadPool started. Using %d threads.\n", pool.getNumThreads());

    int i = 0;
    int x = 0;
    int y = 0;

    while (true)
    {
        //screen.show();
        if (pool.running() && pool.shouldTerminate)
        {
            screen.show();
            render_stop = clock();
            pool.stop();

            double setup_seconds =  ((double)(render_start - setup_start)) / CLOCKS_PER_SEC;
            double render_seconds = ((double)(render_stop - render_start)) / CLOCKS_PER_SEC;
            printf("Render complete.\n");
            printf("Setup took:  %.3f seconds.\n", setup_seconds);
            printf("Render took: %.3f seconds.\n", render_seconds);
            printf("Render took: %.3f scaled seconds.\n", render_seconds/NUM_THREADS);
        }
        while (SDL_PollEvent(&e))
            if (e.type == SDL_QUIT) goto quit;
        SDL_Delay(10);
    }

quit:
    if (pool.running()) pool.stop();
    screen.show();
    free(pWorld);
    free(pFrameBuffer);
    screen.quit(false);
    SDL_Quit();
    exit(0);


    return 0;
}
