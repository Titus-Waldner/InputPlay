#pragma once

struct RecordingOptions
{
    int startKey = 0;
    int pauseKey = 0;
    int stopKey = 0;

    bool waitForStartKey = true;

    bool captureMouse = true;
    bool captureKeyboard = true;
};