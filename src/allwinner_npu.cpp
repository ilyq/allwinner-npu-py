/*
 * Company:    AW
 * Author:     Penng
 * Date:    2023/01/16
 */
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "npulib.h"

namespace py = pybind11;
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

  py::array_t<float> infer(const std::string &input_file) {
    int status = 0;
    void *input_buffer_ptr = nullptr;
    unsigned int input_buffer_size = 0;
    mbv2.get_network_input_buff_info(0, &input_buffer_ptr, &input_buffer_size);
    class_preprocess(input_file.c_str(), input_buffer_ptr, input_buffer_size);
    int output_cnt = mbv2.get_output_cnt();
    int size0 = 1280; // 特征层维度
    float *output = new float[size0];
    float **output_data = new float *[output_cnt]();
    output_data[0] = output;

    status = mbv2.network_input_output_set();
    if (status != 0)
      throw std::runtime_error("network_input_output_set failed");
    status = mbv2.network_run();
    if (status != 0)
      throw std::runtime_error("network_run failed");
    mbv2.get_output(output_data);

    for (int i = 0; i < 10; ++i) {
      printf("output[%d]=%f\n", i, output[i]);
    }

    py::array_t<float> result(size0, output);
    delete[] output;
    delete[] output_data;
    return result;
  }

private:
  NpuUint npu_uint;
  NetworkItem mbv2;
  unsigned int network_id;
};

PYBIND11_MODULE(allwinner_npu, m) {
  py::class_<AllwinnerNPU>(m, "AllwinnerNPU")
      .def(py::init<const std::string &, unsigned int>())
      .def("infer", &AllwinnerNPU::infer);
}
