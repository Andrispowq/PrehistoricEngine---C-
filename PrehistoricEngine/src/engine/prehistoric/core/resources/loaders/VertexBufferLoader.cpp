#include "Includes.hpp"
#include "VertexBufferLoader.h"

namespace Prehistoric
{
    void* VertexBufferLoader::LoadResourceInternal(const std::string& path, Extra* extra)
    {
        VertexBufferLoaderExtra* ext = dynamic_cast<VertexBufferLoaderExtra*>(extra);

        VertexBuffer* ret;
        switch (ext->type)
        {
        case Type::Mesh:
            if (ext->mesh.has_value())
            {
                if (FrameworkConfig::api == OpenGL)
                {
                    ret = new GLMeshVertexBuffer(window, ext->mesh.value());
                }
                else
                {
                    ret = new VKMeshVertexBuffer(window, ext->mesh.value());
                }
            }
            else
            {
                Mesh mesh = OBJLoader::LoadMesh(path, "", "");
                if (FrameworkConfig::api == OpenGL)
                {
                    ret = new GLMeshVertexBuffer(window, mesh);
                }
                else
                {
                    ret = new VKMeshVertexBuffer(window, mesh);
                }
            }

            break;
        case Type::Patch:
            PR_LOG_RUNTIME_ERROR("Patch vertex buffer creation is not yet implemented!\n");
            break;
        default:
            break;
        }

        return ret;
    }
};
