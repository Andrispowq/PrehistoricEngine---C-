#include "Includes.hpp"
#include "WorldSerialiser.h"

#include "prehistoric/core/node/component/renderer/RendererComponent.h"
#include "prehistoric/core/node/component/light/Light.h"
#include "prehistoric/core/node/GameObject.h"

#include "prehistoric/application/Application.h"

#include <json.hpp>

namespace Prehistoric
{
	void WorldSerialiser::SerialiseWorldJSON(const std::string& worldFile, Scene* scene)
	{
		GameObject* root = scene->getSceneRoot();
		sceneName = scene->getName();
		ListResources(root);

		nlohmann::json file_json;
		file_json["name"] = sceneName;

		{
			nlohmann::json directories;
			directories["models"] = directoryModels;
			directories["textures"] = directoryTextures;

			file_json["directories"] = directories;
		}

		{
			std::vector<nlohmann::json> models_list;
			for (auto& model : models)
			{
				nlohmann::json model_json;
				model_json["name"] = model.first;
				model_json["directory"] = model.first; //TODO: get the extension!!!
				
				FrontFace frontFace = model.second->getFrontFace();
				int front_face = 2;
				if (frontFace == FrontFace::CLOCKWISE)
				{
					front_face = 0;
				}
				else if (frontFace == FrontFace::COUNTER_CLOCKWISE)
				{
					front_face = 1;
				}
				model_json["front-face"] = front_face;

				models_list.push_back(model_json);
			}

			file_json["models"] = models_list;
		}

		{
			std::vector<nlohmann::json> textures_list;
			for (auto& texture : textures)
			{
				nlohmann::json texture_json;
				texture_json["name"] = texture.first;
				texture_json["directory"] = texture.first; //TODO: get the extension!!!

				textures_list.push_back(texture_json);
			}

			file_json["textures"] = textures_list;
		}

		{
			std::vector<nlohmann::json> materials_list;
			for (auto& material : materials)
			{
				nlohmann::json material_json;
				material_json["name"] = material.first;

				std::vector<nlohmann::json> contents_list;
				for (auto& tex : material.second->getTextures())
				{
					nlohmann::json content;
					content["name"] = tex.first;
					content["type"] = "texture";

					for (auto& elem : textures)
					{
						if (elem.second.handle == tex.second.handle)
						{
							content["value"] = elem.first;
							break;
						}
					}

					contents_list.push_back(content);
				}
				for (auto& vec4 : material.second->getVector4fs())
				{
					nlohmann::json content;
					content["name"] = vec4.first;
					content["type"] = "vec4";

					std::vector<float> value = { vec4.second.x, vec4.second.y, vec4.second.z, vec4.second.w };
					content["value"] = value;

					contents_list.push_back(content);
				}
				for (auto& vec3 : material.second->getVector3fs())
				{
					nlohmann::json content;
					content["name"] = vec3.first;
					content["type"] = "vec3";

					std::vector<float> value = { vec3.second.x, vec3.second.y, vec3.second.z };
					content["value"] = value;

					contents_list.push_back(content);
				}
				for (auto& vec2 : material.second->getVector2fs())
				{
					nlohmann::json content;
					content["name"] = vec2.first;
					content["type"] = "vec2";

					std::vector<float> value = { vec2.second.x, vec2.second.y };
					content["value"] = value;

					contents_list.push_back(content);
				}
				for (auto& fl : material.second->getFloats())
				{
					nlohmann::json content;
					content["name"] = fl.first;
					content["type"] = "float";
					content["value"] = fl.second;

					contents_list.push_back(content);
				}
				for (auto& in : material.second->getInts())
				{
					nlohmann::json content;
					content["name"] = in.first;
					content["type"] = "int";
					content["value"] = in.second;

					contents_list.push_back(content);
				}

				material_json["contents"] = contents_list;
				materials_list.push_back(material_json);
			}

			file_json["materials"] = materials_list;
		}

		{
			nlohmann::json root_json;
			std::vector<nlohmann::json> list_children;

			std::vector<GameObject*> children = root->getAllChildren(); //For now we serialise everything to be root's child
			for (auto child : children)
			{
				nlohmann::json child_json;
				child_json["name"] = child->getName();

				{
					nlohmann::json transform_json;

					Transform& transform = child->getWorldTransform();
					Vector3f position = transform.getPosition();
					Vector3f rotation = transform.getRotation();
					Vector3f scale = transform.getScaling();

					std::vector<float> position_v = { position.x, position.y, position.z };
					std::vector<float> rotation_v = { rotation.x, rotation.y, rotation.z };
					std::vector<float> scale_v = { scale.x, scale.y, scale.z };

					transform_json["position"] = position_v;
					transform_json["rotation"] = rotation_v;
					transform_json["scaling"] = scale_v;

					child_json["transform"] = transform_json;
				}

				{
					std::vector<nlohmann::json> components;

					std::unordered_map<std::string, Component*> comps = child->getComponents();
					for (auto elem : comps)
					{
						Component* comp = elem.second;
						nlohmann::json comp_json;

						if (comp->getComponentType() == ComponentType::RendererComponent)
						{
							RendererComponent* rendComp = (RendererComponent*)comp;

							comp_json["type"] = "Renderer";

							size_t vboHandle;
							if (__FrameworkConfig.api == API::OpenGL)
							{
								vboHandle = ((GLGraphicsPipeline*)rendComp->getPipeline())->getVertexBufferHandle().handle;
							}
							else if (__FrameworkConfig.api == API::Vulkan)
							{
								vboHandle = ((VKGraphicsPipeline*)rendComp->getPipeline())->getVertexBufferHandle().handle;
							}

							for (auto elem : models)
							{
								if (elem.second.handle == vboHandle)
								{
									comp_json["model"] = elem.first;
									break;
								}
							}

							ShaderHandle shader = rendComp->getPipeline()->getShaderHandle();
							if (dynamic_cast<GLPBRShader*>(shader.pointer))
							{
								comp_json["shader"] = "pbr";
							}

							std::string materialName = rendComp->getMaterial()->getName();
							static int index = 0;
							if (materialName == "")
							{
								materialName = ("UnnamedMaterial" + std::to_string(index++));
							}

							comp_json["material"] = materialName;
						}
						else if (comp->getComponentType() == ComponentType::LightComponent)
						{
							Light* lightComp = (Light*)comp;

							comp_json["type"] = "Light";

							std::vector<float> colour_v = { lightComp->getColour().x, lightComp->getColour().y, lightComp->getColour().z };
							comp_json["colour"] = colour_v;
							comp_json["intensity"] = lightComp->getIntensity();
							comp_json["range"] = lightComp->getRadius();
						}

						components.push_back(comp_json);
					}

					child_json["components"] = components;
				}

				list_children.push_back(child_json);
			}

			root_json["children"] = list_children;
			file_json["root"] = root_json;
		}

		std::fstream file;
		file.open(worldFile.c_str(), std::ios::out);

		if (file.is_open())
		{
			std::string result = file_json.dump(4);
			file.write(&result[0], result.size());

			file.flush();
			file.close();
		}
		else
		{
			PR_LOG_ERROR("Could not open world file %s!\n", worldFile.c_str());
		}
	}

