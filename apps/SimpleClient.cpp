#include "NetCommon/NetMessage.h"
#include <NetCommon/FwNet.h>

#include <GLFW/glfw3.h> // Include glfw3.h after OpenGL32.lib
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <cstdint>
#include <iostream>
#include <windows.h>
enum class CustomMsgTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
};

class CustomClient : public fw::net::ClientInterface<CustomMsgTypes>
{
  public:
    void ping_server()
    {
        fw::net::Message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::ServerPing;

        // Caution with this
        std::chrono::system_clock::time_point time_now{
            std::chrono::system_clock::now()
        };

        msg << time_now;

        send(msg);
    }

    void message_all()
    {
        fw::net::Message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::MessageAll;

        send(msg);
    }
};

void cleanup(GLFWwindow* window)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

int setup_window(GLFWwindow* window)
{
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130"); // OpenGL 3.x

    return 0;
}

void handle_incoming_msg(CustomClient& client)
{
    if (client.is_connected())
    {
        if (!client.incoming().empty())
        {
            auto msg{ client.incoming().pop_front().msg };
            switch (msg.header.id)
            {
            case CustomMsgTypes::ServerAccept:
            {
                std::cout << "Server Accepted Connection\n";
            }
            break;

            case CustomMsgTypes::ServerPing:
            {
                std::chrono::system_clock::time_point time_now{
                    std::chrono::system_clock::now()
                };
                std::chrono::system_clock::time_point time_then;
                msg >> time_then;

                std::cout
                    << "Ping: "
                    << std::chrono::duration<double>(time_now - time_then).count()
                    << "\n";
            }
            break;

            case CustomMsgTypes::ServerMessage:
            {
                uint32_t clientID{};
                msg >> clientID;
                std::cout << "Hello from []" << clientID << "]\n";
            }
            break;
            }
        }
    }
}

void render_loop(GLFWwindow* window, CustomClient& client)
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Hello, ImGui!", nullptr, ImGuiWindowFlags_MenuBar);
        ImGui::Text("This is a simple ImGui example.");

        handle_incoming_msg(client);

        if (ImGui::Button("Exit"))
        {
            glfwSetWindowShouldClose(window, true);
        }
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Options"))
            {
                if (ImGui::MenuItem("Connect"))
                {
                    if (!client.is_connected())
                    {
                        client.connect("127.0.0.1", 60000);
                    }
                }

                if (ImGui::MenuItem("Ping"))
                {
                    client.ping_server();
                }
                if (ImGui::MenuItem("Disconnect"))
                {
                    client.disconnect();
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }


        ImGui::End();

        ImGui::Render();
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

int main()
{
    // fw::net::Message<CustomMsgTypes> msg;
    // msg.header.id = CustomMsgTypes::FireBullet;

    // int a{ 1 };
    // bool b{ true };
    // float c{ 3.14159f };

    // struct Data
    // {
    //     float x;
    //     float y;
    // };

    // std::array<Data, 5> d{};

    // msg << a << b << c << d;

    // a = 99;
    // b = false;
    // c = 99.0f;

    // msg >> d >> c >> b >> a;

    CustomClient client;

    // std::array keys{ false, false, false, false };
    // std::array old_keys{ false, false, false, false };

    // bool is_quit{ false };
    // while (!is_quit)
    // {
    //     if (GetForegroundWindow() == GetConsoleWindow())
    //     {
    //         keys[0] = GetAsyncKeyState('1') & 0x8000;
    //         keys[1] = GetAsyncKeyState('2') & 0x8000;
    //         keys[2] = GetAsyncKeyState('3') & 0x8000;
    //         keys[3] = GetAsyncKeyState('4') & 0x8000;
    //     }

    //     if (keys[0] && !old_keys[0])
    //     {
    //         client.ping_server();
    //     }

    //     if (keys[3] && !old_keys[3])
    //     {
    //         client.message_all();
    //     }

    //     if (keys[2] && !old_keys[2])
    //     {
    //         is_quit = true;
    //     }

    //     for (int i{ 0 }; i < keys.size(); ++i)
    //     {
    //         old_keys[i] = keys[i];
    //     }

    //     if (client.is_connected())
    //     {
    //         if (!client.incoming().empty())
    //         {
    //             auto msg{ client.incoming().pop_front().msg };
    //             switch (msg.header.id)
    //             {
    //             case CustomMsgTypes::ServerAccept:
    //             {
    //                 std::cout << "Server Accepted Connection\n";
    //             }
    //             break;

    //             case CustomMsgTypes::ServerMessage:
    //             {
    //                 uint32_t clientID{};
    //                 msg >> clientID;
    //                 std::cout << "Hello from []" << clientID << "]\n";
    //             }
    //             break;

    //             case CustomMsgTypes::ServerPing:
    //             {
    //                 std::chrono::system_clock::time_point time_now{
    //                     std::chrono::system_clock::now()
    //                 };
    //                 std::chrono::system_clock::time_point time_then;
    //                 msg >> time_then;

    //                 std::cout << "Ping: "
    //                           << std::chrono::duration<double>(time_now -
    //                           time_then)
    //                                  .count()
    //                           << "\n";
    //             }
    //             break;
    //             }
    //         }
    //     }
    //     else
    //     {
    //         std::cout << "Server Down\n";
    //         is_quit = true;
    //     }
    // }

    // return 0;

    if (!glfwInit())
    {
        return -1;
    }

    GLFWwindow* window{ glfwCreateWindow(
        1280, 720, "ImGui Minimal Example", nullptr, nullptr) };

    if (setup_window(window) == -1)
        return -1;

    render_loop(window, client);

    cleanup(window);

    return 0;
}