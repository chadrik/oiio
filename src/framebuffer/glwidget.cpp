#include <GL/glew.h>
//#include <GL/glext.h>
#include "glwidget.h"
#include <QDebug>
#include <iostream>
#include <fstream>
#include <sstream>

#include "imageio.h"
#include "imagebuf.h"

OIIO_NAMESPACE_USING;

#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;


GLWidget::GLWidget(QWidget *parent) :
    QGLWidget(parent)
{
    m_winWidth = 0;
    m_winHeight = 0;

    m_fragShader = 0;
    m_program = 0;

    LUT3D_EDGE_SIZE = 32;

    m_exposure_fstop = 0.0f;
    m_display_gamma = 1.0f;
    m_channelHot[0] = 1;
    m_channelHot[1] = 1;
    m_channelHot[2] = 1;
    m_channelHot[3] = 1;

    m_shaders_using_extensions = true;


}

void GLWidget::initializeGL()
{
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        qDebug() << "paintEvent !!! glewInit failed with: " << err;
    }
    if (!glewIsSupported("GL_VERSION_2_0"))
    {
        printf("OpenGL 2.0 not supported\n");
    }
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    AllocateLut3D();
    InitOCIO("");
    UpdateOCIOGLState();
}

void GLWidget::InitOCIO(const char * filename)
{
    std::cout << "InitOCIO" << std::endl;
    m_ocio_config = OCIO::GetCurrentConfig();
    m_display = m_ocio_config->getDefaultDisplay();
    m_transformName = m_ocio_config->getDefaultView(m_display.c_str());

    m_inputColorSpace = OCIO::ROLE_SCENE_LINEAR;
    if(filename)
    {
        std::string cs = m_ocio_config->parseColorSpaceFromString(filename);
        if(!cs.empty())
        {
            m_inputColorSpace = cs;
            std::cout << "colorspace: " << cs << std::endl;
        }
        else
        {
            std::cout << "colorspace: " << m_inputColorSpace << " \t(could not determine from filename, using default)" << std::endl;
        }
    }
}

void GLWidget::AllocateLut3D()
{
    glGenTextures(1, &m_lut3dTexID);

    int num3Dentries = 3*LUT3D_EDGE_SIZE*LUT3D_EDGE_SIZE*LUT3D_EDGE_SIZE;
    m_lut3d.resize(num3Dentries);
    memset(&m_lut3d[0], 0, sizeof(float)*num3Dentries);

    glActiveTexture(GL_TEXTURE2);

    glBindTexture(GL_TEXTURE_3D, m_lut3dTexID);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F_ARB,
                 LUT3D_EDGE_SIZE, LUT3D_EDGE_SIZE, LUT3D_EDGE_SIZE,
                 0, GL_RGB,GL_FLOAT, &m_lut3d[0]);
}

bool GLWidget::InitImageTexture(ImageBuffer &image)
{
    glGenTextures(1, &m_imageTexID);

    std::vector<float> RGBA;

    int texWidth = 0;
    int texHeight = 0;
    int components = 4;

    try
    {
        image.read(0,0,false,TypeDesc::FLOAT);
        ImageSpec spec = image.spec();

        texWidth = spec.width;
        texHeight = spec.height;
        components = 4;//spec.nchannels;
        RGBA.resize(texWidth*texHeight*components);
        memset(&RGBA[0], 1, texWidth*texHeight*components*sizeof(float));

        image.copy_pixel_channels (0, texWidth, 0, texHeight, 0, components, spec.format, &RGBA[0]);
        std::cout << "Buffer finished" <<std::endl;
    }
    catch(...)
    {
        std::cerr << "Error loading file.";
        exit(1);
    }


    GLenum format = 0;
    if(components == 4) format = GL_RGBA;
    else if(components == 3) format = GL_RGB;
    else
    {
        std::cerr << "Cannot load image with " << components << " components." << std::endl;
        //exit(1);
    }

    m_imageAspect = 1.0;
    if(texHeight!=0)
    {
        m_imageAspect = (float) texWidth / (float) texHeight;
    }

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_imageTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, texWidth, texHeight, 0, format, GL_FLOAT, &RGBA[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    UpdateOCIOGLState();
    updateGL();
    return 0;
}

GLuint
GLWidget::CompileShaderText(GLenum shaderType, const char *text)
{
    GLuint shader;
    GLint stat;

    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const GLchar **) &text, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &stat);

    if (!stat)
    {
        GLchar log[1000];
        GLsizei len;
        glGetShaderInfoLog(shader, 1000, &len, log);
        fprintf(stderr, "Error: problem compiling shader: %s\n", log);
        return 0;
    }

    return shader;
}

