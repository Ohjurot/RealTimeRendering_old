#include "DirWatcher.h"

HANDLE __global__rtr__dirwatcher_hNotifcation = INVALID_HANDLE_VALUE;
UINT64 __global__rtr__dirwatcher_dirIteration = 0;

void RTR::DirWatchInit()
{
    // Get working dir
    wchar_t workDir[MAX_PATH];
    if (!GetCurrentDirectory(MAX_PATH, workDir))
    {
        throw std::exception("Failed to retrive working directory. Is the path too long?");
    }

    // Sign up for change notifications
    __global__rtr__dirwatcher_hNotifcation = FindFirstChangeNotification(workDir, TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE);
    if (__global__rtr__dirwatcher_hNotifcation == INVALID_HANDLE_VALUE)
    {
        throw std::exception("Failed to singup for file change notifications!");
    }
}

void RTR::DirWatchShutdown()
{
    // Close handle
    FindCloseChangeNotification(__global__rtr__dirwatcher_hNotifcation);
}

void RTR::DirWatchRefresh()
{
    // Check if handle was triggered
    switch (WaitForSingleObject(__global__rtr__dirwatcher_hNotifcation, 0))
    {
        // File change occured
        case WAIT_OBJECT_0:
            // Increment global revision
            __global__rtr__dirwatcher_dirIteration++;
            // Singup for further notifications
            if (!FindNextChangeNotification(__global__rtr__dirwatcher_hNotifcation))
            {
                throw std::exception("Failed to singup for further file change notifications!");
            }
            break;

        // No change occured (do nothing)
        case WAIT_TIMEOUT:
            break;

        // Error condition
        default:
            throw std::exception("Unexpected return value while trying to get file notification!");
    }
}

UINT64 RTR::DirWatchGetRevision()
{
    return __global__rtr__dirwatcher_dirIteration;
}
