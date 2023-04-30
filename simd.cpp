#define PNG_NO_SETJMP

#include <assert.h>
#include <omp.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const int MAX_ITER = 10000;

const int vec_size = 8;

void write_png(const char *filename, const size_t width, const size_t height,
               const int *buffer) {
  FILE *fp = fopen(filename, "wb");
  assert(fp);
  png_structp png_ptr =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  assert(png_ptr);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  assert(info_ptr);
  png_init_io(png_ptr, fp);
  png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png_ptr, info_ptr);
  size_t row_size = 3 * width * sizeof(png_byte);
  png_bytep row = (png_bytep)malloc(row_size);
  for (int y = 0; y < height; ++y) {
    memset(row, 0, row_size);
    for (int x = 0; x < width; ++x) {
      int p = buffer[(height - 1 - y) * width + x];
      png_bytep color = row + x * 3;
      if (p != MAX_ITER) {
        if (p & 16) {
          color[0] = 240;
          color[1] = color[2] = p % 16 * 16;
        } else {
          color[0] = p % 16 * 16;
        }
      }
    }
    png_write_row(png_ptr, row);
  }
  free(row);
  png_write_end(png_ptr, NULL);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose(fp);
}

int main(int argc, char **argv) {
  /* argument parsing */
  assert(argc == 9);
  int num_threads = strtol(argv[1], 0, 10);
  double left = strtod(argv[2], 0);
  double right = strtod(argv[3], 0);
  double lower = strtod(argv[4], 0);
  double upper = strtod(argv[5], 0);
  int width = strtol(argv[6], 0, 10);
  int height = strtol(argv[7], 0, 10);
  const char *filename = argv[8];

  /* allocate memory for image */
  int *image = (int *)malloc(width * height * sizeof(int));
  assert(image);

  double start_time = omp_get_wtime();

  /* mandelbrot set */
  double jj = ((upper - lower) / height);
  double ii = ((right - left) / width);

#pragma omp parallel for collapse(2)
  for (int j = 0; j < height; ++j) {
    for (int i = 0; i < width; i += vec_size) { // tiling to 8.

      // 这里我们每4个为一组进行计算
      alignas(vec_size * sizeof(char)) char finished[vec_size] = {0};

      alignas(vec_size * sizeof(double)) double xx[vec_size] = {0.0};
      alignas(vec_size * sizeof(double)) double yy[vec_size] = {0.0};

      alignas(vec_size * sizeof(double)) double newxx[vec_size];
      alignas(vec_size * sizeof(double)) double newyy[vec_size];

      auto const image_p = image + j * width + i;

      for (size_t repeats = 0; repeats < MAX_ITER; ++repeats) {

#pragma omp simd aligned(finished, xx, yy, newxx, newyy)
        for (size_t v = 0; v < vec_size; ++v) {
          double y0 = j * jj + lower;
          double x0 = (i + v) * ii + left;

          auto const xxxx = xx[v] * xx[v];
          auto const yyyy = yy[v] * yy[v];
          auto const xxyy = xx[v] * yy[v];

          newxx[v] = xxxx - yyyy + x0;
          newyy[v] = 2 * xxyy + y0;

          xx[v] = newxx[v];
          yy[v] = newyy[v];

          auto const dis = xxxx + yyyy;

          if (!finished[v] && __builtin_expect(dis > 4, 0)) {
            finished[v] = 1;
            image_p[v] = repeats;
          }
        }

        size_t all_finished = 1;

        for (size_t v = 0; v < vec_size; ++v) {
          all_finished &= finished[v];
        }

        if (all_finished) {
          break;
        }
      }
    }
  }

  double end_time = omp_get_wtime();

  printf("time elapsed: %lf\n", end_time - start_time);

  /* draw and cleanup */
  write_png(filename, width, height, image);
  free(image);
}
