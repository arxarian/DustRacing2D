// This file belongs to the "MiniCore" game engine.
// Copyright (C) 2010 Jussi Lind <jussi.lind@iki.fi>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA  02110-1301, USA.
//

#include "mcsurface.hh"
#include "mccamera.hh"
#include "mcbbox.hh"
#include "mcglshaderprogram.hh"
#include "mcglvertex.hh"
#include "mcgltexcoord.hh"
#include "mctrigonom.hh"

#include <cassert>

#include <GL/gl.h>
#include <GL/glext.h>

static const int gNumVertices           = 6;
static const int gNumVertexComponents   = 3;
static const int gNumColorComponents    = 4;
static const int gNumTexCoordComponents = 2;

MCSurface::MCSurface(GLuint handle, MCFloat width, MCFloat height)
: m_handle(handle)
, m_w(width)
, m_w2(width / 2)
, m_h(height)
, m_h2(height / 2)
, m_center(m_w2, m_h2)
, m_centerSet(false)
, m_useAlphaTest(false)
, m_alphaFunc(GL_ALWAYS)
, m_alphaThreshold(0.0)
, m_useAlphaBlend(false)
, m_src(GL_SRC_ALPHA)
, m_dst(GL_ONE_MINUS_SRC_ALPHA)
, m_program(nullptr)
, m_shadowProgram(nullptr)
{
    // Init vertice data for two triangles.
    const MCGLVertex vertices[gNumVertices] =
    {
        {-(GLfloat)m_w2, -(GLfloat)m_h2, 0},
        {-(GLfloat)m_w2,  (GLfloat)m_h2, 0},
        { (GLfloat)m_w2,  (GLfloat)m_h2, 0},
        {-(GLfloat)m_w2, -(GLfloat)m_h2, 0},
        { (GLfloat)m_w2,  (GLfloat)m_h2, 0},
        { (GLfloat)m_w2, -(GLfloat)m_h2, 0}
    };

    const MCGLVertex normals[gNumVertices] =
    {
        {0, 0, 1},
        {0, 0, 1},
        {0, 0, 1},
        {0, 0, 1},
        {0, 0, 1},
        {0, 0, 1}
    };

    const MCGLTexCoord texCoords[gNumVertices] =
    {
        {0, 0},
        {0, 1},
        {1, 1},
        {0, 0},
        {1, 1},
        {1, 0}
    };

    const GLfloat colors[gNumVertices * gNumColorComponents] =
    {
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1
    };

    initVBOs(vertices, normals, texCoords, colors);
}

MCSurface::MCSurface(GLuint handle, MCFloat width, MCFloat height, const MCGLTexCoord texCoords[4])
: m_handle(handle)
, m_w(width)
, m_w2(width / 2)
, m_h(height)
, m_h2(height / 2)
, m_center(m_w2, m_h2)
, m_centerSet(false)
, m_useAlphaTest(false)
, m_alphaFunc(GL_ALWAYS)
, m_alphaThreshold(0.0)
, m_useAlphaBlend(false)
, m_src(GL_SRC_ALPHA)
, m_dst(GL_ONE_MINUS_SRC_ALPHA)
, m_program(nullptr)
, m_shadowProgram(nullptr)
{
    // Init vertice data for two triangles.
    const MCGLVertex vertices[gNumVertices] =
    {
        {-(GLfloat)m_w2, -(GLfloat)m_h2, 0},
        {-(GLfloat)m_w2,  (GLfloat)m_h2, 0},
        { (GLfloat)m_w2,  (GLfloat)m_h2, 0},
        {-(GLfloat)m_w2, -(GLfloat)m_h2, 0},
        { (GLfloat)m_w2,  (GLfloat)m_h2, 0},
        { (GLfloat)m_w2, -(GLfloat)m_h2, 0}
    };

    const MCGLVertex normals[gNumVertices] =
    {
        {0, 0, 1},
        {0, 0, 1},
        {0, 0, 1},
        {0, 0, 1},
        {0, 0, 1},
        {0, 0, 1}
    };

    const GLfloat colors[gNumVertices * gNumColorComponents] =
    {
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1
    };

    initVBOs(vertices, normals, texCoords, colors);
}

