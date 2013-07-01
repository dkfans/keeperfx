#pragma once

#include <string>
#include <vector>

#if __GNUC__ > 2
#include <ext/hash_map>
#else
#include <hash_map>
#endif
namespace __gnu_cxx{};
using namespace __gnu_cxx;
//#include <unordered_map>

#include <png.h>

#define ADDITIONAL_DATA_LEN 16

class ProgramOptions;

typedef unsigned long RGBAQuad;
template <typename T>
class RGBValues {
public:
    RGBValues(): red(0),green(0),blue(0) {};
    RGBValues(RGBAQuad quad) { fromQuad(quad); };
    void fromQuad(RGBAQuad quad) {
        red = quad&255;
        green = (quad>>8)&255;
        blue = (quad>>16)&255;
    }
    T red;
    T green;
    T blue;
};

template <typename T>
class Vector2d : public std::vector<std::vector<T> >
{
public:
    typedef std::vector<T> Column;
    void resize2d(int dim_x, int dim_y)
    {
        this->resize(dim_y);
        for (auto it=this->begin(); it < this->end(); it++)
            it->resize(dim_x);
    }
    void zeroize2d()
    {
        for (auto it=this->begin(); it < this->end(); it++)
            std::fill(it->begin(),it->end(),0);
    }
};

typedef RGBValues<long> RGBAccum;
typedef Vector2d<bool> ColorTranparency;
typedef RGBValues<unsigned char> RGBColor;
typedef hash_map<RGBAQuad,signed int> MapQuadToPal;
typedef std::vector<RGBColor> ColorPalette;
typedef Vector2d<float> DitherError;

class ImageData
{
public:
    ImageData():png_ptr(NULL),info_ptr(NULL),end_info(NULL),width(0),height(0),
          col_bits(0),transparency_threshold(196){}
    int colorBPP(void) const
    { return col_bits; }
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;
    png_uint_32 width, height;
    ColorTranparency transMap;
    int color_type;
    int col_bits;
    int transparency_threshold;
    /** Any additional data required for specific output file format */
    unsigned char additional_data[ADDITIONAL_DATA_LEN];
};

short load_inp_png_file(ImageData& img, const std::string& fname_inp, ProgramOptions& opts);
