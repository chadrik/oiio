#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>

#include "imagebuffer.h"

#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;

#include "imageio.h"
#include "imagebuf.h"

OIIO_NAMESPACE_USING;

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent = 0);
    
    void initializeGL();
    bool InitImageTexture(ImageBuffer &image);
    void AllocateLut3D();
    void InitOCIO(const char * filename);
    GLuint CompileShaderText(GLenum shaderType, const char *text);
    GLuint LinkShaders(GLuint fragShader);
    void UpdateOCIOGLState();
    void paintGL();
    void resizeGL(int w, int h);

    void exposure_CB(float Ev);
    void gamma_CB(float Y);

    GLint gl_get_uniform_location (const char*);
    void gl_uniform (GLint location, float value);

private slots:
    void inSpace_CB(int index);
    void outSpace_CB(int index);

public:
    int m_channelHot[4];

private:
    int m_winWidth;
    int m_winHeight;
    float m_imageAspect;

    GLuint m_imageTexID;

    GLuint m_fragShader;
    GLuint m_program;

    GLuint m_lut3dTexID;
    int LUT3D_EDGE_SIZE;
    std::vector<float> m_lut3d;
    std::string m_lut3dcacheid;
    std::string m_shadercacheid;

    std::string m_inputColorSpace;
    std::string m_display;
    std::string m_transformName;

    float m_exposure_fstop;
    float m_display_gamma;

    bool m_shaders_using_extensions;

    OCIO::ConstConfigRcPtr m_ocio_config;
    
};

#endif // GLWIDGET_H
