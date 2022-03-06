#pragma once

#include <WinInclude.h>
#include <Util/HrException.h>

namespace RTR
{
    // Called at the beign of the application
    void DirWatchInit();
    // Called at the end of the application
    void DirWatchShutdown();

    // Called every frame to check for changed objects
    void DirWatchRefresh();

    // Get directory revison (increments each time a file changes)
    UINT64 DirWatchGetRevision();
}
