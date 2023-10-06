// renders a Scalable Vector Graphic (SVG)
// a JPG or a PNG
// as an ASCII image
// takes a filename and an optional
// numeric percentage for scaling
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gfx.hpp>
using namespace gfx;

// prints a source as 4-bit grayscale ASCII
template <typename Source>
void print_ascii(const Source& src) {
    // the color table
    static const char* col_table = " .,-~;+=x!1%$O@#";
    // move through the draw source
    for (int y = 0; y < src.dimensions().height; ++y) {
        for (int x = 0; x < src.dimensions().width; ++x) {
            typename Source::pixel_type px;
            // get the pixel at the current point
            src.point(point16(x, y), &px);
            // convert it to 4-bit grayscale (0-15)
            const auto px2 = convert<typename Source::pixel_type, gsc_pixel<4>>(px);
            // get the solitary "L" (luminosity) channel value off the pixel
            size_t i = px2.template channel<channel_name::L>();
            // use it as an index into the color table
            putchar(col_table[i]);
        }
        putchar('\r');
        putchar('\n');
    }
}
int main(int argc, char** argv) {
    if (argc > 1) {       // at least 1 param
        float scale = 1;  // scale of SVG
        if (argc > 2) {   // 2nd arg is scale percentage
            int pct = atoi(argv[2]);
            if (pct > 0 && pct <= 1000) {
                scale = ((float)pct / 100.0f);
            }
        }
        // open the file
        file_stream fs(argv[1]);
        size_t arglen = strlen(argv[1]);
        bool png = false;
        bool jpg = false;
        if (arglen > 4) {
            if (0 == stricmp(argv[1] + arglen - 4, ".svg")) {
                svg_doc doc;
                // read it
                svg_doc::read(&fs, &doc);
                fs.close();
                // create a bitmap the size of our final scaled SVG
                auto bmp = create_bitmap<gsc_pixel<4>>(
                    {uint16_t(doc.dimensions().width * scale),
                     uint16_t(doc.dimensions().height * scale)});
                // if not out of mem allocating bitmap
                if (bmp.begin()) {
                    // clear it
                    bmp.clear(bmp.bounds());
                    // draw the SVG
                    draw::svg(bmp, bmp.bounds(), doc, scale);
                    // dump as ascii
                    print_ascii(bmp);
                    // free the bmp
                    free(bmp.begin());
                }
                return 0;
            } else if (0 == stricmp(argv[1] + arglen - 4, ".jpg")) {
                jpg = true;
            } else if (0 == stricmp(argv[1] + arglen - 4, ".png")) {
                png = true;
            }
            if (jpg || png) {
                int result = 1;
                size16 dim;
                if (gfx_result::success == (jpg ? jpeg_image::dimensions(&fs, &dim) : png_image::dimensions(&fs, &dim))) {
                    fs.seek(0);
                    auto bmp_original = create_bitmap<gsc_pixel<4>>(
                        {uint16_t(dim.width),
                         uint16_t(dim.height)});
                    if (bmp_original.begin()) {
                        bmp_original.clear(bmp_original.bounds());
                        draw::image(bmp_original, bmp_original.bounds(), &fs);
                        fs.close();
                        if (scale != 1) {
                            // create a bitmap the size of our final scaled JPG
                            auto bmp = create_bitmap<gsc_pixel<4>>(
                                {uint16_t(dim.width * scale),
                                 uint16_t(dim.height * scale)});
                            // if not out of mem allocating bitmap
                            if (bmp.begin()) {
                                // clear it
                                bmp.clear(bmp.bounds());
                                // draw the SVG
                                if (scale < 1) {
                                    draw::bitmap(bmp, bmp.bounds(), bmp_original, bmp_original.bounds(), bitmap_resize::resize_bicubic);
                                } else {
                                    draw::bitmap(bmp, bmp.bounds(), bmp_original, bmp_original.bounds(), bitmap_resize::resize_bilinear);
                                }
                                result = 0;
                                // dump as ascii
                                print_ascii(bmp);
                                // free the bmp
                                free(bmp.begin());
                            }
                        } else {
                            result = 0;
                            // dump as ascii
                            print_ascii(bmp_original);
                        }
                        free(bmp_original.begin());
                        return result;
                    }
                }
            }
        }
    }
    return 1;
}