#pragma once

#include "NetCommon/NetMessage.h"
#include "SimpleClient.h"

#include <NetCommon/FwNet.h>

#include <GLFW/glfw3.h> // Include glfw3.h after OpenGL32.lib
#include <array>
#include <ctime>
#include <iomanip>
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
    void render_ui(CustomClient& client)
    {
        if (ImGui::Button("Connect..."))
        {
            ImGui::OpenPopup("SetClientInfo");
        }

        if (ImGui::Button("Ping"))
        {
            client.ping_server();
        }

        if (ImGui::Button("Disconnect"))
        {
            fw::net::Message<CustomMsgTypes> disconnect_msg{};
            disconnect_msg.header.id = CustomMsgTypes::ClientDisconnected;
            client.send(disconnect_msg);
            client.disconnect();
        }

        // Begin Popup
        if (ImGui::BeginPopupModal("SetClientInfo"))
        {
            ImGui::Text("Choose a name:");
            ImGui::InputTextWithHint("##ClientInfoInput",
                                     "Your name...",
                                     client.name().data(),
                                     client.name().size());

            ImGui::Text("Choose a color:");
            ImGui::ColorEdit3("Color", client.color().data());

            if (ImGui::Button("Ok"))
            {
                if (!client.is_connected())
                {
                    client.connect("127.0.0.1", 60000);
                }

                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

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

        ImGui::Text("Message");
        ImGui::SameLine();

        static Message msg_to_send{};
        ImGui::InputTextWithHint("##ChatInput",
                                 "Type something...",
                                 msg_to_send.content.data(),
                                 msg_to_send.content.size());
        ImGui::SameLine();

        if (ImGui::Button("Send"))
        {
            client.set_sending_msg(std::move(msg_to_send));
            client.message_all();
        }

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
};