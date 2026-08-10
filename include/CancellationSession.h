#pragma once

#include <string>

class CancellationSession
{
public:
    CancellationSession() = default;
    ~CancellationSession();

    CancellationSession(
        const CancellationSession&) = delete;

    CancellationSession& operator=(
        const CancellationSession&) = delete;

    bool create(
        const std::string& sessionName,
        std::string& errorMessage);

    [[nodiscard]]
    bool isCancellationRequested() const;

    bool consumePauseRequest();
    bool consumeResumeRequest();

    static bool requestCancellation(
        const std::string& sessionName,
        std::string& errorMessage);

    static bool requestPause(
        const std::string& sessionName,
        std::string& errorMessage);

    static bool requestResume(
        const std::string& sessionName,
        std::string& errorMessage);

    static bool isValidSessionName(
        const std::string& sessionName);

private:
    void closeHandles();

    void* cancellationEventHandle_ = nullptr;
    void* pauseEventHandle_ = nullptr;
    void* resumeEventHandle_ = nullptr;
};