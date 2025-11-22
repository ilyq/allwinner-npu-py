#include "npulib.h"
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <string>
#include <vector>

namespace py = pybind11;

class AllWinnnerNPU {
public:
  AllWinnnerNPU(const std::string &model_file, unsigned int malloc_mbyte = 10) {
    /* NPU初始化 */
    npu_uint = new NpuUint();
    int ret = npu_uint->npu_init(malloc_mbyte * 1024 * 1024);
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

    network->get_network_input_buff_info(0, &input_buffer_ptr,
                                         &input_buffer_size);

    output_cnt = network->get_output_cnt();
    output_data.resize(output_cnt);
    int size0 = 1000;
    output_data[0].resize(size0);
    output_ptrs.resize(output_cnt);
    output_ptrs[0] = output_data[0].data();
  }

  ~AllWinnnerNPU() {
    if (network)
      delete network;

    if (npu_uint)
      delete npu_uint;
  }

  py::array_t<float> infer(py::array_t<float> input) {
    py::buffer_info buf = input.request();
    if ((unsigned int)buf.size * sizeof(float) != input_buffer_size) {
      throw std::runtime_error("Input size  mismatch");
    }

    memcpy(input_buffer_ptr, buf.ptr, input_buffer_size);

    int status = network->network_input_output_set();
    if (status != 0)
      throw std::runtime_error(
          "Set network input/outout failed with error code: " +
          std::to_string(status));

    status = network->network_run();
    if (status != 0)
      throw std::runtime_error("Network run failed with error code: " +
                               std::to_string(status));

    network->get_output(output_ptrs.data());

    return py::array_t<float>(output_data[0].size(), output_data[0].data());
  }

private:
  NpuUint *npu_uint = nullptr;
  NetworkItem *network = nullptr;
  void *input_buffer_ptr = nullptr;
  unsigned int input_buffer_size = 0;
  int output_cnt = 0;
  std::vector<std::vector<float>> output_data;
  std::vector<float *> output_ptrs;
};

PYBIND11_MODULE(allwinner_npu, m) {
  m.doc() = "pybind11 绑定C++数学工具库示例（无独立头文件）";

  py::class_<AllWinnnerNPU>(m, "AllWinnnerNPU")
      .def(py::init<const std::string &, unsigned int>(), py::arg("model_file"),
           py::arg("malloc_mbyte") = 10)
      .def("infer", &AllWinnnerNPU::infer, py::arg("intput"));

  m.def(
      "greet", []() { return "Hello from C++!"; }, "返回问候语");
}