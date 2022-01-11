add_library(nebula_tools STATIC
    debug/DebugOverlay.cpp
    debug/Console.cpp
    editor/Editor.cpp
    editor/EditorViewport.cpp
    editor/EditorGUI.cpp
)
target_link_libraries(nebula_tools PUBLIC nebula_core ${SFML_LIBRARIES})
target_include_directories(nebula_tools PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