GLuint
GLWidget::LinkShaders(GLuint fragShader)
{
    if (!fragShader) return 0;

    GLuint program = glCreateProgram();

    if (fragShader)
        glAttachShader(program, fragShader);

    glLinkProgram(program);

    /* check link */
    {
        GLint stat;
        glGetProgramiv(program, GL_LINK_STATUS, &stat);
        if (!stat) {
            GLchar log[1000];
            GLsizei len;
            glGetProgramInfoLog(program, 1000, &len, log);
            fprintf(stderr, "Shader link error:\n%s\n", log);
            return 0;
        }
    }

    return program;
}

const char * m_fragShaderText = ""
"\n"
"uniform sampler2D tex1;\n"
"uniform sampler3D tex2;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 col = texture2D(tex1, gl_TexCoord[0].st);\n"
"    gl_FragColor = OCIODisplay(col, tex2);\n"
"}\n";

void GLWidget::UpdateOCIOGLState()
{
    // Step 0: Get the processor using any of the pipelines mentioned above.
    OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();

    OCIO::DisplayTransformRcPtr transform = OCIO::DisplayTransform::Create();
    transform->setInputColorSpaceName( m_inputColorSpace.c_str() );
    transform->setDisplay( m_display.c_str() );
    transform->setView( m_transformName.c_str() );

    // Add optional transforms to create a full-featured, "canonical" display pipeline
    // Fstop exposure control (in SCENE_LINEAR)
    {
        float gain = powf(2.0f, m_exposure_fstop);
        const float slope4f[] = { gain, gain, gain, gain };
        float m44[16];
        float offset4[4];
        OCIO::MatrixTransform::Scale(m44, offset4, slope4f);
        OCIO::MatrixTransformRcPtr mtx =  OCIO::MatrixTransform::Create();
        mtx->setValue(m44, offset4);
        transform->setLinearCC(mtx);
    }

    // Channel swizzling
    {
        float lumacoef[3];
        config->getDefaultLumaCoefs(lumacoef);
        float m44[16];
        float offset[4];
        OCIO::MatrixTransform::View(m44, offset, m_channelHot, lumacoef);
        OCIO::MatrixTransformRcPtr swizzle = OCIO::MatrixTransform::Create();
        swizzle->setValue(m44, offset);
        transform->setChannelView(swizzle);
    }

    // Post-display transform gamma
    {
        float exponent = 1.0f/std::max(1e-6f, static_cast<float>(m_display_gamma));
        const float exponent4f[] = { exponent, exponent, exponent, exponent };
        OCIO::ExponentTransformRcPtr expTransform =  OCIO::ExponentTransform::Create();
        expTransform->setValue(exponent4f);
        transform->setDisplayCC(expTransform);
    }

    OCIO::ConstProcessorRcPtr processor;
    try
    {
        processor = config->getProcessor(transform);
    }
    catch(OCIO::Exception & e)
    {
        std::cerr << e.what() << std::endl;
        return;
    }
    catch(...)
    {
        return;
    }

    // Step 1: Create a GPU Shader Description
    OCIO::GpuShaderDesc shaderDesc;
    shaderDesc.setLanguage(OCIO::GPU_LANGUAGE_GLSL_1_0);
    shaderDesc.setFunctionName("OCIODisplay");
    shaderDesc.setLut3DEdgeLen(LUT3D_EDGE_SIZE);

    // Step 2: Compute the 3D LUT
    std::string lut3dCacheID = processor->getGpuLut3DCacheID(shaderDesc);
    if(lut3dCacheID != m_lut3dcacheid)
    {
        std::cerr << "Computing 3DLut " << m_lut3dcacheid << std::endl;

        m_lut3dcacheid = lut3dCacheID;
        processor->getGpuLut3D(&m_lut3d[0], shaderDesc);

        glBindTexture(GL_TEXTURE_3D, m_lut3dTexID);
        glTexSubImage3D(GL_TEXTURE_3D, 0,
                        0, 0, 0,
                        LUT3D_EDGE_SIZE, LUT3D_EDGE_SIZE, LUT3D_EDGE_SIZE,
                        GL_RGB,GL_FLOAT, &m_lut3d[0]);
    }

    // Step 3: Compute the Shader
    std::string shaderCacheID = processor->getGpuShaderTextCacheID(shaderDesc);
    if(m_program == 0 || shaderCacheID != m_shadercacheid)
    {
        //std::cerr << "Computing Shader " << m_shadercacheid << std::endl;

        m_shadercacheid = shaderCacheID;

        std::ostringstream os;
        os << processor->getGpuShaderText(shaderDesc) << "\n";
        os << m_fragShaderText;
        //std::cerr << os.str() << std::endl;

        if(m_fragShader) glDeleteShader(m_fragShader);
        m_fragShader = CompileShaderText(GL_FRAGMENT_SHADER, os.str().c_str());
        if(m_program) glDeleteProgram(m_program);
        m_program = LinkShaders(m_fragShader);
    }

    glUseProgram(m_program);
    glUniform1i(glGetUniformLocation(m_program, "tex1"), 1);
    glUniform1i(glGetUniformLocation(m_program, "tex2"), 2);
    swapBuffers();
}

