#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif
#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>

#define SIZE 1000000

int A[SIZE];
int B[SIZE];
int C[SIZE];

cl::Context context;
std::vector<cl::Device> devices;
cl::Kernel kernel;

void initialize()
{


  for (int i=0;i< SIZE;i++)
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

int main(void)
{
	cl_int err = CL_SUCCESS;
	initialize();
	try {
        
        setupOpenCL();
		
		
		

		cl::Event event;
		cl::Buffer buffer_A(context,CL_MEM_READ_WRITE,sizeof(int)*SIZE);
		cl::Buffer buffer_B(context,CL_MEM_READ_WRITE,sizeof(int)*SIZE);
		cl::Buffer buffer_C(context,CL_MEM_READ_WRITE,sizeof(int)*SIZE);
        cl::ImageFormat clImageFormat(CL_RGBA,CL_UNORM_INT8);
   
        char *buffer = NULL;
        cl_int errNum;
        cl::Image2D imgInput(context,CL_MEM_WRITE_ONLY,clImageFormat,1024,768,0,buffer,&errNum);
		

		cl::CommandQueue queue(context, devices[0], 0, &err);
		queue.enqueueWriteBuffer(buffer_A,CL_TRUE,0,sizeof(int)*SIZE,A);
        queue.enqueueWriteBuffer(buffer_B,CL_TRUE,0,sizeof(int)*SIZE,B);
        
        kernel.setArg(0,buffer_A);
        kernel.setArg(1,buffer_B);
        kernel.setArg(2,buffer_C);
        
		queue.enqueueNDRangeKernel(
				kernel,
				cl::NullRange,
				cl::NDRange(SIZE),
				cl::NullRange,
				NULL,
				&event);

		event.wait();
        
        queue.enqueueReadBuffer(buffer_C,CL_TRUE,0,sizeof(int)*SIZE,C);
        
	}
	catch (cl::Error err) {
		std::cerr
		<< "ERROR: "
		<< err.what()
		<< "("
		<< err.err()
		<< ")"
		<< std::endl;
	}

	return EXIT_SUCCESS;

}
