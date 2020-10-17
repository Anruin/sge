#pragma once
#include "typedefs.h"

typedef struct {
    F32 X;
    F32 Y;
    F32 Z;
} FVector;

#pragma pack (push, 1)
typedef struct {
    U8 X;
    U8 Y;
    U8 Z;
} FByteVector;
#pragma pack (pop)
