/*
 * Company:    AW
 * Author:     Penng
 * Date:    2023/01/16
 */

#include <pybind11/pybind11.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "npulib.h"

/*-------------------------------------------
        Macros and Variables
-------------------------------------------*/

extern int class_preprocess(const char *imagepath, void *buff_ptr,
                            unsigned int buff_size);
extern int class_postprocess(const char *imagepath, float **output);

const char *usage =
    "model_demo -nb modle_path -i input_path -l loop_run_count -m malloc_mbyte "
    "\n"
    "-nb modle_path:    the NBG file path.\n"
    "-i input_path:     the input file path.\n"
    "-l loop_run_count: the number of loop run network.\n"
    "-m malloc_mbyte:   npu_unit init memory Mbytes.\n"
    "-h : help\n"
    "example: model_demo -nb model.nb -i input.tensor -l 10 -m 20 \n";

enum time_idx_e {
  NPU_INIT = 0,
  NETWORK_CREATE,
  NETWORK_PREPARE,
  NETWORK_RUN,
  NETWORK_LOOP,
  TIME_IDX_MAX = 9
};

#if defined(__linux__)
#define TIME_SLOTS 10
static uint64_t time_begin[TIME_SLOTS];
static uint64_t time_end[TIME_SLOTS];
static uint64_t GetTime(void) {
  struct timeval time;
  gettimeofday(&time, NULL);
  return (uint64_t)(time.tv_usec + time.tv_sec * 1000000);
}

static void TimeBegin(int id) { time_begin[id] = GetTime(); }

static void TimeEnd(int id) { time_end[id] = GetTime(); }

static uint64_t TimeGet(int id) { return time_end[id] - time_begin[id]; }
#endif

void run_npu_demo() {
  int status = 0;
  int i = 0, k = 0, j = 0;
  unsigned int count = 0;
  long long total_infer_time = 0;

  char *model_file = (char *)"mobilenetv2_pcq_a733.nb";
  char *input_file = (char *)"1.jpg";
  unsigned int loop_count = 10;
  unsigned int malloc_mbyte = 20;

  printf("model_file=%s, input=%s, loop_count=%d, malloc_mbyte=%d \n",
         model_file, input_file, loop_count, malloc_mbyte);

  NpuUint npu_uint;
  int ret = npu_uint.npu_init(malloc_mbyte * 1024 * 1024);
  if (ret != 0) {
    throw std::runtime_error("npu_init failed");
  }

  NetworkItem mbv2;
  unsigned int network_id = 0;
  status = mbv2.network_create(model_file, network_id);
  if (status != 0) {
    printf("network %d create failed.\n", network_id);
  }

  status = mbv2.network_prepare();
  if (status != 0) {
    printf("network prepare fail, status=%d\n", status);
  }

  void *input_buffer_ptr = nullptr;
  unsigned int input_buffer_size = 0;
  mbv2.get_network_input_buff_info(0, &input_buffer_ptr, &input_buffer_size);
  class_preprocess(input_file, input_buffer_ptr, input_buffer_size);

  int output_cnt = mbv2.get_output_cnt();
  float **output_data = new float *[output_cnt]();
  int size0 = 1 * 1000;
  output_data[0] = new float[size0];

  i = network_id;
  TimeBegin(NETWORK_LOOP);
  while (count < loop_count) {
    count++;
    printf("network: %d, loop count: %d\n", i, count);
    status = mbv2.network_input_output_set();
    if (status != 0) {
      printf("set network input/output %d failed.\n", i);
      throw std::runtime_error("network_input_output_set failed");
    }
#if defined(__linux__)
    TimeBegin(NETWORK_RUN);
#endif
    status = mbv2.network_run();
    if (status != 0) {
      printf("fail to run network, status=%d, batchCount=%d\n", status, i);
      throw std::runtime_error("network_run failed");
    }
#if defined(__linux__)
    TimeEnd(NETWORK_RUN);
    printf("run time for this network %d: %lu us.\n", i,
           (unsigned long)TimeGet(NETWORK_RUN));
#endif
    total_infer_time += (unsigned long)TimeGet(NETWORK_RUN);
    mbv2.get_output(output_data);
    class_postprocess(input_file, output_data);
  }
  TimeEnd(NETWORK_LOOP);
  if (loop_count > 1) {
    printf("network: %d, this network run avg inference time=%d us,  total avg "
           "cost: %d us\n",
           i, (uint32_t)(total_infer_time / loop_count),
           (unsigned int)(TimeGet(NETWORK_LOOP) / loop_count));
  }
  for (int i = 0; i < output_cnt; i++) {
    delete[] output_data[i];
    output_data[i] = nullptr;
  }
  if (output_data != nullptr)
    delete[] output_data;
}

namespace py = pybind11;
PYBIND11_MODULE(allwinner_npu, m) { m.def("run_npu_demo", &run_npu_demo); }
