#pragma once

#include "Recording.h"

#include <QWidget>

class QLabel;
class QVBoxLayout;

class MacroInfoPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MacroInfoPanel(QWidget* parent = nullptr);
    
    void setRecording(Recording* recording, const QString& filePath);
    void refresh();

private:
    void setupUi();
    void updateDisplay();
    void updateDisplayCompatibility();
    
    Recording* recording_ = nullptr;
    QString filePath_;
    
    // File info
    QLabel* fileNameLabel_ = nullptr;
    QLabel* filePathLabel_ = nullptr;
    QLabel* fileSizeLabel_ = nullptr;
    
    // Recording stats
    QLabel* eventCountLabel_ = nullptr;
    QLabel* durationLabel_ = nullptr;
    QLabel* startPosLabel_ = nullptr;
    
    // Event breakdown
    QLabel* mouseMoveCountLabel_ = nullptr;
    QLabel* mouseClickCountLabel_ = nullptr;
    QLabel* mouseWheelCountLabel_ = nullptr;
    QLabel* keyCountLabel_ = nullptr;
    QLabel* waitCountLabel_ = nullptr;
    QLabel* teleportCountLabel_ = nullptr;
    
    // Display info
    QLabel* displayStatusLabel_ = nullptr;
    QLabel* monitorCountLabel_ = nullptr;
    QLabel* virtualDesktopLabel_ = nullptr;
    
};
