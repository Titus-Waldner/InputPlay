#include "Recording.h"
#include "RecordingFile.h"
#include "DryRunBackend.h"
#include "SendInputBackend.h"
#include "InputRecorder.h"
#include "Settings.h"
#include "RecordingValidator.h"
#include "ExitCodes.h"
#include "CancellationSession.h"
#include "PlaybackOptions.h"
#include "PlaybackEngine.h"
#include "CommandLine.h"

#include <limits>
#include <iomanip>
#include <iostream>
#include <string>
#include <stdexcept>

namespace
{
	
void printExitCodes()
{
    std::cout
        << "InputPlay exit codes\n\n";

    std::cout
        << ExitCode::Success
        << "  Success\n";

    std::cout
        << ExitCode::GeneralFailure
        << "  General failure\n";

    std::cout
        << ExitCode::InvalidArguments
        << "  Invalid command-line arguments\n";

    std::cout
        << ExitCode::RecordingLoadFailure
        << "  Recording load or format failure\n";

    std::cout
        << ExitCode::Cancelled
        << "  Operation cancelled\n";

    std::cout
        << ExitCode::Timeout
        << "  Operation timed out\n";

    std::cout
        << ExitCode::DisplayIncompatible
        << "  Display incompatibility\n";

    std::cout
        << ExitCode::ValidationFailure
        << "  Recording validation failure\n";
}

void printHelp()
{
    std::cout << "InputPlay\n\n";
    std::cout << "Commands:\n";
    std::cout << "  record <file>\n";

    std::cout
				<< "  play <file> [--send-input] [--align-start] "
				<< "[--start-immediately] [--loops <number|inf>] "
				<< "[--session <name>] [--timeout <seconds>] "
				<< "[--strict-display|--ignore-display]\n";

    std::cout << "  cancel <session>\n";
    std::cout << "  info <file>\n";
    std::cout << "  validate <file>\n";
    std::cout << "  test-model\n";
    std::cout << "  exit-codes\n";
}

const char* eventTypeName(EventType type)
{
    switch (type)
    {
        case EventType::MouseMove:
            return "MouseMove";

        case EventType::MouseButtonDown:
            return "MouseButtonDown";

        case EventType::MouseButtonUp:
            return "MouseButtonUp";

        case EventType::MouseWheel:
            return "MouseWheel";

        case EventType::KeyDown:
            return "KeyDown";

        case EventType::KeyUp:
            return "KeyUp";
			
		case EventType::Wait:
			return "Wait";
			
		case EventType::MouseTeleport:
			return "MouseTeleport";

        default:
            return "Unknown";
    }
}

Recording createSampleRecording()
{
    Recording recording;

    InputEvent firstMove;
    firstMove.timestampMicroseconds = 0;
    firstMove.type = EventType::MouseMove;
    firstMove.mouseX = 500;
    firstMove.mouseY = 300;
    firstMove.mouseDeltaX = 5;
    firstMove.mouseDeltaY = 2;
    recording.addEvent(firstMove);

    InputEvent secondMove;
    secondMove.timestampMicroseconds = 10'000;
    secondMove.type = EventType::MouseMove;
    secondMove.mouseX = 503;
    secondMove.mouseY = 299;
    secondMove.mouseDeltaX = 3;
    secondMove.mouseDeltaY = -1;
    recording.addEvent(secondMove);

    InputEvent leftButtonDown;
    leftButtonDown.timestampMicroseconds = 20'000;
    leftButtonDown.type = EventType::MouseButtonDown;
    leftButtonDown.mouseX = 503;
    leftButtonDown.mouseY = 299;
    leftButtonDown.mouseButton = 1;
    recording.addEvent(leftButtonDown);

	InputEvent leftButtonUp;
	leftButtonUp.timestampMicroseconds = 70'000;
	leftButtonUp.type = EventType::MouseButtonUp;
	leftButtonUp.mouseX = 503;
	leftButtonUp.mouseY = 299;
	leftButtonUp.mouseButton = 1;
	recording.addEvent(leftButtonUp);

    return recording;
}

int runRecordCommand(
    const std::string& filePath,
    const Settings& settings)
{
    Recording recording;
    InputRecorder recorder;
    std::string errorMessage;

    if (!recorder.record(
        recording,
        settings,
        errorMessage))
    {
        std::cerr << "Recording failed: "
                  << errorMessage
                  << '\n';

        return ExitCode::GeneralFailure;
    }

    if (!RecordingFile::save(
            recording,
            filePath,
            errorMessage))
    {
        std::cerr << "Unable to save recording: "
                  << errorMessage
                  << '\n';

        return ExitCode::GeneralFailure;
    }

    std::cout << "Recording saved\n";
    std::cout << "File: " << filePath << '\n';
    std::cout << "Events: "
              << recording.eventCount()
              << '\n';

    return ExitCode::Success;
}
int runInfoCommand(const std::string& filePath)
{
    Recording recording;
    std::string errorMessage;

    if (!RecordingFile::load(
            filePath,
            recording,
            errorMessage))
    {
        std::cerr
            << "Unable to load recording: "
            << errorMessage
            << '\n';

        return ExitCode::RecordingLoadFailure;
    }

    std::size_t mouseMoveCount = 0;
    std::size_t mouseTeleportCount = 0;
    std::size_t mouseButtonCount = 0;
    std::size_t mouseWheelCount = 0;
    std::size_t keyboardCount = 0;
    std::size_t waitCount = 0;

    std::uint64_t durationMicroseconds = 0;
    std::uint64_t explicitWaitMicroseconds = 0;

    for (const InputEvent& event : recording.events())
    {
        if (event.timestampMicroseconds
            > durationMicroseconds)
        {
            durationMicroseconds =
                event.timestampMicroseconds;
        }

        switch (event.type)
        {
            case EventType::MouseMove:
                ++mouseMoveCount;
                break;

            case EventType::MouseTeleport:
                ++mouseTeleportCount;
                break;

            case EventType::MouseButtonDown:
            case EventType::MouseButtonUp:
                ++mouseButtonCount;
                break;

            case EventType::MouseWheel:
                ++mouseWheelCount;
                break;

            case EventType::KeyDown:
            case EventType::KeyUp:
                ++keyboardCount;
                break;

            case EventType::Wait:
                ++waitCount;

                explicitWaitMicroseconds +=
                    event.waitMicroseconds;

                break;
        }
    }

    const double durationSeconds =
        static_cast<double>(
            durationMicroseconds
            + explicitWaitMicroseconds)
        / 1'000'000.0;

    std::cout
        << "InputPlay recording information\n\n";

    std::cout
        << "File:                  "
        << filePath
        << '\n';

    std::cout
        << "Events:                "
        << recording.eventCount()
        << '\n';

    std::cout
        << "Duration:              "
        << std::fixed
        << std::setprecision(3)
        << durationSeconds
        << " seconds\n";

    if (recording.hasStartingCursorPosition())
    {
        std::cout
            << "Starting cursor:       "
            << recording.startingCursorX()
            << ", "
            << recording.startingCursorY()
            << '\n';
    }
    else
    {
        std::cout
            << "Starting cursor:       Unknown\n";
    }

    if (recording.hasDisplayMetadata())
    {
        const DisplayMetadata& display =
            recording.displayMetadata();

        std::cout
            << "Monitor count:         "
            << display.monitors.size()
            << '\n';

        std::cout
            << "Virtual desktop:       "
            << display.virtualDesktopLeft
            << ","
            << display.virtualDesktopTop
            << " "
            << display.virtualDesktopWidth
            << "x"
            << display.virtualDesktopHeight
            << '\n';

        for (std::size_t index = 0;
             index < display.monitors.size();
             ++index)
        {
            const MonitorMetadata& monitor =
                display.monitors[index];

            const int monitorWidth =
                monitor.right - monitor.left;

            const int monitorHeight =
                monitor.bottom - monitor.top;

            std::cout
                << "Monitor "
                << index + 1
                << ":             "
                << monitor.deviceName
                << " at "
                << monitor.left
                << ","
                << monitor.top
                << " "
                << monitorWidth
                << "x"
                << monitorHeight;

            if (monitor.primary)
            {
                std::cout << " (primary)";
            }

            std::cout << '\n';
        }
    }
    else
    {
        std::cout
            << "Display metadata:      Not available\n";
    }

    std::cout << '\n';

    std::cout
        << "Mouse movements:       "
        << mouseMoveCount
        << '\n';

    std::cout
        << "Mouse teleports:       "
        << mouseTeleportCount
        << '\n';

    std::cout
        << "Mouse button events:   "
        << mouseButtonCount
        << '\n';

    std::cout
        << "Mouse wheel events:    "
        << mouseWheelCount
        << '\n';

    std::cout
        << "Keyboard events:       "
        << keyboardCount
        << '\n';

    std::cout
        << "Wait events:           "
        << waitCount
        << '\n';

    if (!recording.hasDisplayMetadata())
    {
        std::cout
            << "\nDisplay compatibility: Unknown\n";

        return ExitCode::Success;
    }

    DisplayMetadata currentDisplay;

    if (!captureDisplayMetadata(
            currentDisplay,
            errorMessage))
    {
        std::cout
            << "\nDisplay compatibility: Unable to check\n";

        std::cout
            << "Reason: "
            << errorMessage
            << '\n';

        return ExitCode::Success;
    }

    std::string compatibilityMessage;

    const DisplayCompatibility compatibility =
        compareDisplayMetadata(
            recording.displayMetadata(),
            currentDisplay,
            compatibilityMessage);

    std::cout << "\nDisplay compatibility: ";

    switch (compatibility)
    {
        case DisplayCompatibility::Exact:
            std::cout << "Exact\n";
            break;

        case DisplayCompatibility::CompatibleWithWarnings:
            std::cout << "Compatible with warnings\n";
            break;

        case DisplayCompatibility::Incompatible:
            std::cout << "Incompatible\n";
            break;

        case DisplayCompatibility::Unknown:
            std::cout << "Unknown\n";
            break;
    }

    std::cout
        << "Compatibility details: "
        << compatibilityMessage
        << '\n';

    return ExitCode::Success;
}

int runValidateCommand(
    const std::string& filePath)
{
    Recording recording;
    std::string errorMessage;

    if (!RecordingFile::load(
            filePath,
            recording,
            errorMessage))
    {
        std::cerr
            << "Unable to load recording: "
            << errorMessage
            << '\n';

        return ExitCode::RecordingLoadFailure;
    }

    std::cout
        << "Validating recording\n\n";

    std::cout
        << "File:           "
        << filePath
        << '\n';

    std::cout
        << "Events checked: "
        << recording.eventCount()
        << '\n';

    const ValidationResult result =
        validateRecording(recording);

    std::cout
        << "Warnings:       "
        << result.warningCount()
        << '\n';

    std::cout
        << "Errors:         "
        << result.errorCount()
        << "\n\n";

    for (const std::string& warning
         : result.warnings)
    {
        std::cout
            << "WARNING: "
            << warning
            << '\n';
    }

    for (const std::string& error
         : result.errors)
    {
        std::cout
            << "ERROR: "
            << error
            << '\n';
    }

    if (!result.warnings.empty()
        || !result.errors.empty())
    {
        std::cout << '\n';
    }

    if (result.valid())
    {
        std::cout
            << "Result: Valid\n";

        return ExitCode::Success;
    }

    std::cout
        << "Result: Invalid\n";

    return ExitCode::ValidationFailure;
}

int runModelTest()
{
    const Recording recording = createSampleRecording();

    std::cout << "Recording model test\n";
    std::cout << "Event count: "
              << recording.eventCount()
              << '\n';

    for (const InputEvent& event : recording.events())
    {
        std::cout << "Event timestamp: "
                  << event.timestampMicroseconds
                  << " microseconds\n";
    }

    if (recording.eventCount() != 4)
    {
        std::cerr << "Model test failed\n";
        return ExitCode::GeneralFailure;
    }

    std::cout << "Model test passed\n";
    return ExitCode::Success;
}

int runCancelCommand(
    const std::string& sessionName)
{
    std::string errorMessage;

    if (!CancellationSession::requestCancellation(
            sessionName,
            errorMessage))
    {
        std::cerr
            << "Unable to cancel session: "
            << errorMessage
            << '\n';

        return ExitCode::GeneralFailure;
    }

    std::cout
        << "Cancellation requested for session: "
        << sessionName
        << '\n';

    return ExitCode::Success;
}
}

