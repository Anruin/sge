﻿#include <stdio.h>

#include "block.h"
#include "file.h"

void Test_Run() {
    // Test block.
    // FBlock* Block = Block_Create();
    // if (Block == NULL) {
        // printf("Block is nullptr");
    // } else {
        // const U32 Id = Block_GetId(Block);
        // const FByteVector Position = Block_IdToPosition(Id);
        // printf("Block OK: 0x%x 0x%x:0x%x:0x%x\n", Id, Position.X, Position.Y, Position.Z);
    // }

    // Test file read.
    U64 FileLength;
    File_ReadText("./assets/shaders/vs.glsl", &FileLength);
    printf("%llu\n", FileLength);
}
