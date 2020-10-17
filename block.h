#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "malloc.h"
#include "chunk.h"
#include "vector.h"

#pragma pack(push, 1)
typedef struct {
    /** Block position. */
    FByteVector Position;
    /** Chunk that owns the block. */
    FChunk* ParentChunk;
    /** Parent bit, determines direction of the parent block. */
    Byte ParentBit;
    /** Children bits, determines direction of children blocks. */
    Byte ChildBits;
    /** Touching block bits, determine directions occupied by non-hierarchical but adjoined blocks. */
    Byte TouchingBits;
    /** Block type. */
    Byte Type;
    /** Block control flags. */
    Byte Flags;
} FBlock;
#pragma pack(pop)

inline FBlock* Block_Create() {
    FBlock* Block = calloc(1, sizeof *Block);
    return Block;
}

inline void Block_Destroy(FBlock* Block) {
    free(Block);
}

inline U32 Block_GetId(FBlock* Block) {
    U32 Id = 0;
    Id |= Block->Position.X + 0x80;
    Id |= (Block->Position.Y + 0x80) << 0x08;
    Id |= (Block->Position.Y + 0x80) << 0x10;
    return Id;
}

static U32 Block_PositionToId(const FByteVector Position) {
    U32 Id = 0;
    Id |= Position.X + 0x80;
    Id |= (Position.Y + 0x80) << 0x08;
    Id |= (Position.Y + 0x80) << 0x10;
    return Id;
}

static FByteVector Block_IdToPosition(const U32 Id) {
    FByteVector Position;
    Position.X = (Id & 0x000000FF) - 0x80;
    Position.Y = ((Id & 0x0000FF00) >> 0x08) - 0x80;
    Position.Z = ((Id & 0x00FF0000) >> 0x10) - 0x80;
    return Position;
}

inline void Block_SetParentBit(FBlock* Block, const enum EDirection Parent) {
    switch (Parent) {
    case XPositive:
        Block->ParentBit = 0b00000001;
        break;
    case XNegative:
        Block->ParentBit = 0b00000010;
        break;
    case YPositive:
        Block->ParentBit = 0b00000100;
        break;
    case YNegative:
        Block->ParentBit = 0b00001000;
        break;
    case ZPositive:
        Block->ParentBit = 0b00010000;
        break;
    case ZNegative:
        Block->ParentBit = 0b00100000;
        break;
    default:
        Block->ParentBit = 0b00000000;
        break;
    }
}

inline void Block_SwitchChildBit(FBlock* Block, const EDirection Child) {
    switch (Child) {
    case XPositive:
        Block->ChildBits |= 0b00000001;
        break;
    case XNegative:
        Block->ChildBits |= 0b00000010;
        break;
    case YPositive:
        Block->ChildBits |= 0b00000100;
        break;
    case YNegative:
        Block->ChildBits |= 0b00001000;
        break;
    case ZPositive:
        Block->ChildBits |= 0b00010000;
        break;
    case ZNegative:
        Block->ChildBits |= 0b00100000;
        break;
    default:
        break;
    }
}

inline void Block_SwitchTouchingBit(FBlock* Block, const EDirection Child) {
    switch (Child) {
    case XPositive:
        Block->TouchingBits |= 0b00000001;
        break;
    case XNegative:
        Block->TouchingBits |= 0b00000010;
        break;
    case YPositive:
        Block->TouchingBits |= 0b00000100;
        break;
    case YNegative:
        Block->TouchingBits |= 0b00001000;
        break;
    case ZPositive:
        Block->TouchingBits |= 0b00010000;
        break;
    case ZNegative:
        Block->TouchingBits |= 0b00100000;
        break;
    default:
        break;
    }
}

#ifdef __cplusplus
}
#endif