void GLWidget::paintGL()
{
    float windowAspect = 1.0;
    if(m_winHeight != 0)
    {
        windowAspect = (float)m_winWidth/(float)m_winHeight;
    }

    float pts[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // x0,y0,x1,y1
    if(windowAspect>m_imageAspect)
    {
        float imgWidthScreenSpace = m_imageAspect * (float)m_winHeight;
        pts[0] = (float)m_winWidth * 0.5f - (float)imgWidthScreenSpace * 0.5f;
        pts[2] = (float)m_winWidth * 0.5f + (float)imgWidthScreenSpace * 0.5f;
        pts[1] = 0.0f;
        pts[3] = (float)m_winHeight;
    }
    else
    {
        float imgHeightScreenSpace = (float)m_winWidth / m_imageAspect;
        pts[0] = 0.0f;
        pts[2] = (float)m_winWidth;
        pts[1] = (float)m_winHeight * 0.5f - imgHeightScreenSpace * 0.5f;
        pts[3] = (float)m_winHeight * 0.5f + imgHeightScreenSpace * 0.5f;
    }

    glEnable(GL_TEXTURE_2D);
    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1, 1, 1);

    glPushMatrix();
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(pts[0], pts[1]);

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(pts[0], pts[3]);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(pts[2], pts[3]);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(pts[2], pts[1]);

    glEnd();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

void GLWidget::resizeGL(int w, int h) {
    m_winWidth = w;
    m_winHeight = h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, m_winWidth, 0.0, m_winHeight, -100.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void GLWidget::exposure_CB(float Ev)
{
    m_exposure_fstop = Ev;
    UpdateOCIOGLState();
    updateGL();
}

void GLWidget::gamma_CB(float Y)
{
    m_display_gamma = Y;
    UpdateOCIOGLState();
    updateGL();
}

void GLWidget::inSpace_CB(int index)
{

    OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();
    std::string name = config->getColorSpaceNameByIndex(index);
    std::cout << "InSpace: " << name.c_str() << std::endl;

    m_inputColorSpace = name;

    UpdateOCIOGLState();
    updateGL();
}

void GLWidget::outSpace_CB(int index)
{
    OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();
    const char * defaultDisplay = config->getDefaultDisplay();
    m_transformName = config->getView(defaultDisplay, index);

    UpdateOCIOGLState();
    updateGL();
}
