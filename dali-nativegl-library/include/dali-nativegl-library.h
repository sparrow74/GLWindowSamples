/*
 * Copyright (c) 2011-2017 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __DALI_NATIVEGL_LIBRARY_UTIL_H__
#define __DALI_NATIVEGL_LIBRARY_UTIL_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file dali-nativegl-library.h
 */

/**
 * @addtogroup DALI_NATIVEGL_LIBARRY
 * @{
 */


typedef struct {
    float x, y;
} FloatPoint;

/* Application data */
typedef struct GLDATA {
    float model[16];
    float view[16];
    float mvp[16];

    FloatPoint anglePoint;
    FloatPoint curPoint;
    FloatPoint prevPoint;

    GLuint       vtx_shader;
    GLuint       fgmt_shader;
    /*A program object is an object to which shader objects can be attached*/
    unsigned int program;

    /* Generate Vertex Buffer */
    unsigned int vbo;

    int width;
    int height;

    bool mouse_down;

    int windowAngle;
} GLData;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* __DALI_NATIVEGL_LIBRARY_UTIL_H__ */
