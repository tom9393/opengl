#include "pch.h"
#include "Utils/PlatformUtils.h"

namespace Engine
{

    static std::string ExecuteCommand(const std::string& command)
    {
        std::array<char, 256> buffer{};
        std::string result;
        std::unique_ptr<FILE, int(*)(FILE*)> pipe(popen(command.c_str(), "r"), static_cast<int(*)(FILE*)>(pclose));
        if (!pipe)
            return "";
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
            result += buffer.data();

        // Remove trailing newline (zenity adds one)
        if (!result.empty() && result.back() == '\n')
            result.pop_back();

        return result;
    }

    std::string FileDialogs::OpenFile(const char* filter = nullptr)
    {
        std::string command = "zenity --file-selection --title=\"Open File\"";
        if (filter)
            command += std::string(" --file-filter=\"") + filter + "\"";

        return ExecuteCommand(command);
    }

    std::string FileDialogs::SaveFile(const char* filter = nullptr)
    {
        std::string command = "zenity --file-selection --save --confirm-overwrite --title=\"Save File\"";
        if (filter)
            command += std::string(" --file-filter=\"") + filter + "\"";

        return ExecuteCommand(command);
    }

};
