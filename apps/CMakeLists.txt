add_executable(app app.cpp)

# Ensure that the Asio headers are correctly included
target_include_directories(app PRIVATE ${asio_SOURCE_DIR}/include)

target_compile_features(app PRIVATE cxx_std_20)
target_link_libraries(app PRIVATE NetCommon)
