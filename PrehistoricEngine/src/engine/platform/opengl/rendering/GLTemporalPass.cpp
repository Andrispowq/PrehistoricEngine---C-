#include "Includes.hpp"
#include "GLTemporalPass.h"
#include "GLRenderer.h"
#include "prehistoric/application/Application.h"
#include "platform/opengl/rendering/pipeline/GLGraphicsPipeline.h"
#include "prehistoric/core/node/component/renderer/RenderableComponent.h"

namespace Prehistoric {
    static std::unique_ptr<GLShader> TemporalShader(const char *vertex, const char *fragment) {
        auto shader = std::make_unique<GLShader>();
        shader->AddShader(ResourceLoader::LoadShaderGL(vertex), VERTEX_SHADER);
        shader->AddShader(ResourceLoader::LoadShaderGL(fragment), FRAGMENT_SHADER);
        if (!shader->CompileShader())
            throw std::runtime_error("Temporal shader compilation failed");
        return shader;
    }

    GLTemporalPass::GLTemporalPass() {
        velocity = TemporalShader("opengl/temporal/velocity.vert", "opengl/temporal/velocity.frag");
        velocity->AddUniform("current");
        velocity->AddUniform("previous");
        filter = TemporalShader("opengl/temporal/fullscreen.vert", "opengl/temporal/filter.frag");
        filter->AddUniform("mode");
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &uniformBuffer);
        glGenFramebuffers(1, &copyFrame);
        glGenQueries(24, &queries[0][0]);
    }

    void GLTemporalPass::Release() {
        glDeleteFramebuffers(8, frames);
        glDeleteTextures(8, textures);
        glDeleteRenderbuffers(1, &depth);
        std::fill_n(frames, 8, 0);
        std::fill_n(textures, 8, 0);
        depth = 0;
    }

    GLTemporalPass::~GLTemporalPass() {
        Release();
        glDeleteFramebuffers(1, &copyFrame);
        glDeleteQueries(24, &queries[0][0]);
        glDeleteBuffers(1, &uniformBuffer);
        glDeleteVertexArrays(1, &vao);
    }

    void GLTemporalPass::Render(GLRenderer &renderer, Texture *scene) {
        renderer.temporalRendered = true;
        auto &history = renderer.temporalHistory;
        const unsigned w = renderer.getWindow()->getWidth(), h = renderer.getWindow()->getHeight();
        const unsigned count = std::max(1u, __FrameworkConfig.windowNumSamples);
        GLint oldFrame, oldReadFrame, oldProgram, oldVAO, oldFront, oldCull, oldActive, oldUniform, viewport[4];
        glGetIntegerv(GL_ACTIVE_TEXTURE, &oldActive);
        glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, 6, &oldUniform);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldReadFrame);
        glGetIntegerv(GL_FRONT_FACE, &oldFront);
        glGetIntegerv(GL_CULL_FACE_MODE, &oldCull);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldFrame);
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVAO);
        glGetIntegerv(GL_VIEWPORT, viewport);
        const bool depthTest = glIsEnabled(GL_DEPTH_TEST), blend = glIsEnabled(GL_BLEND),
                cull = glIsEnabled(GL_CULL_FACE);
        const bool scissor = glIsEnabled(GL_SCISSOR_TEST);
        glDisable(GL_SCISSOR_TEST);
        GLboolean depthWrite;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
        GLint depthFunc;
        glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
        if (width != w || height != h || samples != count) {
            Release();
            width = w;
            height = h;
            samples = count;
            history.valid = false;
            resolve = TemporalShader("opengl/temporal/fullscreen.vert",
                                     count > 1 ? "opengl/temporal/resolve_ms.frag" : "opengl/temporal/resolve.frag");
            if (count > 1)
                resolve->AddUniform("rasterSamples");
            motionFormat = GL_RGBA32F;
            if (count > 1) {
                GLint number = 0;
                glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_RGBA32F, GL_NUM_SAMPLE_COUNTS, 1, &number);
                std::vector<GLint> supported(std::max(0, number));
                if (number)
                    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_RGBA32F, GL_SAMPLES, number, supported.data());
                if (std::find(supported.begin(), supported.end(), GLint(count)) == supported.end())
                    motionFormat = GL_RGBA16F;
            }
            glGenTextures(8, textures);
            glGenFramebuffers(8, frames);
            for (int i = 0; i < 8; ++i) {
                const unsigned tw = i == 6 || i == 7 ? (w + 15) / 16 : w;
                const unsigned th = i == 6 || i == 7 ? (h + 15) / 16 : h;
                const bool ms = i == 0 && count > 1;
                GLenum target = ms ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
                glBindTexture(target, textures[i]);
                // Motion/depth metadata uses 32-bit floats to preserve distant depth tests.
                const GLenum format = i == 0 ? motionFormat : i <= 2 ? GL_RGBA32F : GL_RGBA16F;
                if (ms)
                    glTexImage2DMultisample(target, count, format, tw, th, GL_TRUE);
                else {
                    glTexImage2D(target, 0, format, tw, th, 0, GL_RGBA, GL_FLOAT, nullptr);
                    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, i == 3 || i == 4 || i == 5 ? GL_LINEAR : GL_NEAREST);
                    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, i == 3 || i == 4 || i == 5 ? GL_LINEAR : GL_NEAREST);
                    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                }
                glBindFramebuffer(GL_FRAMEBUFFER, frames[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, textures[i], 0);
                if (i != 0) {
                    float zero[4]{};
                    glClearBufferfv(GL_COLOR, 0, zero);
                }
            }
            glGenRenderbuffers(1, &depth);
            glBindRenderbuffer(GL_RENDERBUFFER, depth);
            if (count > 1)
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, count, GL_DEPTH_COMPONENT32F, w, h);
            else
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, w, h);
            glBindFramebuffer(GL_FRAMEBUFFER, frames[0]);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                throw std::runtime_error("Temporal framebuffer incomplete");
        }
        renderer.temporalBytes = uint64_t(w) * h * ((motionFormat == GL_RGBA32F ? 20 : 12) * count + 56) +
                                 uint64_t((w + 15) / 16) * ((h + 15) / 16) * 16;
        const unsigned timerSlot = unsigned(history.frame % 3);
        if (queried[timerSlot]) {
            GLint ready = 0;
            glGetQueryObjectiv(queries[timerSlot][7], GL_QUERY_RESULT_AVAILABLE, &ready);
            if (ready)
                for (int i = 0; i < 4; ++i) {
                    GLuint64 start = 0, end = 0;
                    glGetQueryObjectui64v(queries[timerSlot][2 * i], GL_QUERY_RESULT, &start);
                    glGetQueryObjectui64v(queries[timerSlot][2 * i + 1], GL_QUERY_RESULT, &end);
                    renderer.temporalMilliseconds[i] = double(end - start) * 1e-6;
                }
        }
        queried[timerSlot] = true;
        auto stamp = [&](int i) { glQueryCounter(queries[timerSlot][i], GL_TIMESTAMP); };
        stamp(0);
        auto params = renderer.GetTemporalParameters();
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(params), &params, GL_STREAM_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 6, uniformBuffer);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glBindFramebuffer(GL_FRAMEBUFFER, frames[0]);
        glViewport(0, 0, w, h);
        float zero[4]{};
        glClearBufferfv(GL_COLOR, 0, zero);
        glClearDepth(1);
        glClear(GL_DEPTH_BUFFER_BIT);
        velocity->Bind(nullptr);
        VertexBuffer *vertices = nullptr;
        for (const auto *batches: {&renderer.getModels3D(), &renderer.getModelsTransparency()})
            Renderer::VisitBatches(
                *batches,
                [&](Pipeline *pipeline) {
                    auto *graphics = static_cast<GLGraphicsPipeline *>(pipeline);
                    vertices = graphics->getVertexBuffer();
                    if (vertices->getType() != VertexBufferType::MESH) {
                        vertices = nullptr;
                        return;
                    }
                    vertices->Bind(nullptr);
                    if (!graphics->isBackfaceCulling() || graphics->getFrontFace() == FrontFace::DOUBLE_SIDED)
                        glDisable(GL_CULL_FACE);
                    else {
                        glEnable(GL_CULL_FACE);
                        glCullFace(GL_BACK);
                        glFrontFace(graphics->getFrontFace() == FrontFace::CLOCKWISE ? GL_CW : GL_CCW);
                    }
                },
                [](Pipeline *, Material *) {
                },
                [&](Pipeline *, RenderableComponent *object) {
                    if (!vertices)
                        return;
                    const auto &model = history.pendingModels.at(object->getTemporalIdentity());
                    const auto old = history.models.find(object->getTemporalIdentity());
                    const bool valid = history.CanReproject(object->getTemporalIdentity(), model) &&
                                       object->getPriority() == RenderPriority::_3D;
                    velocity->SetUniform("current", history.current * model);
                    velocity->SetUniform("previous", valid ? history.previous * old->second : Matrix4f::Zero());
                    for (unsigned i = 0; i < vertices->getSubmeshCount(); ++i)
                        vertices->Draw(nullptr, i);
                },
                [&](Pipeline *) {
                    if (vertices)
                        vertices->Unbind();
                });
        stamp(1);
        stamp(2);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glBindVertexArray(vao);
        const int write = int(history.frame % 2), read = 1 - write;
        auto bind = [&](unsigned unit, GLuint tex) {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, tex);
        };
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(count > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, textures[0]);
        renderer.getMainPass()->getPositionMetallicImage()->Bind(5);
        glBindFramebuffer(GL_FRAMEBUFFER, frames[1 + write]);
        resolve->Bind(nullptr);
        if (count > 1)
            resolve->SetUniformi("rasterSamples", int(count));
        glDrawArrays(GL_TRIANGLES, 0, 3);
        stamp(3);
        stamp(4);
        bind(1, textures[1 + write]);
        bind(2, textures[3 + read]);
        bind(3, textures[1 + read]);
        auto source = static_cast<GLTexture *>(scene)->getTextureID();
        auto draw = [&](int target, int mode, unsigned tw, unsigned th) {
            glBindFramebuffer(GL_FRAMEBUFFER, frames[target]);
            glViewport(0, 0, tw, th);
            filter->Bind(nullptr);
            filter->SetUniformi("mode", mode);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        };
        bind(0, source);
        if (renderer.temporal.antialiasing || renderer.temporal.debug >= 3) {
            draw(3 + write, 0, w, h);
            source = textures[3 + write];
            bind(0, source);
        }
        stamp(5);
        stamp(6);
        if (renderer.temporal.debug >= 3) {
            bind(0, static_cast<GLTexture *>(scene)->getTextureID());
            draw(5, 4, w, h);
            source = textures[5];
        } else if (renderer.temporal.motionBlur || renderer.temporal.debug == 1 || renderer.temporal.debug == 2) {
            draw(6, 2, (w + 15) / 16, (h + 15) / 16);
            bind(4, textures[6]);
            draw(7, 3, (w + 15) / 16, (h + 15) / 16);
            bind(4, textures[7]);
            draw(5, 1, w, h);
            source = textures[5];
        }
        stamp(7);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, copyFrame);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, source, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frames[0]);
        // Attach the original HDR colour only to a temporary destination, never a history texture.
        GLuint destination;
        glGenFramebuffers(1, &destination);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               static_cast<GLTexture *>(scene)->getTextureID(), 0);
        if (source != static_cast<GLTexture *>(scene)->getTextureID())
            glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glDeleteFramebuffers(1, &destination);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldFrame);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, oldReadFrame);
        glFrontFace(oldFront);
        glCullFace(oldCull);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glUseProgram(oldProgram);
        glBindVertexArray(oldVAO);
        if (depthTest)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
        if (blend)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);
        if (cull)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);
        if (scissor) glEnable(GL_SCISSOR_TEST);
        else glDisable(GL_SCISSOR_TEST);
        glDepthMask(depthWrite);
        glDepthFunc(depthFunc);
        glBindBufferBase(GL_UNIFORM_BUFFER, 6, oldUniform);
        glActiveTexture(oldActive);
    }
} // namespace Prehistoric