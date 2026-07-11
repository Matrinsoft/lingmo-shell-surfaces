set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Test Quick QuickTest)
include(CTest)

function(lingmo_add_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE
        Lingmo::ShellSurfaces
        Qt6::Test
        Qt6::Quick
        Qt6::Qml
    )
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES TIMEOUT 60)
endfunction()

lingmo_add_test(tst_ShellSurface)
lingmo_add_test(tst_PanelSurface)
lingmo_add_test(tst_DesktopSurface)
lingmo_add_test(tst_OverviewSurface)
lingmo_add_test(tst_SurfaceBackend)

# QML import test needs the QML plugin built alongside
add_executable(tst_QmlImport tst_QmlImport.cpp)
target_link_libraries(tst_QmlImport PRIVATE
    Lingmo::ShellSurfaces
    Qt6::Test
    Qt6::Quick
    Qt6::Qml
)
target_compile_definitions(tst_QmlImport PRIVATE
    QML_IMPORT_PATH="${CMAKE_BINARY_DIR}/qml"
)
add_test(NAME tst_QmlImport COMMAND tst_QmlImport)
set_tests_properties(tst_QmlImport PROPERTIES TIMEOUT 60)

lingmo_add_test(tst_LayerShell)
lingmo_add_test(tst_WindowManagerBackend)
target_include_directories(tst_WindowManagerBackend PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

lingmo_add_test(tst_MockBackend)
target_include_directories(tst_MockBackend PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)
