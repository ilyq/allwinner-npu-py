/*
 * Company:    AW
 * Author:     Penng
 * Date:    2022/09/22
 */

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* model_inputmeta.yml file param modify, eg:

    preproc_node_params:
      add_preproc_node: true
      preproc_type: IMAGE_RGB


demo model: model_rgb_xxx.nb.
*/

void get_input_data(const char *image_file, unsigned char *input_data,
                    int input_h, int input_w) {
  cv::Mat sample = cv::imread(image_file, 1);
  if (sample.empty()) {
    fprintf(stderr, "cv::imread %s failed\n", image_file);
    return;
  }

  cv::Mat img;
  cv::cvtColor(sample, img, cv::COLOR_BGR2RGB);

  if ((img.rows != input_h) || (img.cols != input_w))
    cv::resize(img, img, cv::Size(input_w, input_h));

  unsigned char *img_data = img.data;

  memcpy(input_data, img_data, input_h * input_w * 3);
}

int class_preprocess(const char *imagepath, void *buff_ptr,
                     unsigned int buff_size) {
  // printf("class_preprocess.cpp run. \n");

  int img_c = 3;

  // set default  size
  int input_size = 224; // 224 x 224
  int img_size = input_size * input_size * img_c;

  unsigned int data_size = img_size * sizeof(uint8_t);

  if (data_size > buff_size) {
    printf("data size > buff size, please check code. \n");
    return -1;
  }

  get_input_data(imagepath, (unsigned char *)buff_ptr, input_size, input_size);

  return 0;
}
