#include <pybind11/pybind11.h>

class NPU {
public:
  NPU() {}
  ~NPU() {}
};

PYBIND11_MODULE(allwinner_npu, m) {
  m.doc() = "pybind11 example plugin"; // optional module docstring

  pybind11::class_<NPU>(m, "NPU")
      .def(pybind11::init<>())
      .def("__str__", [](const NPU &a) { return "<NPU>"; });
}
