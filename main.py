import sys

sys.path.insert(0, "./build")

import allwinner_npu

npu = allwinner_npu.NPU()

print(npu)


def main():
    print("Hello from allwinner-npu-py!")


if __name__ == "__main__":
    main()