int runCommandLine(
    int argc,
    char* argv[])
{
    Settings settings;
	std::string settingsPath;
	std::string settingsError;

	if (!loadOrCreateSettings(
			settings,
			settingsPath,
			settingsError))
	{
		std::cerr
			<< "Unable to load settings: "
			<< settingsError
			<< '\n';

		return ExitCode::GeneralFailure;
	}
		
	
	
	
    if (argc < 2)
    {
        std::cout << "InputPlay\n";
        std::cout << "Use --help for commands\n";
        return ExitCode::Success;
    }

    const std::string command = argv[1];

    if (command == "--help" || command == "help")
    {
        printHelp();
        return ExitCode::Success;
    }
	
	if (command == "exit-codes")
	{
		printExitCodes();
		return ExitCode::Success;
	}

    if (command == "test-model")
    {
        return runModelTest();
    }

    if (command == "record")
    {
        if (argc < 3)
        {
            std::cerr << "Missing recording file path\n";
            std::cerr << "Usage: InputPlay record <file>\n";
            return ExitCode::InvalidArguments;
        }

        return runRecordCommand(
			argv[2],
			settings);
    }

    if (command == "info")
    {
        if (argc < 3)
        {
            std::cerr << "Missing recording file path\n";
            std::cerr << "Usage: InputPlay info <file>\n";
            return ExitCode::InvalidArguments;
        }

        return runInfoCommand(argv[2]);
    }
	
	if (command == "validate")
	{
		if (argc < 3)
		{
			std::cerr
				<< "Missing recording file path\n";

			std::cerr
				<< "Usage: InputPlay validate <file>\n";

			return ExitCode::InvalidArguments;
		}

		return runValidateCommand(argv[2]);
	}
	
	if (command == "cancel")
	{
		if (argc < 3)
		{
			std::cerr
				<< "Missing playback session name\n";

			std::cerr
				<< "Usage: InputPlay cancel <session>\n";

			return ExitCode::InvalidArguments;
		}

		return runCancelCommand(argv[2]);
	}

	if (command == "play")
	{
		if (argc < 3)
		{
			std::cerr
				<< "Usage: InputPlay play <file> "
				<< "[--send-input] [--align-start] "
				<< "[--start-immediately] [--loops <number|inf>] "
				<< "[--session <name>] [--timeout <seconds>] "
				<< "[--strict-display|--ignore-display]\n";
			return ExitCode::InvalidArguments;
		}

		bool useSendInput = false;

		PlaybackOptions options;
		options.loopCount = settings.defaultLoops;

		for (int argumentIndex = 3;
			 argumentIndex < argc;
			 ++argumentIndex)
		{
			const std::string option =
				argv[argumentIndex];

			if (option == "--send-input")
			{
				useSendInput = true;
			}
			else if (option == "--start-immediately")
			{
				options.startImmediately = true;
			}
			else if (option == "--align-start")
			{
				options.alignStart = true;
			}
			else if (option == "--strict-display")
			{
				options.strictDisplay = true;
			}
			else if (option == "--ignore-display")
			{
				options.ignoreDisplay = true;
			}
			else if (option == "--session")
			{
				if (argumentIndex + 1 >= argc)
				{
					std::cerr
						<< "--session requires a session name\n";

					return ExitCode::InvalidArguments;
				}

				options.sessionName =
					argv[++argumentIndex];

				if (!CancellationSession::isValidSessionName(options.sessionName))
				{
					std::cerr
						<< "Invalid session name. Use only letters, "
						<< "numbers, hyphens, and underscores.\n";

					return ExitCode::InvalidArguments;
				}
			}
			else if (option == "--timeout")
			{
				if (argumentIndex + 1 >= argc)
				{
					std::cerr
						<< "--timeout requires a positive number "
						<< "of seconds\n";

					return ExitCode::InvalidArguments;
				}

				const std::string timeoutValue =
					argv[++argumentIndex];

				try
				{
					std::size_t parsedLength = 0;

					const unsigned long parsed =
						std::stoul(
							timeoutValue,
							&parsedLength,
							10);

					if (parsedLength != timeoutValue.length())
					{
						throw std::invalid_argument(
							"timeout contains extra characters");
					}

					if (parsed == 0)
					{
						throw std::invalid_argument(
							"zero timeout");
					}

					if (parsed
						> std::numeric_limits<unsigned int>::max())
					{
						throw std::out_of_range(
							"timeout is too large");
					}

					options.timeoutSeconds =
						static_cast<unsigned int>(parsed);

					options.timeoutEnabled = true;
				}
				catch (...)
				{
					std::cerr
						<< "Invalid timeout value: "
						<< timeoutValue
						<< '\n';

					return ExitCode::InvalidArguments;
				}
			}
			else if (option == "--loops")
			{
				if (argumentIndex + 1 >= argc)
				{
					std::cerr
						<< "--loops requires a number or inf\n";

					return ExitCode::InvalidArguments;
				}

				const std::string loopValue =
					argv[++argumentIndex];

				if (loopValue == "inf")
				{
					options.infiniteLoops = true;
				}
				else
				{
					try
					{
						std::size_t parsedLength = 0;

						const unsigned long parsed =
							std::stoul(
								loopValue,
								&parsedLength,
								10);

						if (parsedLength != loopValue.length())
						{
							throw std::invalid_argument(
								"loop count contains extra characters");
						}

						if (parsed == 0)
						{
							throw std::invalid_argument(
								"zero loops");
						}

						if (parsed
							> std::numeric_limits<unsigned int>::max())
						{
							throw std::out_of_range(
								"loop count is too large");
						}

						options.loopCount =
							static_cast<unsigned int>(parsed);
					}
					catch (...)
					{
						std::cerr
							<< "Invalid loop count: "
							<< loopValue
							<< '\n';

						return ExitCode::InvalidArguments;
					}
				}
			}
			else
			{
				std::cerr
					<< "Unknown playback option: "
					<< option
					<< '\n';

				return ExitCode::InvalidArguments;
			}
		}

		if (options.strictDisplay && options.ignoreDisplay)
		{
			std::cerr
				<< "--strict-display and --ignore-display "
				<< "cannot be used together\n";

			return ExitCode::InvalidArguments;
		}
		if (useSendInput)
		{
			std::cout
				<< "WARNING: Live input playback enabled\n";

			SendInputBackend backend;

			return runPlayback(
				argv[2],
				backend,
				settings,
				options);
		}

		DryRunBackend backend;

		return runPlayback(
			argv[2],
			backend,
			settings,
			options);
	}

    std::cerr << "Unknown command: "
              << command
              << '\n';

    return ExitCode::InvalidArguments;
}