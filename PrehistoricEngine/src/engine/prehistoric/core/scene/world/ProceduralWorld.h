#pragma once
#include <json.hpp>
#include "prehistoric/core/node/GameObject.h"

namespace Prehistoric {
    class Window;
    class AssembledAssetManager;
    nlohmann::json WriteProceduralObject(GameObject & object, AssembledAssetManager * manager);

    std::unique_ptr<GameObject> ReadProceduralObject(const nlohmann::json &record, Window *window,
                                                     AssembledAssetManager *manager);

    void ResolveProceduralReferences(GameObject &root, const nlohmann::json &records);

    void RemapProceduralReferences(const GameObject &source, GameObject &copy);

    void CopyProceduralMaterials(const GameObject &source, GameObject &copy);
}