#ifndef THREADPOOLH
#define THREADPOOLH

#include <thread>
#include <mutex>
#include <iostream>
#include <cassert>

#include "vector.h"
#include "ray.h"
#include "hitable.h"
#include "camera.h"
#include "material.h"

struct threadInfo
{
    Hitable *pWorld;
    Camera *pCam;
    uint32_t *pTextureBuffer;
};

class ThreadPool
{
public:
    ThreadPool(uint32_t num_threads);

    void init(threadInfo* global);
    void start();
    void stop();
    bool busy();
    uint32_t num_remaining();
    uint32_t num_consumed();
    uint32_t num_total() { return m_total; }
    uint32_t getNumThreads() { return m_num_threads; }
    bool running();
    bool shouldTerminate = false;            // Tells threads to stop looking for jobs

private:
    void m_threadLoop();

    bool m_is_running = false;
    std::mutex m_queueMutex;                   // Job queue race condition lock
    uint32_t m_num_threads;
    std::vector<std::thread> m_threads;
    threadInfo* m_globalInfoPtr;
    uint32_t m_numConsumedSoFar;
    uint32_t m_total;

    void doRayTrace(threadInfo *pGlobalInfo, int index);
    vec3 color(ray& r, Hitable* pWorld);
};


ThreadPool::ThreadPool(uint32_t n_threads)
{
    // std::thread::hardware_concurrency() returns the maximum amount
    // of threads the system can support at once. Here I subtract one
    // because I don't want my entire system to crawl to a halt. That,
    // and the main thread still needs attention!
    const uint32_t m = std::thread::hardware_concurrency() - 1;
    assert(m>0);
    if (n_threads > m) m_num_threads = m;
    else if (n_threads < 1) m_num_threads = 1;
    else m_num_threads = n_threads;
    m_threads = std::vector<std::thread>(m_num_threads);
    m_total = 0;
}

void ThreadPool::init(threadInfo* global)
{
    m_globalInfoPtr = global;
    m_total = WINDOW_WIDTH * WINDOW_HEIGHT;
}

// initialize all threads
void ThreadPool::start()
{
    shouldTerminate = false;
    m_numConsumedSoFar = 0;
    for (uint32_t i = 0; i < m_num_threads; i++)
    {
        m_threads[i] = std::thread(&ThreadPool::m_threadLoop,this);
    }
    m_is_running = true;
}

void ThreadPool::stop() {
    shouldTerminate = true;
    for (std::thread& active_thread : m_threads) {
        active_thread.join();
    }
    m_threads.clear();
    m_is_running = false;
}

bool ThreadPool::busy() {
    bool poolbusy;
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        poolbusy = m_numConsumedSoFar < m_total;
    }
    return poolbusy;
}

bool ThreadPool::running() {
    return m_is_running;
}

uint32_t ThreadPool::num_remaining() {
    uint32_t len;
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        len = m_numConsumedSoFar;
    }
    return m_total - len;
}

uint32_t ThreadPool::num_consumed() {
    uint32_t len;
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        len = m_numConsumedSoFar;
    }
    return len;
}

void ThreadPool::m_threadLoop()
{
    while (!shouldTerminate) {
        int index;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            shouldTerminate |= (m_numConsumedSoFar >= m_total);
            if (shouldTerminate) break;
            index = m_numConsumedSoFar++;
        }
        // printf("m_threadLoop Popped (%d,%d) %#x %#x\n", jobptr->x, jobptr->y, &jobptr->x, &jobptr->y);
        // printf("m_threadLoop Length now: %d\n", this->length());
        doRayTrace(m_globalInfoPtr, index);
    }
    printf("Thread stopped.\n");
    return;
}

vec3 ThreadPool::color(ray& r, Hitable* pWorld)
{
    vec3 runningAttenuation = vec3(1,1,1);
    hit_record rec;
    vec3 attenuation;
    for (int i = 0; i < MAX_NUM_REFLECTIONS; i++)  // do MAX_NUM_REFLECTIONS reflections
    {
        bool isLightSource = false;
        if (pWorld->hit(r, 0.0001f, FLT_MAX, rec))
        {
            if (rec.pMat->scatter(r, rec, attenuation, isLightSource))
                runningAttenuation *= attenuation;
            else return (isLightSource)? runningAttenuation * attenuation : vec3(0,0,0);
        }
        else return runningAttenuation * SKYBOX_COLOR;
        // // lerp white...blue and multiply by attenuation
        // vec3 unit_direction = unit_vector(r.direction());
        // float t = 0.5f*(unit_direction.y()+1.0f);
        // return runningAttenuation * ((1.0f-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0));
    }

    // exceeded recursion
    return vec3(0,0,0);
}

void ThreadPool::doRayTrace(threadInfo *pGlobalInfo, int index)
{
    const int x = index % WINDOW_WIDTH;
    const int y = index * (1.0f / WINDOW_WIDTH);
    //printf("(%d,%d) starting.\n", x, y);
    // average the colors over num_its iterations. not only does
    // this achieve basic antialiasing, but also smoothes out the
    // render artifacts and raytracing noise.
    vec3 col(0,0,0);
    for (int iter = 0; iter < NUM_ALIAS_STEPS; iter++)
    {
        // add drand48() for a slight randomization to the direction.
        // this non-uniformity is what achieves the above benefits.
        float u = float(x + drand48()) * (1.0f / WINDOW_WIDTH);
        float v = float(y + drand48()) * (1.0f / WINDOW_HEIGHT);

        ray r = pGlobalInfo->pCam->getRay(u, v);

        // clamping the colors to 0-1 is important because light sources
        // can go above that, and cause overflow issues and really strange
        // visual glitches.
        col += color(r, pGlobalInfo->pWorld).clamp(0.0f, 1.0f);
    }
    col /= NUM_ALIAS_STEPS;
    // A square root is present because SDL assumes the image is gamma-
    // corrected. It is not. This is corrected by raising the color to
    // the power of 1/gamma; to simplify this math, gamma=2 is used.
    // The rest of this mess scales 0..1 --> 0..255, and converts to
    // uint8_t to chain together.
    col = vec3(_mm_mul_ps(_mm_sqrt_ps(col.xmm), _mm_set1_ps(255.99f)));
    uint8_t ir = uint8_t(col.r());
    uint8_t ig = uint8_t(col.g());
    uint8_t ib = uint8_t(col.b());

    // write directly to pixel buffer for efficiency
    #if __BYTE_ORDER == __LITTLE_ENDIAN
        pGlobalInfo->pTextureBuffer[index] = (ir<<24) | (ig<<16) | (ib<<8) | 0xFF;
    #elif __BYTE_ORDER == __BIG_ENDIAN
        pGlobalInfo->pTextureBuffer[index] = (0xFF<<24) | (ib<<16) | (ig<<8) | ir;
    #else
    # error "Please fix <bits/endian.h>"
    #endif
}

#endif
