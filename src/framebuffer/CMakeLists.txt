if (QT4_FOUND AND OPENGL_FOUND AND GLEW_FOUND)
    include (${QT_USE_FILE})
    include_directories (${QT_INCLUDES} ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDES} ${GLUT_INCLUDES} ${OCIO_INCLUDES} ${CMAKE_BINARY_DIR}/framebuffer ${PROJECT_SOURCE_DIR}/framebuffer)
    message(STATUS "UIC: ${QT_UIC_EXECUTABLE}\n")
    
    SET(framebuffer_HEADERS ${PROJECT_SOURCE_DIR}/framebuffer/mainwindow.h
                            ${PROJECT_SOURCE_DIR}/framebuffer/glwidget.h
                            ${PROJECT_SOURCE_DIR}/framebuffer/preferencesdialog.h)
                            
    SET(framebuffer_FORMS ${PROJECT_SOURCE_DIR}/framebuffer/mainwindow.ui
                          ${PROJECT_SOURCE_DIR}/framebuffer/preferencesdialog.ui)

    QT4_WRAP_CPP(framebuffer_HEADERS_MOC ${framebuffer_HEADERS})
    QT4_WRAP_UI(framebuffer_FORMS_HEADERS ${framebuffer_FORMS})
    
    set (framebuffer_srcs glwidget.cpp imagebuffer.cpp main.cpp mainwindow.cpp preferencesdialog.cpp)
    if (FORCE_OPENGL_1)
        add_definitions(-DFORCE_OPENGL_1)
    endif()
    add_executable (framebuffer ${framebuffer_HEADERS_MOC} ${framebuffer_FORMS_HEADERS} ${framebuffer_srcs})
    link_ilmbase (framebuffer)
    target_link_libraries (framebuffer OpenImageIO ${OCIO_LIBRARIES} ${QT_LIBRARIES} ${OPENGL_LIBRARIES} 
                               ${GLUT_LIBRARIES} ${CMAKE_DL_LIBS}
                               ${GLEW_LIBRARIES})
    oiio_install_targets (framebuffer)

else ()
    message (STATUS "\n\n   WARNING: No QT/OpenGL/Glew/Glut/OCIO -- 'framebuffer' will not be built!\n")
endif ()
