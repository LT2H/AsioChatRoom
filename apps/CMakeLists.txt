add_executable(ClientApp Client/ClientApp.cpp)

target_include_directories(ClientApp PRIVATE ${asio_SOURCE_DIR}/include)

target_compile_features(ClientApp PRIVATE cxx_std_20)
target_link_libraries(ClientApp PRIVATE imgui_backend NetCommon ${PLATFORM_LIBS})


add_executable(SimpleServer Server/SimpleServer.cpp)

target_include_directories(SimpleServer PRIVATE ${asio_SOURCE_DIR}/include)

target_compile_features(SimpleServer PRIVATE cxx_std_20)
target_link_libraries(SimpleServer PRIVATE NetCommon ${PLATFORM_LIBS})