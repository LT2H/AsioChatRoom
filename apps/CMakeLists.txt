add_executable(ClientApp Client/ClientApp.cpp)

target_include_directories(ClientApp PRIVATE ${asio_SOURCE_DIR}/include)

target_compile_features(ClientApp PRIVATE cxx_std_20)
target_link_libraries(ClientApp PRIVATE imgui_backend NetCommon ${PLATFORM_LIBS})


add_executable(ServerApp Server/Server.cpp)

target_include_directories(ServerApp PRIVATE ${asio_SOURCE_DIR}/include)

target_compile_features(ServerApp PRIVATE cxx_std_20)
target_link_libraries(ServerApp PRIVATE NetCommon ${PLATFORM_LIBS})