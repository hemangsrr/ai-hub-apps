// ---------------------------------------------------------------------
// Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause
// ---------------------------------------------------------------------

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "ChatServer.hpp"
#include "httplib.h"
#include <windows.h>

namespace
{

constexpr const std::string_view c_option_genie_config = "--genie-config";
constexpr const std::string_view c_option_base_dir = "--base-dir";
constexpr const std::string_view c_option_help = "--help";
constexpr const std::string_view c_option_help_short = "-h";

void PrintHelp()
{
    std::cout << "\n:::::::: Chat with " << App::c_bot_name << " options ::::::::\n\n";
    std::cout << c_option_genie_config << " <Local file path>: [Required] Path to local Genie config for model.\n";
    std::cout << c_option_base_dir
              << " <Local directory path>: [Required] Base directory to set as the working directory.\n";
}

} // namespace

int main(int argc, char* argv[])
{
    std::string genie_config_path;
    std::string base_dir;
    bool invalid_arguments = false;

    SetConsoleOutputCP(CP_UTF8);

    for (int i = 1; i < argc; ++i)
    {
        if (c_option_genie_config == argv[i])
        {
            if (i + 1 < argc)
            {
                genie_config_path = argv[++i];
            }
            else
            {
                std::cout << "\nMissing value for " << c_option_genie_config << " option.\n";
                invalid_arguments = true;
            }
        }
        else if (c_option_base_dir == argv[i])
        {
            if (i + 1 < argc)
            {
                base_dir = argv[++i];
            }
            else
            {
                std::cout << "\nMissing value for " << c_option_base_dir << " option.\n";
                invalid_arguments = true;
            }
        }
        else if (c_option_help == argv[i] || c_option_help_short == argv[i])
        {
            PrintHelp();
            return 0;
        }
        else
        {
            std::cout << "Unsupported option " << argv[i] << " provided.\n";
            invalid_arguments = true;
        }
    }

    if (invalid_arguments || genie_config_path.empty() || base_dir.empty())
    {
        PrintHelp();
        return 1;
    }

    try
    {
        std::ifstream config_file(genie_config_path);
        if (!config_file)
        {
            throw std::runtime_error("Failed to open Genie config file: " + genie_config_path);
        }

        std::string config((std::istreambuf_iterator<char>(config_file)), std::istreambuf_iterator<char>());
        std::filesystem::current_path(base_dir);

        Server::ChatServer server(config);
        server.RunServer();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown error.\n";
        return 1;
    }
    return 0;
}
