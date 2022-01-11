#include <stdio.h>
#include <string.h>
#include "tiffio.h"

#include <string>
#include <vector>

struct pix {
	unsigned short r, g, b;
};

void err(std::string msg)
{
	fprintf(stderr,"%s\n",msg.c_str());
	fflush(stdout);
	exit(1);
}

int main(int argc, char ** argv)
{
	char *img, *buf;
	FILE * infile;
	std::string intiff, outtiff;
	uint32_t w, h;
	uint16_t c, b, config;
	

	TIFFSetErrorHandler(0);

	if (argc < 3) 
			err("Usage: quadtiff2rgbtiff quad.tif rgb.tif\n");
	
	intiff = std::string(argv[1]);
	outtiff = std::string(argv[2]);
	
	TIFF* intif = TIFFOpen(intiff.c_str(), "r");
	if (intif) {
		
		TIFFGetField(intif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(intif, TIFFTAG_IMAGELENGTH, &h);
		TIFFGetField(intif, TIFFTAG_SAMPLESPERPIXEL, &c);
		TIFFGetField(intif, TIFFTAG_BITSPERSAMPLE, &b);
		TIFFGetField(intif, TIFFTAG_PLANARCONFIG, &config);
		
		if (config != PLANARCONFIG_CONTIG) err("Error: TIFF file not planar.");
		if (c != 1) err("Error: image not single-channel.");
		//if (b != 16) err("Error: image not 16-bit integer");
		
		img = new char[w*h*c*(b/8)];
		buf = (char *) _TIFFmalloc(TIFFScanlineSize(intif));
		int stride = TIFFScanlineSize(intif);
		
		char * dst = (char *) img;
		for (unsigned y = 0; y < h; y++){
			TIFFReadScanline(intif, buf, y, 0);
			memcpy(dst,buf,stride);
			dst += stride;
		}
		
		unsigned short * image = (unsigned short *) img;
		
		

		
		//quad-bayer demosaic:

		unsigned qfarray[4][4] = {
			{0,0,1,1},
			{0,0,1,1},
			{3,3,2,2},
			{3,3,2,2}
		};

		int arraydim = 4;

		std::vector<pix> quadimage;
		quadimage.resize((h/4)*(w/4));

		for (unsigned y=0; y<h-(arraydim-1); y+=arraydim) {
			for (unsigned x=0; x<w-(arraydim-1); x+=arraydim) {
				unsigned Hpos = (x/4) + (y/4)*(w/4);
				float pix[4] = {0.0, 0.0, 0.0, 0.0};
				for (unsigned r=0; r<arraydim; r++) {  //walk the 16x16 image subset, collect the channel values 
					for (unsigned c=0; c<arraydim; c++) {
						int pos = (x+c) + (y+r) * w;
						pix[ qfarray[r][c] ] += image[pos]; 
					}
				}
				
				for (unsigned i=0; i<4; i++) pix[i] /= 4.0;  //calculate quad average
				
				pix[1] = (pix[1] + pix[3]) / 2.0; //make a single green of G1 and G2

				//put the result in the appropriate place in the halfsize image:
				quadimage[Hpos].r = (unsigned short) pix[0];
				quadimage[Hpos].g = (unsigned short) pix[1];
				quadimage[Hpos].b = (unsigned short) pix[2];
			}
		}





		//write TIFF:
		
		TIFF* outtif = TIFFOpen(outtiff.c_str(), "w");
		if (outtif) {

			TIFFSetField(outtif, TIFFTAG_IMAGEWIDTH, w/4);  
			TIFFSetField(outtif, TIFFTAG_IMAGELENGTH, h/4);    
			TIFFSetField(outtif, TIFFTAG_SAMPLESPERPIXEL, 3);   
			TIFFSetField(outtif, TIFFTAG_BITSPERSAMPLE, 16);
			TIFFSetField(outtif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
			TIFFSetField(outtif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);   

			TIFFSetField(outtif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
			TIFFSetField(outtif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
			TIFFSetField(outtif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
			// We set the strip size of the file to be size of one row of pixels
			TIFFSetField(outtif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(outtif, (w/4)*3));
			
			unsigned scanlinesize = TIFFScanlineSize(outtif);
			unsigned char * outbuf = (unsigned char *) _TIFFmalloc(scanlinesize);
			char * outimg = (char *) quadimage.data();

			for (unsigned row = 0; row < h/4; row++)
			{
				memcpy(outbuf, outimg, scanlinesize);
				if (TIFFWriteScanline(outtif, outbuf, row, 0) < 0) {
					printf("TIFFWriteScanline got an error...\n");
					TIFFError(NULL,NULL);
					break;
				}
				outimg+=scanlinesize;
			}

			
			if (outbuf) _TIFFfree(outbuf);
			TIFFClose(outtif);
			
		}
		
		
		if (buf) _TIFFfree(buf);
		TIFFClose(intif);
	}
	else err("Error: file failed to open.");
	
}
