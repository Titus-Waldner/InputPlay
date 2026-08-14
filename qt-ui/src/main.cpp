#include "MainWindow.h"
#include "DarkStyle.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QSettings>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellscalingapi.h>
#endif

int main(int argc, char* argv[])
{
#ifdef _WIN32
    // Match the DPI awareness of the CLI application (system-aware)
    // This ensures GetSystemMetrics returns consistent values for display compatibility checks
    SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
#endif

    QApplication app(argc, argv);
    
    app.setApplicationName("InputPlay Studio");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("InputPlay");
    
    // Set default font
    QFont defaultFont("Segoe UI", 10);
    app.setFont(defaultFont);
    
    // Load saved theme settings
    QSettings settings("InputPlay", "Studio");
    ThemeType theme = static_cast<ThemeType>(settings.value("theme", static_cast<int>(ThemeType::DarkBlue)).toInt());
    AccentColor accent = static_cast<AccentColor>(settings.value("accentColor", static_cast<int>(AccentColor::Cyan)).toInt());
    
    // Apply saved style
    DarkStyle::apply(&app, theme, accent);
    
    MainWindow window;
    window.show();
    
    // Handle command line file argument
    if (argc > 1) {
        QString filePath = QString::fromLocal8Bit(argv[1]);
        if (filePath.endsWith(".irec", Qt::CaseInsensitive)) {
            window.loadMacro(filePath);
        }
    }
    
    return app.exec();
}
