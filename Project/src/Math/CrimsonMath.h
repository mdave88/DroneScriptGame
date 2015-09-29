#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include "vec3.h"
#include "matrix.h"
#include "quaternion.h"

float normalizeAngle(float angle);
vec3 normalizeOrientationVector(const vec3& v);
