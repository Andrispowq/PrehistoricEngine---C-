#ifndef GL_VOXEL_RENDERER_PASS_H
#define GL_VOXEL_RENDERER_PASS_H

#include "prehistoric/common/rendering/renderStage/RenderStage.h"
#include "platform/opengl/rendering/shaders/GLShader.h"

namespace Prehistoric {
    class GLVoxelRendererPass : public RenderStage {
    public:
        struct Settings {
            bool debugView = false;
            bool globalIllumination = false;
            bool voxelSunShadows = false;
            bool halfResolution = false;
            int resolutionIndex = 1;
            Vector3f center = Vector3f(0);
            float extent = 400.0f;
            float giStrength = 1.0f;
            int traceDepth = 1;
            bool directionalMips = false;
        };

        static constexpr int Resolutions[] = {64, 128, 256, 512, 1024};
        static constexpr int MaxTraceDepth = 5;

        enum class GpuStage { Occluders, Injection, Propagation, RadianceMips, Trace, Composite, Count };

        double GetGpuMilliseconds(GpuStage stage) const;

        explicit GLVoxelRendererPass(Renderer *renderer);

        ~GLVoxelRendererPass() override;

        void OnResized() override {
        }

        void Render() override;

        void RenderScreenReflections(Texture *output);

        void Composite(Texture *output);

        bool IsSupported() const { return GLAD_GL_VERSION_4_5; }
        bool IsReady() const { return ready; }
        Settings &GetSettings() { return settings; }

        bool CanUseResolution(int index) const;

        bool HasOccluders() const { return usesOccluders; }
        bool UsesVoxelSunShadows() const { return HasOccluders() && settings.voxelSunShadows; }

        void BindOccluders(const GLShader &shader, uint32_t textureUnit) const;

    private:
        bool Initialise();

        void SetResolution(int size);

        GLuint GetPackedVolume(GLuint &texture, bool mipmaps = false);

        void RenderVolume(bool debugView);

        void GenerateOccluderMips() const;

        void AllocateIndirectImage(uint32_t width, uint32_t height);

        void AllocateRadianceVolume(GLuint &texture, int size);

        void GenerateRadianceMips(GLuint texture);

        void BindDirectionalMips(const GLShader &shader, bool enabled) const;

        void ReleaseDirectionalMips();

        void PropagateBounces(int traceDepth);

        void ReleaseBounceVolumes();

        Matrix4f RenderSunShadow(const Vector3f &direction);

        struct BakeCache;
        std::unique_ptr<BakeCache> cache;

        Settings settings;
        bool initialisationAttempted = false;
        bool ready = false;
        bool usesOccluders = false;
        mutable uint64_t voxelBudget = 0;
        mutable GLint maxVolumeSize = 0;
        int resolution = 0;
        GLuint framebuffer = 0;
        GLuint debugVolume = 0;
        GLuint packedRadiance = 0;
        GLuint radianceVolume = 0;
        GLuint bounceSurfaces = 0;
        GLuint bounceVolumes[2] = {};
        std::unique_ptr<GLShader> screenShader;
        GLuint screenColour = 0, screenConfidence = 0;
        uint32_t screenWidth = 0, screenHeight = 0;
        GLuint indirectImage = 0;
        uint32_t indirectWidth = 0;
        uint32_t indirectHeight = 0;
        GLuint shadowFramebuffer = 0;
        GLuint shadowDepth = 0;
        ShaderHandle voxelisationShader;
        std::unique_ptr<GLShader> shadowShader;
        std::unique_ptr<GLShader> resolveShader;
        std::unique_ptr<GLShader> mipShader;
        std::unique_ptr<GLShader> occluderMipShader;
        std::unique_ptr<GLShader> bounceShader;
        std::unique_ptr<GLShader> compositeShader;
        std::unique_ptr<GLShader> filterShader;
    };
}

#endif