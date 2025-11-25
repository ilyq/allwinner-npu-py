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

class AllwinnerNPU {
public:
  AllwinnerNPU(const std::string &model_file, unsigned int malloc_mbyte) {
    int ret = npu_uint.npu_init(malloc_mbyte * 1024 * 1024);
    if (ret != 0) {
      throw std::runtime_error("npu_init failed");
    }
    network_id = 0;
    int status =
        mbv2.network_create(const_cast<char *>(model_file.c_str()), network_id);
    if (status != 0) {
      throw std::runtime_error("network_create failed");
    }
    status = mbv2.network_prepare();
    if (status != 0) {
      throw std::runtime_error("network_prepare failed");
    }
  }

  void infer(const std::string &input_file, unsigned int loop_count) {
    int status = 0;
    unsigned int count = 0;
    long long total_infer_time = 0;
    void *input_buffer_ptr = nullptr;
    unsigned int input_buffer_size = 0;
    mbv2.get_network_input_buff_info(0, &input_buffer_ptr, &input_buffer_size);
    class_preprocess(input_file.c_str(), input_buffer_ptr, input_buffer_size);
    int output_cnt = mbv2.get_output_cnt();
    float **output_data = new float *[output_cnt]();
    int size0 = 1 * 1000;
    output_data[0] = new float[size0];
    TimeBegin(NETWORK_LOOP);
    while (count < loop_count) {
      count++;
      status = mbv2.network_input_output_set();
      if (status != 0) {
        throw std::runtime_error("network_input_output_set failed");
      }
#if defined(__linux__)
      TimeBegin(NETWORK_RUN);
#endif
      status = mbv2.network_run();
      if (status != 0) {
        throw std::runtime_error("network_run failed");
      }
#if defined(__linux__)
      TimeEnd(NETWORK_RUN);
      printf("run time for this network: %lu us.\n",
             (unsigned long)TimeGet(NETWORK_RUN));
#endif
      total_infer_time += (unsigned long)TimeGet(NETWORK_RUN);
      mbv2.get_output(output_data);
      class_postprocess(input_file.c_str(), output_data);
    }
    TimeEnd(NETWORK_LOOP);
    if (loop_count > 1) {
      printf(
          "this network run avg inference time=%d us,  total avg cost: %d us\n",
          (uint32_t)(total_infer_time / loop_count),
          (unsigned int)(TimeGet(NETWORK_LOOP) / loop_count));
    }
    for (int i = 0; i < output_cnt; i++) {
      delete[] output_data[i];
      output_data[i] = nullptr;
    }
    if (output_data != nullptr)
      delete[] output_data;
  }

private:
  NpuUint npu_uint;
  NetworkItem mbv2;
  unsigned int network_id;
};

namespace py = pybind11;
PYBIND11_MODULE(allwinner_npu, m) {
  py::class_<AllwinnerNPU>(m, "AllwinnerNPU")
      .def(py::init<const std::string &, unsigned int>())
      .def("infer", &AllwinnerNPU::infer);
}