void MCSurface::initVBOs(
    const MCGLVertex   * vertices,
    const MCGLVertex   * normals,
    const MCGLTexCoord * texCoords,
    const GLfloat      * colors)
{
    glGenBuffers(VBOTypes, m_vbos);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbos[VBOVertex]);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(MCGLVertex) * gNumVertices, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbos[VBONormal]);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(MCGLVertex) * gNumVertices, normals, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbos[VBOTexture]);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(MCGLTexCoord) * gNumVertices, texCoords, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbos[VBOColor]);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(GLfloat) * gNumVertices * gNumColorComponents,
        colors, GL_DYNAMIC_DRAW);
}

MCSurface::~MCSurface()
{
    glDeleteBuffers(VBOTypes, m_vbos);
}

void MCSurface::setCenter(MCVector2dFR center)
{
    m_centerSet = true;
    m_center    = center;
}

void MCSurface::setAlphaTest(
    bool useAlphaTest, GLenum alphaFunc, GLclampf threshold)
{
    m_useAlphaTest   = useAlphaTest;
    m_alphaFunc      = alphaFunc;
    m_alphaThreshold = threshold;
}

void MCSurface::setAlphaBlend(
    bool useAlphaBlend, GLenum src, GLenum dst)
{
    m_useAlphaBlend  = useAlphaBlend;
    m_src            = src;
    m_dst            = dst;
}

void MCSurface::doAlphaTest() const
{
    if (m_useAlphaTest)
    {
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(m_alphaFunc, m_alphaThreshold);
    }
}

void MCSurface::doAlphaBlend() const
{
    if (m_useAlphaBlend)
    {
        glEnable(GL_BLEND);
        glBlendFunc(m_src, m_dst);
    }
}