	void WorldSerialiser::ListResources(GameObject* root)
	{
		std::vector<GameObject*> children = root->getAllChildren(); //we assume that root doesn't have components, which is the way it should be

		//TODO: directory.model and .texture
		std::vector<std::string> modelLocations;
		std::vector<std::string> textureLocation;
		for (auto child : children)
		{
			if (child->HasComponent<RendererComponent>())
			{
				RendererComponent* renderer = child->GetComponent<RendererComponent>();

				VertexBufferHandle vbo;
				if (__FrameworkConfig.api == API::OpenGL)
				{
					vbo = ((GLGraphicsPipeline*)renderer->getPipeline())->getVertexBufferHandle();
				}
				else if(__FrameworkConfig.api == API::Vulkan)
				{
					vbo = ((VKGraphicsPipeline*)renderer->getPipeline())->getVertexBufferHandle();
				}

				MaterialHandle material = renderer->getMaterialHandle();
				std::unordered_map<std::string, TextureHandle>& matTextures = material->getTextures();

				//Store models
				std::unordered_map<std::string, Resource> ID_map = manager->getAssetManager()->getIDMap();
				for (auto& elem : ID_map)
				{
					if (elem.second.type == ResourceType::VertexBuffer)
					{
						if (elem.second.handle == vbo.handle)
						{
							auto idx = models.find(elem.first);
							if (idx == models.end())
							{
								models.insert(std::make_pair(elem.first, vbo));
								break;
							}
						}
					}
				}

				//Store textures
				for (auto tex : matTextures)
				{
					for (auto& elem : ID_map)
					{
						if (elem.second.type == ResourceType::Texture)
						{
							if (elem.second.handle == tex.second.handle)
							{
								auto idx = textures.find(elem.first);
								if (idx == textures.end())
								{
									textures.insert(std::make_pair(elem.first, tex.second));
									break;
								}
							}
						}
					}
				}

				//Store material
				std::string matName = material->getName();
				static int index = 0;
				if (matName == "")
				{
					matName = ("UnnamedMaterial" + std::to_string(index++));
				}

				auto idx = materials.find(matName);
				if (idx == materials.end())
				{
					materials.insert(std::make_pair(matName, material));
				}				
			}
		}
	}
};
