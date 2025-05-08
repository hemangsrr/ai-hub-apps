// ---------------------------------------------------------------------
// Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause
// ---------------------------------------------------------------------

#pragma once

#include <string>

#include "GenieCommon.h"
#include "GenieDialog.h"

namespace Server
{
constexpr const std::string_view c_exit_prompt = "exit";
constexpr const std::string_view c_bot_name = "Qbot";

class ChatServer
{
  private:
    GenieDialogConfig_Handle_t m_config_handle = nullptr;
    GenieDialog_Handle_t m_dialog_handle = nullptr;

  public:
    ChatServer(const std::string& config);
    ChatServer() = delete;
    ChatServer(const ChatServer&) = delete;
    ChatServer(ChatServer&&) = delete;
    ChatServer& operator=(const ChatServer&) = delete;
    ChatServer& operator=(ChatServer&&) = delete;
    ~ChatServer();

    void RunServer();
};
} // namespace App
