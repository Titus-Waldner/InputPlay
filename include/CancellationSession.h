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

    [[nodiscard]] bool isCancellationRequested() const;

    static bool requestCancellation(
        const std::string& sessionName,
        std::string& errorMessage);

    static bool isValidSessionName(
        const std::string& sessionName);

private:
    void* eventHandle_ = nullptr;
};