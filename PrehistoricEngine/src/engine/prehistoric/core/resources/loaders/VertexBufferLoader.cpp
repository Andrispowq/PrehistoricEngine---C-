#include "Includes.hpp"
#include "VertexBufferLoader.h"

namespace Prehistoric
{
    static Mesh meshes[NUM_THREADS];

    static void load_meshes(size_t start, size_t count, std::string locations[])
    {
        for (size_t i = start; i < (start + count); i++)
        {
            meshes[i] = OBJLoader::LoadMesh(locations[i], "", "");
        }
    }

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

	void VertexBufferLoader::LoadResources()
	{
		size_t jobs = (int)loadQueue.size();

		PR_PROFILE("VertexBufferLoader::LoadResources() for " + std::to_string(jobs) + " jobs");

		threads.reserve(NUM_THREADS);

		size_t threadCount = 0;

		std::string* locations = new std::string[jobs];
		for (size_t i = 0; i < jobs; i++)
		{
			auto [name, extra, heap] = loadQueue[i];
			locations[i] = name;
		}

		for (size_t i = 0; i < NUM_THREADS; i++)
		{
			if (jobs == 0)
				break;

            threads.emplace_back(load_meshes, size_t(i), (size_t)1, locations);
            jobs--;
			threadCount++;
		}

		for (size_t i = 0; i < threadCount; i++)
		{
			threads[i].join();
		}

		delete[] locations;
		jobs = (int)loadQueue.size();

		for (size_t i = 0; i < jobs; i++)
		{
            VertexBuffer* ret;

            if (FrameworkConfig::api == OpenGL)
            {
                ret = new GLMeshVertexBuffer(window, meshes[i]);
            }
            else
            {
                ret = new VKMeshVertexBuffer(window, meshes[i]);
            }

            pointers.push_back(ret);
		}

		loadQueue.clear();
		threads.clear();
	}

    Mesh VertexBufferLoader::LoadMesh(const std::string& path)
    {
        Mesh mesh = OBJLoader::LoadMesh(path, "", "");
        return mesh;
    }
};
