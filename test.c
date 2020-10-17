#include <stdio.h>

#include "block.h"
#include "file.h"
#include "graphics.h"

void Test_Run() {
    FBlock* Block = Block_Create();
    if (Block == NULL) {
        printf("Block is nullptr");
    } else {
        const U32 Id = Block_GetId(Block);
        printf("0x%x\n", Id);
        const FByteVector Position = Block_IdToPosition(Id);
        printf("0x%x:0x%x:0x%x\n", Position.X, Position.Y, Position.Z);
    }

    RenderService* RenderService = RenderService_Get();
    if (RenderService == NULL) {
        printf("Render service is nullptr\n");
    } else {
        printf("0x%p\n", RenderService);
    }

    RenderService_Initialize();

    const pStr FileContents = File_Read("./text.txt", NULL);
    printf("%s", FileContents);
}
