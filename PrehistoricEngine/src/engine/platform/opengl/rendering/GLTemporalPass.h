#pragma once
#include "platform/opengl/rendering/shaders/GLShader.h"

namespace Prehistoric {
    class GLRenderer;

    class GLTemporalPass {
    public:
        GLTemporalPass();

        ~GLTemporalPass();

        void Render(GLRenderer &renderer, Texture *scene);

    private:
        void Release();

        unsigned width = 0, height = 0, samples = 0;
        GLenum motionFormat = GL_RGBA32F;
        GLuint queries[3][8]{};
        bool queried[3]{};
        GLuint textures[8]{}, frames[8]{}, depth{}, vao{}, uniformBuffer{}, copyFrame{};
        std::unique_ptr<GLShader> velocity, resolve, filter;
    };
} // namespace Prehistoric