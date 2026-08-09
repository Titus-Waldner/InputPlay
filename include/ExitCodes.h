#pragma once

namespace ExitCode
{
constexpr int Success = 0;
constexpr int GeneralFailure = 1;
constexpr int InvalidArguments = 2;
constexpr int RecordingLoadFailure = 3;
constexpr int Cancelled = 4;
constexpr int Timeout = 5;
constexpr int DisplayIncompatible = 6;
constexpr int ValidationFailure = 7;
}