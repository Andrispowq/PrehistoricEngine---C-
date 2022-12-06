#include "Includes.hpp"
#include "VertexBufferLoader.h"

#include "prehistoric/core/resources/AssetManager.h"
#include "prehistoric/application/Application.h"

#include "prehistoric/core/model/loader/PrehistoricModelFormat.h"

namespace Prehistoric
{
    static Model meshes[NUM_THREADS];

    static OBJLoader loader;
    static PrehistoricModelFormat prehistoric_format_loader;

    static void load_meshes(size_t start, size_t count, std::string locations[])
    {
        for (size_t i = start; i < (start + count); i++)
        {
            Model model;
            std::string location = locations[i];
            if (location.substr(location.length() - 3, 3) != "pmf")
            {
                model = loader.LoadModel(location, "", "");
                PrehistoricModelFormat::SavePrehistoricModel(location.substr(0, location.length() - 3) + "pmf", model);
            }
            else
            {
                model = prehistoric_format_loader.LoadModel(location);
            }
            meshes[i] = std::move(model); //TODO
        }
    }

    VertexBufferLoader::VertexBufferLoader(Window* window, AssetManager* manager)
        : Loader(window), manager(manager)
    {
        loader = OBJLoader(window, manager);
        prehistoric_format_loader = PrehistoricModelFormat(window, manager);
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
                if (__FrameworkConfig.api == OpenGL)
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
                size_t last_slash = path.find_last_of('/');

                std::string _path = path.substr(0, last_slash);
                std::string _obj = path.substr(last_slash + 1, path.length() - last_slash - 1);

                Model model;
                if (path.substr(path.length() - 3, 3) != "pmf")
                {
                    model = loader.LoadModel(_path, _obj, "");
                    PrehistoricModelFormat::SavePrehistoricModel(path.substr(0, path.length() - 3) + "pmf", model);
                }
                else
                {
                    model = prehistoric_format_loader.LoadModel(path);
                }

                if (__FrameworkConfig.api == OpenGL)
                {
                    ret = new GLMeshVertexBuffer(window, model);
                }
                else
                {
                    ret = new VKMeshVertexBuffer(window, model);
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

            if (__FrameworkConfig.api == OpenGL)
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

    Model VertexBufferLoader::LoadModel(const std::string& path)
    {
        Model model;

        if (path.substr(path.length() - 3, 3) != "pmf")
        {
            model = loader.LoadModel(path, "", "");
            PrehistoricModelFormat::SavePrehistoricModel(path.substr(0, path.length() - 3) + "pmf", model);
        }
        else
        {
            model = prehistoric_format_loader.LoadModel(path);
        }

        return model;
    }
};
