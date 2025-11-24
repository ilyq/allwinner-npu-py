#include "npulib.h"
#include "pybind11/cast.h"
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <string>
#include <vector>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace py = pybind11;

class AllWinnnerNPU {
public:
  AllWinnnerNPU(const std::string &model_file, unsigned int malloc_mbyte = 10) {
    /* NPU初始化 */
    cv::Mat image;
    cv::Size size(100, 100);  // 这里应该能正常提示和跳转
    npu_uint = new NpuUint();
    int ret = npu_uint->npu_init();
    if (ret != 0)
      throw std::runtime_error("NPU init failed with error code: " +
                               std::to_string(ret));

    network = new NetworkItem();
    unsigned int network_id = 0;
    int status = network->network_create(const_cast<char *>(model_file.c_str()),
                                         network_id);
    if (status != 0)
      throw std::runtime_error("Network create failed with error code: " +
                               std::to_string(status));

    status = network->network_prepare();
    if (status != 0)
      throw std::runtime_error("Network prepare failed with error code: " +
                               std::to_string(status));

    network->get_network_input_buff_info(0, &input_ptr, &input_size);

    output_cnt = network->get_output_cnt();
    output_data.resize(output_cnt);
    output_ptrs.resize(output_cnt);

    for (int i = 0; i < output_cnt; i++) {
      output_ptrs[i] = nullptr; // FP_NO_COPY
    }
  }

  ~AllWinnnerNPU() {
    if (network)
      delete network;

    if (npu_uint)
      delete npu_uint;
  }

  py::list infer(py::array_t<float> input) {
    py::buffer_info buf = input.request();

    // if ((unsigned int)buf.size * sizeof(float) != input_size)
    //     throw std::runtime_error("Input tensor size mismatch.");

    memcpy(input_ptr, buf.ptr, input_size);

    int status = network->network_input_output_set();
    if (status != 0)
      throw std::runtime_error(
          "Set network input/outout failed with error code: " +
          std::to_string(status));

    status = network->network_run();
    if (status != 0)
      throw std::runtime_error("Network run failed with error code: " +
                               std::to_string(status));


    // 网络推理
    status = network->network_run();
    if (status != 0)
      throw std::runtime_error("Network run failed with error code: " +
                               std::to_string(status));

    // FP_NO_COPY 输出
    std::vector<output_info_s> outputs_info(output_cnt);
    network->get_output_fp_nocopy(outputs_info.data());

    py::list py_outputs;
    for (int i = 0; i < output_cnt; i++) {
        float* ptr = outputs_info[i].ptr;
        int len = outputs_info[i].length / sizeof(float);

        // debug
        float min_val = *std::min_element(ptr, ptr + len);
        float max_val = *std::max_element(ptr, ptr + len);
        float mean_val = std::accumulate(ptr, ptr + len, 0.0f) / len;
        std::cout << "[Debug] output[" << i << "] len=" << len
                  << " min=" << min_val << " max=" << max_val
                  << " mean=" << mean_val << std::endl;

        py_outputs.append(py::array_t<float>(len, ptr));
    }
    return py_outputs;
  }

private:
  NpuUint *npu_uint = nullptr;
  NetworkItem *network = nullptr;
  void *input_ptr = nullptr;
  unsigned int input_size = 0;
  int output_cnt = 0;
  std::vector<std::vector<float>> output_data;
  std::vector<float *> output_ptrs;
};

PYBIND11_MODULE(allwinner_npu, m) {
  m.doc() = "Allwinner NPU Inference";

  py::class_<AllWinnnerNPU>(m, "AllWinnnerNPU")
      .def(py::init<const std::string &, unsigned int>(),
           py::arg("model_file"),
           py::arg("malloc_mbyte") = 10)
      .def("infer", &AllWinnnerNPU::infer, py::arg("intput"));
}