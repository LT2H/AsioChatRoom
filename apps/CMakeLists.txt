add_executable(app app.cpp)

# Ensure that the Asio headers are correctly included
target_include_directories(app PRIVATE ${asio_SOURCE_DIR}/include)

target_compile_features(app PRIVATE cxx_std_20)
target_link_libraries(app PRIVATE NetCommon ws2_32
    mswsock
    kernel32
    user32
    gdi32
    winspool
    shell32
    ole32
    oleaut32
    uuid
    comdlg32
    advapi32)




add_executable(SimpleClient SimpleClient.cpp)

target_include_directories(SimpleClient PRIVATE ${asio_SOURCE_DIR}/include)

target_compile_features(SimpleClient PRIVATE cxx_std_20)
target_link_libraries(SimpleClient PRIVATE imgui_backend NetCommon ws2_32
    mswsock
    kernel32
    user32
    gdi32
    winspool
    shell32
    ole32
    oleaut32
    uuid
    comdlg32
    advapi32)




add_executable(SimpleServer SimpleServer.cpp)

target_include_directories(SimpleServer PRIVATE ${asio_SOURCE_DIR}/include)

target_compile_features(SimpleServer PRIVATE cxx_std_20)
target_link_libraries(SimpleServer PRIVATE NetCommon ws2_32
    mswsock
    kernel32
    user32
    gdi32
    winspool
    shell32
    ole32
    oleaut32
    uuid
    comdlg32
    advapi32)