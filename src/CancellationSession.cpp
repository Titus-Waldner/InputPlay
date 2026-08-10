#include "CancellationSession.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cctype>
#include <string>

namespace
{
constexpr const char* CancelPrefix =
    "Local\\InputPlay.Cancel.";

constexpr const char* PausePrefix =
    "Local\\InputPlay.Pause.";

constexpr const char* ResumePrefix =
    "Local\\InputPlay.Resume.";

std::string createEventName(
    const char* prefix,
    const std::string& sessionName)
{
    return
        std::string(prefix)
        + sessionName;
}

bool signalSessionEvent(
    const char* prefix,
    const std::string& sessionName,
    const std::string& missingSessionMessage,
    std::string& errorMessage)
{
    if (!CancellationSession::isValidSessionName(
            sessionName))
    {
        errorMessage =
            "Session names may contain only letters, numbers, "
            "hyphens, and underscores, with a maximum length "
            "of 64 characters.";

        return false;
    }

    const std::string eventName =
        createEventName(
            prefix,
            sessionName);

    HANDLE eventHandle = OpenEventA(
        EVENT_MODIFY_STATE,
        FALSE,
        eventName.c_str());

    if (eventHandle == nullptr)
    {
        errorMessage =
            missingSessionMessage;

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
}

CancellationSession::~CancellationSession()
{
    closeHandles();
}

void CancellationSession::closeHandles()
{
    if (cancellationEventHandle_ != nullptr)
    {
        CloseHandle(
            static_cast<HANDLE>(
                cancellationEventHandle_));

        cancellationEventHandle_ = nullptr;
    }

    if (pauseEventHandle_ != nullptr)
    {
        CloseHandle(
            static_cast<HANDLE>(
                pauseEventHandle_));

        pauseEventHandle_ = nullptr;
    }

    if (resumeEventHandle_ != nullptr)
    {
        CloseHandle(
            static_cast<HANDLE>(
                resumeEventHandle_));

        resumeEventHandle_ = nullptr;
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

    for (const unsigned char character
         : sessionName)
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
    closeHandles();

    if (!isValidSessionName(sessionName))
    {
        errorMessage =
            "Session names may contain only letters, numbers, "
            "hyphens, and underscores, with a maximum length "
            "of 64 characters.";

        return false;
    }

    const std::string cancellationEventName =
        createEventName(
            CancelPrefix,
            sessionName);

    HANDLE cancellationEvent = CreateEventA(
        nullptr,
        TRUE,
        FALSE,
        cancellationEventName.c_str());

    if (cancellationEvent == nullptr)
    {
        errorMessage =
            "Windows was unable to create the "
            "cancellation session event.";

        return false;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(cancellationEvent);

        errorMessage =
            "A playback session with this name already exists.";

        return false;
    }

    cancellationEventHandle_ =
        cancellationEvent;

    const std::string pauseEventName =
        createEventName(
            PausePrefix,
            sessionName);

    HANDLE pauseEvent = CreateEventA(
        nullptr,
        FALSE,
        FALSE,
        pauseEventName.c_str());

    if (pauseEvent == nullptr)
    {
        closeHandles();

        errorMessage =
            "Windows was unable to create the "
            "pause session event.";

        return false;
    }

    pauseEventHandle_ =
        pauseEvent;

    const std::string resumeEventName =
        createEventName(
            ResumePrefix,
            sessionName);

    HANDLE resumeEvent = CreateEventA(
        nullptr,
        FALSE,
        FALSE,
        resumeEventName.c_str());

    if (resumeEvent == nullptr)
    {
        closeHandles();

        errorMessage =
            "Windows was unable to create the "
            "resume session event.";

        return false;
    }

    resumeEventHandle_ =
        resumeEvent;

    errorMessage.clear();
    return true;
}

bool CancellationSession::isCancellationRequested() const
{
    if (cancellationEventHandle_ == nullptr)
    {
        return false;
    }

    const DWORD waitResult =
        WaitForSingleObject(
            static_cast<HANDLE>(
                cancellationEventHandle_),
            0);

    return waitResult == WAIT_OBJECT_0;
}

bool CancellationSession::consumePauseRequest()
{
    if (pauseEventHandle_ == nullptr)
    {
        return false;
    }

    const DWORD waitResult =
        WaitForSingleObject(
            static_cast<HANDLE>(
                pauseEventHandle_),
            0);

    return waitResult == WAIT_OBJECT_0;
}

bool CancellationSession::consumeResumeRequest()
{
    if (resumeEventHandle_ == nullptr)
    {
        return false;
    }

    const DWORD waitResult =
        WaitForSingleObject(
            static_cast<HANDLE>(
                resumeEventHandle_),
            0);

    return waitResult == WAIT_OBJECT_0;
}

bool CancellationSession::requestCancellation(
    const std::string& sessionName,
    std::string& errorMessage)
{
    return signalSessionEvent(
        CancelPrefix,
        sessionName,
        "No active playback session was found with that name.",
        errorMessage);
}

bool CancellationSession::requestPause(
    const std::string& sessionName,
    std::string& errorMessage)
{
    return signalSessionEvent(
        PausePrefix,
        sessionName,
        "No active playback session was found with that name.",
        errorMessage);
}

bool CancellationSession::requestResume(
    const std::string& sessionName,
    std::string& errorMessage)
{
    return signalSessionEvent(
        ResumePrefix,
        sessionName,
        "No active playback session was found with that name.",
        errorMessage);
}