/****************************************************************************
*  model config header file
****************************************************************************/
#ifndef _MODEL_CONFIG_H_
#define _MODEL_CONFIG_H_

#include <iostream>
#include <vector>


#define COCO    1
//#define COCO    0

#if COCO
// coco, 80 class
#define CLASS_NUM           1

/* 640 x 640 */
#define LETTERBOX_ROWS      640
#define LETTERBOX_COLS      640

/* 640 x 384 */
//#define LETTERBOX_ROWS      384
//#define LETTERBOX_COLS      640

#define SCORE_THRESHOLD     0.4f
#define NMS_THRESHOLD       0.45f


const std::vector<std::string> g_classes_name{
    "bread"
};

#elif 1
// customer, 4 class
#define CLASS_NUM           1

#define LETTERBOX_ROWS      512
#define LETTERBOX_COLS      512

#define SCORE_THRESHOLD     0.4f
#define NMS_THRESHOLD       0.45f

const std::vector<std::string> g_classes_name{
    "bread"
};

#else
// eg: plant, 1 class
#define CLASS_NUM           1

#define LETTERBOX_ROWS      640
#define LETTERBOX_COLS      640

#define SCORE_THRESHOLD     0.4f
#define NMS_THRESHOLD       0.45f

const std::vector<std::string> g_classes_name{
    "bread"
};

#endif

#endif