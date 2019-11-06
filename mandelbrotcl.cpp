#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif
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


const char *simple_add_str  = "   void kernel simple_add(global const int* A, global const int* B, global int* C){       "
            "       C[get_global_id(0)]=A[get_global_id(0)]+B[get_global_id(0)];                 "
            "   }   ";



void initialize()
{


  for (int i=0;i< SIZE;i++)
    {
      A[i] = i;
      B[i] = i;
      C[i] = 0;
    }
}

int main(void)
{
	cl_int err = CL_SUCCESS;
	initialize();
	try {

		std::vector<cl::Platform> platforms;
        cl_ulong mem_size;
		cl::Platform::get(&platforms);
		if (platforms.size() == 0) {
		  
			std::cout << "Platform size 0\n";
			return -1;
		}

		cl_context_properties properties[] =
		{
				CL_CONTEXT_PLATFORM,
				(cl_context_properties)(platforms[0])(),
				0
		};
        
		cl::Context context(CL_DEVICE_TYPE_ALL, properties);

		std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
		
		std::ifstream t("kernel.cl");
		std::string kernel_str(std::istreambuf_iterator(t),(std::istreambuf_iterator()));
		cl::Program::Sources source(1,
				std::make_pair( kernel_str.c_str(), kernel_str.lenght()+1));
        
		cl::Program program_ = cl::Program(context, source);
		program_.build(devices);

		cl::Kernel kernel(program_, "simple_add", &err);

		cl::Event event;
		//cl::Device default_device=devices[0];
		cl::Buffer buffer_A(context,CL_MEM_READ_WRITE,sizeof(int)*SIZE);
		cl::Buffer buffer_B(context,CL_MEM_READ_WRITE,sizeof(int)*SIZE);
		cl::Buffer buffer_C(context,CL_MEM_READ_WRITE,sizeof(int)*SIZE);
		
		devices[0].getInfo(CL_DEVICE_LOCAL_MEM_SIZE, &mem_size);
        std::cout << "Local memory Size: " << (mem_size/1024) << " Kbytes " << std::endl;
        
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
        
        for (int i=0;i<SIZE;i++)
        {
            int x = C[i];
        }
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
