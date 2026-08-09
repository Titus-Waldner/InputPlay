#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "InputPlay\n";
        std::cout << "Use --help for commands\n";
        return 0;
    }

    std::string command = argv[1];

    if (command == "--help")
    {
        std::cout << "InputPlay\n\n";
        std::cout << "Commands:\n";
        std::cout << "  record\n";
        std::cout << "  play\n";
        std::cout << "  info\n";
        return 0;
    }

    if (command == "record")
    {
        std::cout << "Record command selected\n";
        return 0;
    }

    if (command == "play")
    {
        std::cout << "Play command selected\n";
        return 0;
    }

    if (command == "info")
    {
        std::cout << "Info command selected\n";
        return 0;
    }

    std::cout << "Unknown command: "
              << command
              << std::endl;

    return 1;
}