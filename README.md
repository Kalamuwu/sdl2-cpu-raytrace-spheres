# sdl2-cpu-raytrace-spheres

A simple raytracing engine with a variety of simple materials. Can run with vectors represented as either triplets of floating-point vectors, or as a single SIMD XMM register.

### Why

I've learned a lot over the course of this project and its extensions:

1. This project started with an exploration into vector math, especially in 3d, and especially-er with respects to raytracing. I used this project as a way to see if I could figure out the vector math on my own, and if I could, if it was the best-slash-industry-standard method. The "skeleton" so-to-speak of this project started from reading [P. Shirley's Ray Tracing in a Weekend](https://raytracing.github.io/), but has been *heavily* modified, with inclusions like multi-core rendering and the entirety of the SIMD vector operations, plus a few all-around optimizations. Really, the book was just used as a conceptual overview and guiding hand, with the implementation and mathematics coming from me first and foremost, with the book as an "answer key".

2. This was also the first *real* program I've written in C++, and I did it all without a tutorial! (StackOverflow not included).

3. As I've never touched C++ before, this was a brand-new step into SDL2 as well (although, mostly this was a step into the VRAM and rendering each pixel to an array, just using SDL2 to render that array to the screen.)

4. Since then, I've extended it to multi-core rendering, using a threadpool-style executor and job queue.

5. Since-er than since then, I've adapted it to CUDA to run on nVidia GPUs, and personally, gotten it pretty optimized :)

6. Since-er-er than since then, I've extended it to SIMD XMM operations. The SIMD vector implementation runs worse than the standard `float` vector implementation, yes, I realize that; the goal wasn't performance, it was to learn about SIMD. It runs slower because (1.) all the loads/reads between XMM registers and normal registers, and (2.) there are a very, very small amount of XMM registers.

### How

The file `macros.h` includes a couple tunables, for testing and setting up shots, including render size, float or SIMD vectors, and render steps and de-noising steps, among others.<br>
Note that this will need to be compiled with `-mavx` if the macro `USE_SIMD` is set to `true`.
