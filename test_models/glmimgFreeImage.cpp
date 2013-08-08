/**
 * Modified by Rajesh
 */
#include <string.h>
#include "glm.h"
#include "glmint.h"

#include "FreeImage.h"
#include <stdlib.h>

GLubyte* 
glmReadFreeImage(const char* filename, GLboolean alpha, int* width, int* height, int* type)
{
    FIBITMAP *dib(0);
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    int dim;
    GLubyte *data;
	BYTE* bits(0);

	fif = FreeImage_GetFileType(filename, 0);

	if(fif == FIF_UNKNOWN) {
		fif = FreeImage_GetFIFFromFilename(filename);
	}
	//if still unkown, return failure
	if(fif == FIF_UNKNOWN){
		return NULL;
	}

	if(FreeImage_FIFSupportsReading(fif)){
		dib = FreeImage_Load(fif, filename);
	}

	if(!dib){
		return NULL;
	}
		
	bits = FreeImage_GetBits(dib);
	//get the image width and height
	*width = FreeImage_GetWidth(dib);
	*height = FreeImage_GetHeight(dib);

	if((bits == 0) || (width == 0) || (height == 0)){
		return NULL;
	}
   
    *type = alpha ? GL_RGBA : GL_RGB;
    dim = *width * *height * ((alpha) ? 4 : 3);
    data = (GLubyte*)malloc(sizeof(GLubyte) * dim);
	memset(data, 255, sizeof(GLubyte) * dim);

	if(*type == GL_RGBA){
		switch(FreeImage_GetBPP(dib)){
	    case 24:
			for(int i=0; i<FreeImage_GetWidth(dib)*FreeImage_GetHeight(dib); i++){
				data[i*4+0] = bits[i*3+2];
				data[i*4+1] = bits[i*3+1];
				data[i*4+2] = bits[i*3+0];
				//data[i*4+3] = bits[i*4+3];
			}
			break;
		case 32:
			for(int i=0; i<FreeImage_GetWidth(dib)*FreeImage_GetHeight(dib); i++){
				data[i*4+0] = bits[i*4+2];
				data[i*4+1] = bits[i*4+1];
				data[i*4+2] = bits[i*4+0];
				data[i*4+3] = bits[i*4+3];
			}
			//memcpy(data,bits,sizeof(GLubyte) * dim);
			break;
		}
	}
	else if(*type == GL_RGB){
		switch(FreeImage_GetBPP(dib)){
	    case 24:
			for(int i=0; i<FreeImage_GetWidth(dib)*FreeImage_GetHeight(dib); i++){
				data[i*3+0] = bits[i*3+2];
				data[i*3+1] = bits[i*3+1];
				data[i*3+2] = bits[i*3+0];
				//data[i*4+3] = bits[i*4+3];
			}
			break;
		case 32:
			for(int i=0; i<FreeImage_GetWidth(dib)*FreeImage_GetHeight(dib); i++){
				data[i*3+0] = bits[i*4+2];
				data[i*3+1] = bits[i*4+1];
				data[i*3+2] = bits[i*4+0];
			}
			//memcpy(data,bits,sizeof(GLubyte) * dim);
			break;
		}
	}
    //ilCopyPixels( 0, 0, 0, *width, *height, 1, format, IL_UNSIGNED_BYTE, data);
    FreeImage_Unload(dib);
    return data;
}
