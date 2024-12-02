// renders a Scalable Vector Graphic (SVG)
// a JPG or a PNG or text
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
static unsigned char to_lower_u(unsigned char ch) {
    if (ch >= 'A' && ch <= 'Z')
        ch = 'a' + (ch - 'A');
    return ch;
}
// from Zohar81 @ https://stackoverflow.com/questions/5820810/case-insensitive-string-comparison-in-c
static int strcmp_i(const char *s1, const char *s2) {
    const unsigned char *us1 = (const unsigned char *)s1,
                        *us2 = (const unsigned char *)s2;

    while (to_lower_u(*us1) == to_lower_u(*us2++))
        if (*us1++ == '\0')
            return (0);
    return (to_lower_u(*us1) - to_lower_u(*--us2));
}

int main(int argc, char **argv) {
    if (argc > 1) {       // at least 1 param
        float scale = 1;  // scale of image
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
            if (0 == strcmp_i(argv[1] + arglen - 4, ".svg")) {
                sizef dim;
                if(gfx_result::success!=canvas::svg_dimensions(fs,&dim)) {
                    fputs("Unable to read SVG",stderr);
                    fs.close();
                    return 1;
                }
                fs.seek(0);
                canvas cvs(size16(ceilf(dim.width),ceilf(dim.height)));
                
                // create a bitmap the size of our final scaled SVG
                auto bmp = create_bitmap<gsc_pixel<4>>(
                    {uint16_t(cvs.dimensions().width * scale),
                     uint16_t(cvs.dimensions().height * scale)});
                // if not out of mem allocating bitmap
                if (gfx_result::success==cvs.initialize() && bmp.begin()) {
                    // clear it
                    bmp.clear(bmp.bounds());
                    // bind the canvas to it
                    draw::canvas(bmp,cvs,point16::zero());
                    matrix m=matrix::create_fit_to(dim,(rectf)bmp.bounds());
                    // draw the SVG
                    if(gfx_result::success==cvs.render_svg(fs,m)) {
                        // dump as ascii
                        print_ascii(bmp);
                    } else {
                        fputs("Error reading SVG",stderr);
                    }
                    // free the bmp
                    free(bmp.begin());
                    fs.close();
                
                } else { 
                    fs.close();
                }
                return 0;
            } else if (0 == strcmp_i(argv[1] + arglen - 4, ".jpg")) {
                jpg = true;
            } else if (0 == strcmp_i(argv[1] + arglen - 4, ".png")) {
                png = true;
            } else if (0 == strcmp_i(argv[1] + arglen - 4, ".ttf") || 0 == strcmp_i(argv[1] + arglen - 4, ".otf")) {
                if(argc<4) {
                    fputs("Not enough arguments",stderr);
                    return 1;
                }
                float lh = scale*100.0f;
                file_stream fs(argv[1]);
                tt_font text_font(fs,lh,font_size_units::px);
                if(gfx_result::success!=text_font.initialize()) {
                    fputs("Error reading font",stderr);
                }
                text_info oti;
                oti.text_font = &text_font;
                oti.encoding = &text_encoding::utf8;
                oti.text_sz(argv[3]);
                size16 txtsz;
                oti.text_font->measure(-1,oti,&txtsz);
                rect16 text_rect = txtsz.bounds();
                auto bmp = create_bitmap<gsc_pixel<4>>(text_rect.dimensions());
                if(bmp.begin()) {
                    bmp.clear(bmp.bounds());
                    draw::text(bmp,bmp.bounds(),oti,color<gsc_pixel<4>>::white);
                    // dump as ascii
                    print_ascii(bmp);
                    free(bmp.begin());
                    return 0;
                } 
                fprintf(stderr, "Out of memory creating bitmap\r\n");
                return 1;
            }
            if (jpg || png) {
                int result = 1;
                size16 dim;
                image* img = nullptr;
                jpg_image jimg;
                png_image pimg;
                if(jpg) {
                    jimg=jpg_image(fs);
                    img=&jimg;
                } else {
                    pimg=png_image(fs);
                    img=&pimg;
                }
                gfx_result r = img->initialize();
                if (gfx_result::success == r) {
                    dim = img->dimensions();
                    fs.seek(0);
                    auto bmp_original = create_bitmap<gsc_pixel<4>>(
                        {uint16_t(dim.width),
                            uint16_t(dim.height)});
                    if (bmp_original.begin()) {
                        bmp_original.clear(bmp_original.bounds());
                        r = draw::image(bmp_original, bmp_original.bounds(), *img);
                        if (gfx_result::success == r) {
                            fs.close();
                            if (scale != 1) {
                                // create a bitmap the size of our final scaled image
                                auto bmp = create_bitmap<gsc_pixel<4>>(
                                    {uint16_t(dim.width * scale),
                                        uint16_t(dim.height * scale)});
                                // if not out of mem allocating bitmap
                                if (bmp.begin()) {
                                    // clear it
                                    bmp.clear(bmp.bounds());
                                    // draw the SVG
                                    if (scale < 1) {
                                        draw::bitmap(bmp,
                                                        bmp.bounds(),
                                                        bmp_original,
                                                        bmp_original.bounds(),
                                                        bitmap_resize::resize_bicubic);
                                    } else {
                                        draw::bitmap(bmp,
                                                        bmp.bounds(),
                                                        bmp_original,
                                                        bmp_original.bounds(),
                                                        bitmap_resize::resize_bilinear);
                                    }
                                    result = 0;
                                    // dump as ascii
                                    print_ascii(bmp);
                                    // free the bmp
                                    free(bmp.begin());
                                } else {
                                    fprintf(stderr, "Out of memory creating final bitmap\r\n", (int)r);
                                }
                            } else {
                                result = 0;
                                // dump as ascii
                                print_ascii(bmp_original);
                            }
                        } else {
                            fprintf(stderr, "Error drawing image. Error: %d\r\n", (int)r);
                        }
                        free(bmp_original.begin());
                        return result;
                    } else {
                        fprintf(stderr, "Out of memory creating image bitmap\r\n");
                    }
                } else {
                    fprintf(stderr, "Unable to get image dimensions: Error %d\r\n", (int)r);
                }
            }
        }
    } else {
        fprintf(stderr, "Not enough arguments\r\n");
    }
    return 1;
}