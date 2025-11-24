import cv2
import numpy as np
import allwinner_npu

npu = allwinner_npu.AllWinnnerNPU(
    "/data/data/com.termux/files/home/allwinner/mobilenetv2_pcq_a733.nb"
)

# 2. 加载并预处理图片
img = cv2.imread("1.jpg")
img = cv2.resize(img, (224, 224))  # mobilenet 通常输入 224x224
img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
img = img.astype(np.float32) / 255.0
img = np.transpose(img, (2, 0, 1))  # HWC -> CHW
img = np.expand_dims(img, axis=0)  # 增加 batch 维度

# 3. 推理
result = npu.infer(img)

# 4. 处理输出
print("推理结果:", result)
