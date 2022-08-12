project (pineapple-pictures)

cmake_minimum_required(VERSION 3.16)

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (QT_MINIMUM_VERSION "5.12.5")

find_package(Qt5 ${QT_MINIMUM_VERSION} CONFIG REQUIRED Widgets Svg LinguistTools)

set (PPIC_CPP_FILES
    main.cpp
    mainwindow.cpp
    graphicsview.cpp
    graphicsscene.cpp
    bottombuttongroup.cpp
    navigatorview.cpp
    opacityhelper.cpp
    toolbutton.cpp
)

set (PPIC_HEADER_FILES
    mainwindow.h
    graphicsview.h
    graphicsscene.h
    bottombuttongroup.h
    navigatorview.h
    opacityhelper.h
    toolbutton.h
)

set (PPIC_ORC_FILES
    resources.qrc
)

set (EXE_NAME ppic)

file (GLOB PPIC_TS_FILE languages/*.ts)
set (PPIC_CPP_FILE_FOR_I18N ${PPIC_CPP_FILES})

qt5_create_translation(PPIC_QM_FILES ${PPIC_CPP_FILES_FOR_I18N} ${PPIC_TS_FILE})

add_executable(${EXE_NAME}
    ${PPIC_HEADER_FILES}
    ${PPIC_CPP_FILES}
    ${PPIC_HEADER_FILES}
    ${PPIC_ORC_FILES}
    ${PPIC_QM_FILES}
)

target_link_libraries(${EXE_NAME} Qt5::Widgets Qt5::Svg)

if (UNIX)
    if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX /usr)
    endif()

    # 二进制
    install (
        TARGETS ${EXE_NAME}
        RUNTIME DESTINATION ${CMAKE_IINSTALL_BINDIR}
        )

    # 图标
    install (
        FILES icons/app-icon.svg
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps"
        RENAME pineapple-pictures.svg
    )

    # desktop文件
    install (
        FILES  pineapple-pictures.desktop
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
     )

endif()





