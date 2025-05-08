// ---------------------------------------------------------------------
// Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause
// ---------------------------------------------------------------------

#include "ChatServer.hpp"
#include "PromptHandler.hpp"
#include "httplib.h"
#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>

using namespace Server;

namespace
{
constexpr const int c_chat_separater_length = 80;

void ChatSplit(bool end_line = true)
{
    std::string split_line(c_chat_separater_length, '-');
    std::cout << "\n" << split_line;
    if (end_line)
    {
        std::cout << "\n";
    }
}

std::wstring Utf8ToUtf16(const std::string& utf8)
{
    if (utf8.empty())
        return L"";

    int wide_size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    if (wide_size == 0)
        return L"";

    std::wstring utf16(wide_size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &utf16[0], wide_size);
    return utf16;
}

std::string Utf16ToUtf8(const std::wstring& utf16)
{
    if (utf16.empty())
        return "";

    int utf8_size = WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utf8_size == 0)
        return "";

    std::string utf8(utf8_size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), -1, &utf8[0], utf8_size, nullptr, nullptr);
    return utf8;
}

void GenieCallBack(const char* response_back, const GenieDialog_SentenceCode_t sentence_code, const void* user_data)
{
    std::string* user_data_str = static_cast<std::string*>(const_cast<void*>(user_data));
    user_data_str->append(response_back);

    // Convert UTF-8 response to UTF-16 for proper console output
    std::wstring wide_response = Utf8ToUtf16(response_back);

    // Output the converted UTF-16 string
    std::wcout << wide_response << std::endl;

    if (sentence_code == GenieDialog_SentenceCode_t::GENIE_DIALOG_SENTENCE_END)
    {
        ChatSplit(false);
    }
}

} // namespace

ChatServer::ChatServer(const std::string& config)
{
    if (GENIE_STATUS_SUCCESS != GenieDialogConfig_createFromJson(config.c_str(), &m_config_handle))
    {
        throw std::runtime_error("Failed to create the Genie Dialog config. Please check config.");
    }

    if (GENIE_STATUS_SUCCESS != GenieDialog_create(m_config_handle, &m_dialog_handle))
    {
        throw std::runtime_error("Failed to create the Genie Dialog.");
    }
}

ChatServer::~ChatServer()
{
    if (m_config_handle != nullptr)
    {
        if (GENIE_STATUS_SUCCESS != GenieDialogConfig_free(m_config_handle))
        {
            std::cerr << "Failed to free the Genie Dialog config.";
        }
    }

    if (m_dialog_handle != nullptr)
    {
        if (GENIE_STATUS_SUCCESS != GenieDialog_free(m_dialog_handle))
        {
            std::cerr << "Failed to free the Genie Dialog.";
        }
    }
}

void ChatServer::RunServer()
{
    AppUtils::PromptHandler prompt_handler;
    httplib::Server svr;

    svr.Post(
        "/chat",
        [&](const httplib::Request& req, httplib::Response& res)
        {
            // Ensure UTF-8 handling
            std::string user_prompt = req.body;

            // Debugging: Log raw request bytes
            std::cout << "[DEBUG] Raw input bytes: " << user_prompt << std::endl;

            // Convert to wide string (UTF-16)
            std::wstring wide_user_prompt = Utf8ToUtf16(user_prompt);

            std::string model_response;
            std::string tagged_prompt =
                App::c_bot_name.data() + std::string(": ") + prompt_handler.GetPromptWithTag(user_prompt);

            std::cout << "[LOG] Received query: " << user_prompt << std::endl;

            if (GENIE_STATUS_SUCCESS != GenieDialog_query(m_dialog_handle, tagged_prompt.c_str(),
                                                          GenieDialog_SentenceCode_t::GENIE_DIALOG_SENTENCE_COMPLETE,
                                                          GenieCallBack, &model_response))
            {
                throw std::runtime_error("Failed to get response from GenieDialog. Please restart the ChatServer.");
            }

            if (model_response.empty())
            {
                if (GENIE_STATUS_SUCCESS != GenieDialog_reset(m_dialog_handle))
                {
                    throw std::runtime_error("Failed to reset Genie Dialog.");
                }
                if (GENIE_STATUS_SUCCESS !=
                    GenieDialog_query(m_dialog_handle, tagged_prompt.c_str(),
                                      GenieDialog_SentenceCode_t::GENIE_DIALOG_SENTENCE_COMPLETE, GenieCallBack,
                                      &model_response))
                {
                    throw std::runtime_error("Failed to get response from GenieDialog. Please restart the ChatServer.");
                }
            }

            std::cout << "[LOG] Generated response: " << model_response << std::endl;

            // Ensure UTF-8 encoding in response
            res.set_header("Content-Type", "text/plain; charset=utf-8");
            res.set_content(model_response, "text/plain; charset=utf-8");
        });

    std::cout << "Server listening on port 8080..." << std::endl;
    svr.listen("0.0.0.0", 8080);
}
