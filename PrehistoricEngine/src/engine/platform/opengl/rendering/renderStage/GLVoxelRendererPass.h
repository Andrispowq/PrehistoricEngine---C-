#ifndef GL_VOXEL_RENDERER_PASS_H
#define GL_VOXEL_RENDERER_PASS_H

#include "prehistoric/common/rendering/renderStage/RenderStage.h"
#include "platform/opengl/rendering/shaders/GLShader.h"

namespace Prehistoric
{
    class GLVoxelRendererPass : public RenderStage
    {
    public:
        struct Settings
        {
            bool debugView = false;
            bool globalIllumination = false;
            int resolutionIndex = 1;
            Vector3f center = Vector3f(0);
            float extent = 400.0f;
            float giStrength = 1.0f;
            int traceDepth = 1;
        };

        static constexpr int Resolutions[] = {64, 128, 256};
        static constexpr int MaxTraceDepth = 5;

        explicit GLVoxelRendererPass(Renderer* renderer);
        ~GLVoxelRendererPass() override;

        void OnResized() override {}
        void Render() override;
        void Composite(Texture* output) const;

        bool IsSupported() const { return GLAD_GL_VERSION_4_5; }
        bool IsReady() const { return ready; }
        Settings& GetSettings() { return settings; }

    private:
        bool Initialise();
        void SetResolution(int size);
        GLuint GetPackedVolume(GLuint& texture);
        void AllocateRadianceVolume(GLuint& texture);
        void GenerateRadianceMips(GLuint texture) const;
        void PropagateBounces(int traceDepth);
        void ReleaseBounceVolumes();
        Matrix4f RenderSunShadow(const Vector3f& direction);

        struct BakeCache;
        std::unique_ptr<BakeCache> cache;

        Settings settings;
        bool initialisationAttempted = false;
        bool ready = false;
        int resolution = 0;
        GLuint framebuffer = 0;
        GLuint debugVolume = 0;
        GLuint packedRadiance = 0;
        GLuint radianceVolume = 0;
        GLuint bounceSurfaces = 0;
        GLuint bounceVolumes[2] = {};
        GLuint shadowFramebuffer = 0;
        GLuint shadowDepth = 0;
        ShaderHandle voxelisationShader;
        std::unique_ptr<GLShader> shadowShader;
        std::unique_ptr<GLShader> resolveShader;
        std::unique_ptr<GLShader> mipShader;
        std::unique_ptr<GLShader> bounceShader;
        std::unique_ptr<GLShader> compositeShader;
    };
}

#endif