void MCSurface::setTexCoords(const MCGLTexCoord texCoords[4])
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vbos[VBOTexture]);

    // This should make things a bit faster especially on NVIDIA.
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(MCGLTexCoord) * gNumVertices, nullptr, GL_DYNAMIC_DRAW);

    MCGLTexCoord * pTexData = (MCGLTexCoord *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (pTexData)
    {
        pTexData[0] = texCoords[0];
        pTexData[1] = texCoords[1];
        pTexData[2] = texCoords[2];
        pTexData[3] = texCoords[0];
        pTexData[4] = texCoords[2];
        pTexData[5] = texCoords[3];

        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
}

void MCSurface::setColor(MCFloat r, MCFloat g, MCFloat b, MCFloat a)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vbos[VBOColor]);

    // This should make things a bit faster especially on NVIDIA.
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(GLfloat) * gNumVertices * gNumColorComponents, nullptr, GL_DYNAMIC_DRAW);

    GLfloat * pColorData = (GLfloat *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (pColorData)
    {
        for (int i = 0; i < gNumVertices; i++)
        {
            const int offset = (i << 2);
            pColorData[offset + 0] = r;
            pColorData[offset + 1] = g;
            pColorData[offset + 2] = b;
            pColorData[offset + 3] = a;
        }

        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
}

MCBBox<MCFloat> MCSurface::rotatedBBox(MCVector2dFR pos, MCFloat angle)
{
    using std::abs;

    const MCFloat cos = MCTrigonom::cos(angle);
    const MCFloat sin = MCTrigonom::sin(angle);
    const MCFloat  w1 = (abs(cos * m_w2) + abs(sin * m_h2)) / 2;
    const MCFloat  h1 = (abs(sin * m_w2) + abs(cos * m_h2)) / 2;

    return MCBBox<MCFloat>(pos.i() - w1, pos.j() - h1, pos.i() + w1, pos.j() + h1);
}

MCBBox<MCFloat> MCSurface::rotatedScaledBBox(
    MCVector2dFR pos, MCFloat angle, MCFloat w2, MCFloat h2)
{
    using std::abs;

    const MCFloat cos = MCTrigonom::cos(angle);
    const MCFloat sin = MCTrigonom::sin(angle);
    const MCFloat  w1 = (abs(cos * w2) + abs(sin * h2)) / 2;
    const MCFloat  h1 = (abs(sin * w2) + abs(cos * h2)) / 2;

    return MCBBox<MCFloat>(pos.i() - w1, pos.j() - h1, pos.i() + w1, pos.j() + h1);
}

void MCSurface::renderVBOs(bool autoClientState)
{
    if (autoClientState)
    {
        enableClientState(true);
    }

    glDrawArrays(GL_TRIANGLES, 0, gNumVertices);

    if (autoClientState)
    {
        enableClientState(false);
    }
}

void MCSurface::bindTexture() const
{
    glBindTexture(GL_TEXTURE_2D, m_handle);
}

void MCSurface::setShaderProgram(MCGLShaderProgram & program)
{
    m_program = &program;
}

void MCSurface::setShadowShaderProgram(MCGLShaderProgram & program)
{
    m_shadowProgram = &program;
}

void MCSurface::render(MCCamera * pCamera, MCVector3dFR pos, MCFloat angle,
    bool autoClientState)
{
    if (m_program)
    {
        MCFloat x = pos.i();
        MCFloat y = pos.j();
        MCFloat z = pos.k();

        if (pCamera)
        {
            pCamera->mapToCamera(x, y);
        }

        m_program->bind();
        m_program->setScale(1.0, 1.0, 1.0);

        if (m_centerSet)
        {
            m_program->translate(MCVector3dF(x + m_w2 - m_center.i(), y + m_h2 - m_center.j(), z));
        }
        else
        {
            m_program->translate(MCVector3dF(x, y, z));
        }

        m_program->rotate(angle);

        renderVBOs(autoClientState);

        m_program->release();
    }
}

void MCSurface::renderScaled(
    MCCamera * pCamera, MCVector3dFR pos, MCFloat wr, MCFloat hr, MCFloat angle,
    bool autoClientState)
{
    if (m_program)
    {
        MCFloat x = pos.i();
        MCFloat y = pos.j();
        MCFloat z = pos.k();

        if (pCamera)
        {
            pCamera->mapToCamera(x, y);
        }

        m_program->bind();
        m_program->setScale(wr / m_w2, hr / m_h2, 1.0);

        if (m_centerSet)
        {
            m_program->translate(MCVector3dF(x + m_w2 - m_center.i(), y + m_h2 - m_center.j(), z));
        }
        else
        {
            m_program->translate(MCVector3dF(x, y, z));
        }

        m_program->rotate(angle);

        renderVBOs(autoClientState);

        m_program->release();
    }
}

void MCSurface::renderShadow(MCCamera * pCamera, MCVector2dFR pos, MCFloat angle,
    bool autoClientState)
{
    if (m_shadowProgram)
    {
        MCFloat x = pos.i();
        MCFloat y = pos.j();
        MCFloat z = 0;

        if (pCamera)
        {
            pCamera->mapToCamera(x, y);
        }

        m_shadowProgram->bind();
        m_shadowProgram->setScale(1.0, 1.0, 1.0);

        if (m_centerSet)
        {
            m_shadowProgram->translate(MCVector3dF(x + m_w2 - m_center.i(), y + m_h2 - m_center.j(), z));
        }
        else
        {
            m_shadowProgram->translate(MCVector3dF(x, y, z));
        }

        m_shadowProgram->rotate(angle);

        renderVBOs(autoClientState);

        m_shadowProgram->release();
    }
}

void MCSurface::renderShadowScaled(
    MCCamera * pCamera, MCVector2dFR pos, MCFloat wr, MCFloat hr, MCFloat angle,
    bool autoClientState)
{
    if (m_shadowProgram)
    {
        MCFloat x = pos.i();
        MCFloat y = pos.j();
        MCFloat z = 0;

        if (pCamera)
        {
            pCamera->mapToCamera(x, y);
        }

        m_shadowProgram->bind();
        m_shadowProgram->setScale(wr / m_w2, hr / m_h2, 1.0);

        if (m_centerSet)
        {
            m_shadowProgram->translate(MCVector3dF(x + m_w2 - m_center.i(), y + m_h2 - m_center.j(), z));
        }
        else
        {
            m_shadowProgram->translate(MCVector3dF(x, y, z));
        }

        m_shadowProgram->rotate(angle);

        renderVBOs(autoClientState);

        m_shadowProgram->release();
    }
}

void MCSurface::enableClientState(bool enable, bool bindTexture) const
{
    if (enable)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbos[VBOVertex]);
        glVertexPointer(gNumVertexComponents, GL_FLOAT, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbos[VBONormal]);
        glNormalPointer(GL_FLOAT, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbos[VBOTexture]);
        glTexCoordPointer(gNumTexCoordComponents, GL_FLOAT, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbos[VBOColor]);
        glColorPointer(gNumColorComponents, GL_FLOAT, 0, 0);

        if (bindTexture)
        {
            this->bindTexture();
        }
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
    }
}

GLuint MCSurface::handle() const
{
    return m_handle;
}

MCFloat MCSurface::width() const
{
    return m_w;
}

MCFloat MCSurface::height() const
{
    return m_h;
}

MCVector2dF MCSurface::center() const
{
    return m_center;
}