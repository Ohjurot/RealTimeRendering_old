#pragma once

#include <WinInclude.h>

namespace RTR
{
    // Memory expresions
    inline constexpr UINT64 MemByte(UINT64 byte)
    {
        return byte;
    }
    inline constexpr UINT64 MemKiB(UINT64 kib)
    {
        return kib * 1024ULL;
    }
    inline constexpr UINT64 MemMiB(UINT64 mib)
    {
        return MemKiB(mib) * 1024ULL;
    }
    inline constexpr UINT64 MemGiB(UINT64 gib)
    {
        return MemMiB(gib) * 1024ULL;
    }
    inline constexpr UINT64 MemTiB(UINT64 tib)
    {
        return MemGiB(tib) * 1024ULL;
    }
}
