#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif
#include <SDL2/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <stdio.h>
#include "TextRenderer.h"
#define SIZE 1000000
#define WIDTH 1024
#define HEIGHT 768


int A[SIZE];
int B[SIZE];
int C[SIZE];
float MinRe = 0.0000829;
float MaxRe = -0.874;
float MinIm = 0.0064;
float sZoom = -0.02;
int fpsCounter = 0;
cl::Context context;
std::vector<cl::Device> devices;
cl::Kernel kernel;
SDL_Texture *screen_texture = NULL;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
TextRenderer *fontRender = NULL;


//The timer for provide FPS Count
class Timer
{
private:
    //The clock time when the timer started
    int startTicks;

    //The ticks stored when the timer was paused
    int pausedTicks;

    //The timer status
    bool paused;
    bool started;

public:
    //Initializes variables
    Timer();

    //The various clock actions
    void start();
    void stop();
    void pause();
    void unpause();

    //Gets the timer's time
    int get_ticks();

    //Checks the status of the timer
    bool is_started();
    bool is_paused();
};




//Keep track of the frame count
int frame = 0;
int CPUCount = 0;
//Timer used to calculate the frames per second
Timer fps;

//Timer used to update the caption
Timer update;


void initialize()
{


    for (int i=0; i< SIZE; i++)
    {
        A[i] = i;
        B[i] = i;
        C[i] = 0;
    }
}

bool setupOpenCL()
{
    cl_int err = CL_SUCCESS;
    std::vector<cl::Platform> platforms;

    cl::Platform::get(&platforms);
    if (platforms.size() == 0) {


        std::cout << "Platform size 0\n";
        return false;
    }

    cl_context_properties properties[] =
    {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)(platforms[0])(),
        0
    };


    context = cl::Context(CL_DEVICE_TYPE_ALL, properties);

    devices = context.getInfo<CL_CONTEXT_DEVICES>();

    std::ifstream t("kernel.cl");
    std::string kernel_str(std::istreambuf_iterator<char>(t),(std::istreambuf_iterator<char>()));
    cl::Program::Sources source(1,
                                std::make_pair( kernel_str.c_str(), kernel_str.length()+1));

    cl::Program program_ = cl::Program(context, source);
    program_.build(devices);

    kernel = cl::Kernel(program_, "mandelbrot", &err);

    return true;
}

void setupSDL2()
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();


    window = SDL_CreateWindow("",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WIDTH, HEIGHT,
                              SDL_WINDOW_RESIZABLE);

    renderer = SDL_CreateRenderer(window,
                                  -1, SDL_RENDERER_PRESENTVSYNC);

    screen_texture = SDL_CreateTexture(renderer,
                                       SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
                                       WIDTH, HEIGHT);

    fontRender = new TextRenderer(renderer,"Roboto-Light.ttf",16,SDL_Color {255,255,255,255});

}
int main(void)
{
    cl_int err = CL_SUCCESS;
    initialize();
    try {

        setupOpenCL();
        setupSDL2();



        cl::Event event;

        cl::ImageFormat clImageFormat(CL_RGBA,CL_UNSIGNED_INT8);

        char *buffer = NULL;
        cl_int errNum;
        cl::Image2D imgOutput(context,CL_MEM_WRITE_ONLY,clImageFormat,1024,768,0,buffer,&errNum);


        cl::CommandQueue queue(context, devices[0], 0, &err);


        kernel.setArg(0,imgOutput);


        int width = WIDTH;
        int height = HEIGHT;

        char* pixels = new char[width * height * 4];
        cl::size_t<3> origin;
        origin[0] = 0;
        origin[1] = 0, origin[2] = 0;
        cl::size_t<3> region;
        region[0] = width;
        region[1] = height;
        region[2] = 1;//Reset the caption




        update.start();
        fps.start();


        while (1)
        {
            SDL_Event sdl_event;
            while (SDL_PollEvent(&sdl_event))
            {

                if (sdl_event.type == SDL_QUIT) exit(0);

            }

            kernel.setArg(1,sizeof(float),&MinRe);
            kernel.setArg(2,sizeof(float),&MaxRe);
            kernel.setArg(3,sizeof(float),&MinIm);
            kernel.setArg(4,sizeof(float),&sZoom);

            queue.enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(width,height),
                cl::NDRange(1,1),
                NULL,
                &event);

            event.wait();

            queue.enqueueReadImage(imgOutput,true,origin,region,0,0,pixels,NULL,NULL);
            queue.flush();

            SDL_RenderClear(renderer);
            SDL_UpdateTexture(screen_texture, NULL, pixels, width * 4);
            SDL_RenderCopy(renderer, screen_texture, NULL, NULL);

            frame++;
              
            std::stringstream caption;

                
            
            if(  fps.get_ticks() > 1000 )
            {
                fpsCounter = frame / (int)( fps.get_ticks() / 1000.f );
                SDL_GetTicks();
            }
            
            caption << "FPS: " << fpsCounter;

            if (fontRender != NULL)
            {
                fontRender->Draw(caption.str(),20,20);
            }


            SDL_RenderPresent(renderer);
        }



    }
    catch (cl::Error err) {
        std::cerr
                << "ERROR: "
                << err.what()
                << "("
                << err.err()
                << std::endl;
    }



    return EXIT_SUCCESS;

}



Timer::Timer()
{
    //Initialize the variables
    startTicks = 0;
    pausedTicks = 0;
    paused = false;
    started = false;
}

void Timer::start()
{
    //Start the timer
    started = true;

    //Unpause the timer
    paused = false;

    //Get the current clock time
    startTicks = SDL_GetTicks();
}

void Timer::stop()
{
    //Stop the timer
    started = false;

    //Unpause the timer
    paused = false;
}

void Timer::pause()
{
    //If the timer is running and isn't already paused
    if( ( started == true ) && ( paused == false ) )
    {
        //Pause the timer
        paused = true;

        //Calculate the paused ticks
        pausedTicks = SDL_GetTicks() - startTicks;
    }
}

void Timer::unpause()
{
    //If the timer is paused
    if( paused == true )
    {
        //Unpause the timer
        paused = false;

        //Reset the starting ticks
        startTicks = SDL_GetTicks() - pausedTicks;

        //Reset the paused ticks
        pausedTicks = 0;
    }
}

int Timer::get_ticks()
{
    //If the timer is running
    if( started == true )
    {
        //If the timer is paused
        if( paused == true )
        {
            //Return the number of ticks when the timer was paused
            return pausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            return SDL_GetTicks() - startTicks;
        }
    }

    //If the timer isn't running
    return 0;
}

bool Timer::is_started()
{
    return started;
}

bool Timer::is_paused()
{
    return paused;
}

