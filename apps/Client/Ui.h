#pragma once

#include "SimpleClient.h"

#include <array>

#include "NetCommon/NetMessage.h"
#include <NetCommon/FwNet.h>

#include <GLFW/glfw3.h> // Include glfw3.h after OpenGL32.lib
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

class Ui
{
  public:
    int init()
    {
        if (!glfwInit())
        {
            return -1;
        }

        window_ =
            glfwCreateWindow(1280, 720, "ImGui Minimal Example", nullptr, nullptr);

        if (!window_)
        {
            std::cerr << "Failed to create GLFW window\n";
            glfwTerminate();
            return -1;
        }

        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL3_Init("#version 130"); // OpenGL 3.x

        return 0;
    }

    void render_loop(CustomClient& client)
    {
        while (!glfwWindowShouldClose(window_))
        {
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            client.handle_incoming_msg();

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

            ImGui::Begin("Main Window",
                         nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_MenuBar |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse);

            render_ui(client);

            ImGui::End();

            ImGui::Render();
            glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window_);
        }
    }

    ~Ui() { cleanup(); }

  private:
    void render_menu(CustomClient& client)
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Options"))
            {
                if (client.is_connected())
                {
                    ImGui::BeginDisabled(true);
                }
                else
                {
                    ImGui::BeginDisabled(false);
                }
                if (ImGui::MenuItem("Connect..."))
                {
                    should_open_popup_ = true;
                }
                ImGui::EndDisabled();

                // if (ImGui::MenuItem("Ping"))
                // {
                //     client.ping_server();
                // }

                if (!client.is_connected())
                {
                    ImGui::BeginDisabled(true);
                }
                else
                {
                    ImGui::BeginDisabled(false);
                }
                if (ImGui::MenuItem("Disconnect"))
                {
                    fw::net::Message<CustomMsgTypes> disconnect_msg{};
                    disconnect_msg.header.id = CustomMsgTypes::ClientDisconnected;
                    client.send(disconnect_msg);
                    client.disconnect();
                    client.clear_other_clients_list();
                }
                ImGui::EndDisabled();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        if (should_open_popup_)
        {
            ImGui::OpenPopup("Connect");
            should_open_popup_ = false;
        }

        // Begin Popup
        if (ImGui::BeginPopupModal("Connect",
                                   nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize |
                                       ImGuiWindowFlags_NoResize))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                                ImVec2(10, 10)); // Add space between items
            ImGui::PushStyleVar(
                ImGuiStyleVar_FramePadding,
                ImVec2(8, 6)); // Add internal padding to inputs/buttons


            ImGui::Text("Enter your display name");
            ImGui::InputTextWithHint("##ClientInfoInput",
                                     "Your name...",
                                     client.name().data(),
                                     client.name().size());

            ImGui::ColorEdit3("###ColorPicker",
                              client.color().data(),
                              ImGuiColorEditFlags_NoInputs);
            ImGui::SameLine();
            ImGui::Text("Pick your favorite color");

            float button_width{ 100.0f };
            float spacing{ 20.0f };
            float total_width{ button_width * 2 + spacing };
            float window_width{ ImGui::GetWindowSize().x };
            float start_x{ (window_width - total_width) * 0.5f };

            ImGui::SetCursorPosX(start_x);

            if (ImGui::Button("Ok", ImVec2(button_width, 0)))
            {
                if (!client.is_connected())
                {
                    client.connect("127.0.0.1", 60000);
                }

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(button_width, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::PopStyleVar(2);

            ImGui::EndPopup();
        }
    }

    void render_ui(CustomClient& client)
    {
        render_menu(client);

        // Split into two columns
        ImGui::Columns(2, nullptr, true);

        // --- Left panel: Chats ---
        float available_height{ ImGui::GetContentRegionAvail().y };
        float padding{ 20.0f };
        available_height -= padding;

        float top_heigth{ available_height * 0.90f };
        float bottom_height{ available_height - top_heigth };

        // Top Row (75% height)
        ImGui::Text("Chats");
        ImGui::BeginChild("ChatsListPanel", ImVec2(0, top_heigth), true);

        for (const auto& msg : client.messages())
        {
            const auto color{ msg.client_info.color };
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4{ color[0], color[1], color[2], 1.0f });

            ImGui::Text("%s: %s", msg.client_info.name.data(), msg.content.data());
            // Right-align the time text
            float window_width{ ImGui::GetWindowWidth() };
            float text_width{
                ImGui::CalcTextSize(msg.formatted_time().c_str()).x
            }; // Calculate the width of the time text
            ImGui::SetCursorPosX(window_width - text_width -
                                 10.0f); // Move the cursor to the right side

            // Push the color for the time text
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4{ color[0], color[1], color[2], 1.0f });
            ImGui::Text("%s", msg.formatted_time().c_str()); // Display the time text
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
        }

        // Your top content
        ImGui::EndChild();

        // Bottom Row (25% height)
        ImGui::BeginChild("InputPanel", ImVec2(0, bottom_height), true);

        ImGui::Dummy(ImVec2(0.0f, 10.0f)); // Bottom padding
        ImGui::Indent(40.0f);              // Left padding

        if (!client.is_connected())
        {
            ImGui::BeginDisabled(true);
        }
        else
        {
            ImGui::BeginDisabled(false);
        }

        ImGui::Text("Message");
        ImGui::SameLine();

        ImGui::InputTextWithHint("##ChatInput",
                                 "Type something...",
                                 msg_to_send_.content.data(),
                                 msg_to_send_.content.size());
        ImGui::SameLine();

        if (ImGui::Button("Send"))
        {
            client.set_sending_msg(std::move(msg_to_send_));
            client.message_all();
            msg_to_send_.content.fill('\0');
        }
        ImGui::EndDisabled();

        ImGui::Dummy(ImVec2(0.0f, 10.0f)); // Bottom padding

        ImGui::EndChild();

        ImGui::NextColumn();
        // --- Left panel: Chats ---

        // --- Right panel: Clients ---
        ImGui::Text("Clients");
        ImGui::BeginChild("ClientsChild", ImVec2(0, -30), true);
        for (const auto& other_client : client.other_clients_list())
        {
            const auto color{ other_client.color };
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4{ color[0], color[1], color[2], 1.0f });
            ImGui::Text("%s", other_client.to_string().c_str());
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();

        // --- Right panel: Clients ---
    }

    void cleanup()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window_);
        glfwTerminate();
    }

  private:
    GLFWwindow* window_;
    bool should_open_popup_{ false };
    Message msg_to_send_{};
};