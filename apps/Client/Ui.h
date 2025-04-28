#pragma once

#include "SimpleClient.h"

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
    void render_ui(CustomClient& client)
    {
        // if (ImGui::BeginMenuBar())
        // {
        if (ImGui::Button("Connect..."))
        {
            ImGui::OpenPopup("SetClientInfo");
        }
        // if (ImGui::BeginMenu("Options"))
        // {
        //     if (ImGui::MenuItem("Connect"))
        //     {
        //         std::cout << "Opening popup...\n"; // Debug log
        //         ImGui::OpenPopup("SetClientInfo");
        //     }

        //     if (ImGui::MenuItem("Ping"))
        //     {
        //         client.ping_server();
        //     }
        //     if (ImGui::MenuItem("Disconnect"))
        //     {
        //         client.disconnect();
        //     }
        //     if (ImGui::MenuItem("Quit"))
        //     {
        //         glfwSetWindowShouldClose(window_, true);
        //     }

        //     ImGui::EndMenu();
        // }

        //     ImGui::EndMenuBar();
        // }

        // Begin Popup
        if (ImGui::BeginPopupModal("SetClientInfo"))
        {
            static std::string client_info{ "(unknown)" };

            ImGui::Text("Choose a name:");
            ImGui::InputTextWithHint("##ClientInfoInput",
                                     "Your name...",
                                     client_info.data(),
                                     client_info.size());

            client.set_info(client_info);

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
            ImGui::Text("%s", msg.data());
        }

        // Your top content
        ImGui::EndChild();

        // Bottom Row (25% height)
        ImGui::BeginChild("InputPanel", ImVec2(0, bottom_height), true);

        ImGui::Dummy(ImVec2(0.0f, 10.0f)); // Bottom padding
        ImGui::Indent(40.0f);              // Left padding

        ImGui::Text("Message");
        ImGui::SameLine();

        static std::array<char, fw::net::array_size> msg_content{};
        ImGui::InputTextWithHint("##ChatInput",
                                 "Type something...",
                                 msg_content.data(),
                                 msg_content.size());
        ImGui::SameLine();

        if (ImGui::Button("Send"))
        {
            client.set_sending_msg(std::move(msg_content));
            client.message_all();
        }

        ImGui::Dummy(ImVec2(0.0f, 10.0f)); // Bottom padding

        ImGui::EndChild();

        ImGui::NextColumn();
        // --- Left panel: Chats ---

        // --- Right panel: Clients ---
        ImGui::Text("Clients");
        ImGui::BeginChild("ClientsChild", ImVec2(0, -30), true);
        for (const auto& c : client.other_clients_list())
        {
            ImGui::Text("%s", c.name.c_str());
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