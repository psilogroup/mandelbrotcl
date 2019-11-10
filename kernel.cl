


void kernel mandelbrot(__write_only image2d_t imgout,float MinRe,float MaxRe,float MinIm,float sZoom )
{
    

int screen_width = 1024;
int screen_height = 768;
int MaxIterations = 1024;
float MaxIm = MinIm+(MaxRe-MinRe)*screen_height/screen_width;
float Re_factor = +(MaxRe-MinRe)/(screen_width-1);
float Im_factor = (MaxIm-MinIm)/(screen_height-1);
    
int2 coord = (int2)(get_global_id(0), get_global_id(1));
uint4 color;
color.x = 255;
color.y = 255;
color.z = 255;
color.w = 255;

const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | //Natural coordinates
                      CLK_ADDRESS_CLAMP | //Clamp to zeros
                      CLK_FILTER_NEAREST; //Don't interpolate
                      

float c_im = MaxIm - coord.y*Im_factor*sZoom;
float c_re = MinRe + coord.x*Re_factor*sZoom;
float Z_re = c_re, Z_im = c_im;
bool isInside = true;
int n = 0;
for(n=0; n<MaxIterations; ++n)
{
    float Z_re2 = Z_re*Z_re, Z_im2 = Z_im*Z_im;
    
    if(Z_re2 + Z_im2 > 4)
    {
        isInside = false;
        break;
    }
    else
    {
        color.xyzw = (uint4)(255,0,n,n*2);

    }
    Z_im = 2*Z_re*Z_im + c_im;
    Z_re = Z_re2 - Z_im2 + c_re;
}
    

 if(isInside)
 {
     color.xyzw = (uint4)(0,0,0,0);
 }
 
                      
write_imageui(imgout, coord, color);
} 
