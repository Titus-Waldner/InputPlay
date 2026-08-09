#include "CancellationSession.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cctype>
#include <string>

namespace
{
constexpr const char* SessionPrefix =
    "Local\\InputPlay.Cancel.";

std::string createEventName(
    const std::string& sessionName)
{
    return
        std::string(SessionPrefix)
        + sessionName;
}
}

CancellationSession::~CancellationSession()
{
    if (eventHandle_ != nullptr)
    {
        CloseHandle(
            static_cast<HANDLE>(eventHandle_));

        eventHandle_ = nullptr;
    }
}

bool CancellationSession::isValidSessionName(
    const std::string& sessionName)
{
    if (sessionName.empty()
        || sessionName.length() > 64)
    {
        return false;
    }

    for (const unsigned char character : sessionName)
    {
        if (!std::isalnum(character)
            && character != '-'
            && character != '_')
        {
            return false;
        }
    }

    return true;
}

bool CancellationSession::create(
    const std::string& sessionName,
    std::string& errorMessage)
{
    if (!isValidSessionName(sessionName))
    {
        errorMessage =
            "Session names may contain only letters, numbers, "
            "hyphens, and underscores, with a maximum length "
            "of 64 characters.";

        return false;
    }

    const std::string eventName =
        createEventName(sessionName);

    HANDLE eventHandle = CreateEventA(
        nullptr,
        TRUE,
        FALSE,
        eventName.c_str());

    if (eventHandle == nullptr)
    {
        errorMessage =
            "Windows was unable to create the cancellation session.";

        return false;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(eventHandle);

        errorMessage =
            "A playback session with this name already exists.";

        return false;
    }

    eventHandle_ = eventHandle;

    errorMessage.clear();
    return true;
}

bool CancellationSession::isCancellationRequested() const
{
    if (eventHandle_ == nullptr)
    {
        return false;
    }

    const DWORD waitResult = WaitForSingleObject(
        static_cast<HANDLE>(eventHandle_),
        0);

    return waitResult == WAIT_OBJECT_0;
}

bool CancellationSession::requestCancellation(
    const std::string& sessionName,
    std::string& errorMessage)
{
    if (!isValidSessionName(sessionName))
    {
        errorMessage =
            "Session names may contain only letters, numbers, "
            "hyphens, and underscores, with a maximum length "
            "of 64 characters.";

        return false;
    }

    const std::string eventName =
        createEventName(sessionName);

    HANDLE eventHandle = OpenEventA(
        EVENT_MODIFY_STATE,
        FALSE,
        eventName.c_str());

    if (eventHandle == nullptr)
    {
        errorMessage =
            "No active playback session was found with that name.";

        return false;
    }

    const BOOL signalResult =
        SetEvent(eventHandle);

    CloseHandle(eventHandle);

    if (!signalResult)
    {
        errorMessage =
            "Windows was unable to signal the playback session.";

        return false;
    }

    errorMessage.clear();
    return true;
}