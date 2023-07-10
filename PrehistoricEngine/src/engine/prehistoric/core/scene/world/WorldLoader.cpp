#include "Includes.hpp"
#include "WorldLoader.h"

#include "prehistoric/application/Application.h"

#include <json.hpp>

namespace Prehistoric
{
	void WorldLoader::LoadWorld(const std::string& worldFile, GameObject* root)
	{
		PR_PROFILE("Loading world from .wrld file");

		std::ifstream file;
		file.open(worldFile.c_str());

		std::string line;
		size_t texIndex = 0;
		size_t meshIndex = 0;

		AssetManager* man = manager->getAssetManager();

		if (file.is_open())
		{
			while (file.good())
			{
				std::getline(file, line);
				std::vector<std::string> tokens = Util::Split(line, ' ');
				std::vector<std::string> nameTokens = Util::Split(tokens[0], '.');

				if (line.empty() || nameTokens[0][0] == '#')
					continue;

				//Setting the directories which we'll be using to load models and textures from
				if (nameTokens[0] == "world")
				{
					if (nameTokens[1] == "name")
					{
						sceneName = tokens[1];
					}
				}

				if (nameTokens[0] == "directories")
				{
					if (nameTokens[1] == "models")
					{
						directoryModels = tokens[1];
					}
					if (nameTokens[1] == "textures")
					{
						directoryTextures = tokens[1];
					}
				}

				//Loading models and texture
				if (nameTokens[0] == "models")
				{
					if (nameTokens[1] == "load")
					{
						std::string meshName = tokens[1];
						std::string directory = tokens[2];

						// In case we already loaded this, don't add it to the queue
						std::optional<VertexBufferHandle> handle = man->loadVertexBuffer(std::nullopt, directoryModels + directory, BatchSettings::QueuedLoad);
						if (handle.has_value())
						{
							models.insert(std::make_pair(meshName, handle.value()));
						}
						else
						{
							meshIndex++;
							meshNames.push_back(meshName);
							meshDirectories.push_back(directory);
							frontFaces.push_back(tokens[3] == "clockwise" ? FrontFace::CLOCKWISE :
								(tokens[3] == "counter-clockwise" ? FrontFace::COUNTER_CLOCKWISE : FrontFace::DOUBLE_SIDED));
						}
					}
					else if (nameTokens[1] == "dispatch")
					{
						man->getVertexBufferLoader()->ForceLoadQueue();

						size_t count;
						VertexBuffer** pointers = (VertexBuffer**)man->getVertexBufferLoader()->GetLoadedPointers(count);

						if (count != meshIndex)
						{
							PR_LOG_ERROR("The loaded VertexBuffer count isn't equal to the requested VertexBuffer count!\n");
						}

						for (int i = 0; i < meshIndex; i++)
						{
							models.insert(std::make_pair(meshNames[i], man->storeVertexBuffer(pointers[i], directoryModels + meshDirectories[i])));
							pointers[i]->setFrontFace(frontFaces[i]);
						}

						man->getVertexBufferLoader()->FlushPointers();
					}
				}
				if (nameTokens[0] == "textures")
				{
					if (nameTokens[1] == "load")
					{
						std::string texName = tokens[1];
						std::string directory = tokens[2];

						// In case we already loaded this, don't add it to the queue
						std::optional<TextureHandle> handle = man->loadTexture(directoryTextures + directory, Anisotropic, Repeat, BatchSettings::QueuedLoad);
						if (handle.has_value())
						{
							textures.insert(std::make_pair(texName, handle.value()));
						}
						else
						{
							texIndex++;
							textureNames.push_back(texName);
							textureDirectories.push_back(directory);
						}
					}
					else if (nameTokens[1] == "dispatch")
					{
						//texIndex++;
						//textureNames.push_back(EnvironmentMapConfig::environmentMapLocation);
						//man->loadTexture(EnvironmentMapConfig::environmentMapLocation, Bilinear, ClampToEdge, BatchSettings::QueuedLoad);

						man->getTextureLoader()->ForceLoadQueue();

						size_t count;
						Texture** pointers = (Texture**) man->getTextureLoader()->GetLoadedPointers(count);

						if (count != texIndex)
						{
							PR_LOG_ERROR("The loaded texture count isn't equal to the requested texture count!\n");
						}

						for (int i = 0; i < texIndex; i++)
						{
							textures.insert(std::make_pair(textureNames[i], man->storeTexture(pointers[i], directoryTextures + textureDirectories[i])));
						}

						man->getTextureLoader()->FlushPointers();
					}
				}

				//Creating materials from textures and other data
				if (nameTokens[0] == "materials")
				{
					if (nameTokens[1] == "add")
					{
						MaterialHandle material = manager->storeMaterial(new Material(man));
						material->setName(tokens[1]);
						materials.insert(std::make_pair(tokens[1], material));
					}
					else
					{
						auto index = materials.find(nameTokens[1]);

						if (index == materials.end())
						{
							PR_LOG_ERROR("Material (name: %s) does not exist, or haven't been created in the world file!\n", nameTokens[1].c_str());
							continue;
						}

						MaterialHandle material = materials.at(nameTokens[1]);

						if (nameTokens[2] == "texture")
						{
							auto index = textures.find(tokens[2]);

							if (index == textures.end())
							{
								PR_LOG_ERROR("Texture (name: %s) does not exist, or hasn't been created in the world file!\n", tokens[1].c_str());
								continue;
							}

							material->addTexture(tokens[1] + "Map", textures.at(tokens[2]));
						}

						if (nameTokens[2] == "vec4")
						{
							std::vector<std::string> compTokens = Util::Split(tokens[2], ',');

							Vector4f value = Vector4f((float)std::atof(compTokens[0].c_str()), (float)std::atof(compTokens[1].c_str()), (float)std::atof(compTokens[2].c_str()), (float)std::atof(compTokens[3].c_str()));
							material->addVector4f(tokens[1], value);
						}
						if (nameTokens[2] == "vec3")
						{
							std::vector<std::string> compTokens = Util::Split(tokens[2], ',');

							Vector3f value = Vector3f((float)std::atof(compTokens[0].c_str()), (float)std::atof(compTokens[1].c_str()), (float)std::atof(compTokens[2].c_str()));
							material->addVector3f(tokens[1], value);
						}
						if (nameTokens[2] == "vec2")
						{
							std::vector<std::string> compTokens = Util::Split(tokens[2], ',');

							Vector2f value = Vector2f((float)std::atof(compTokens[0].c_str()), (float)std::atof(compTokens[1].c_str()));
							material->addVector2f(tokens[1], value);
						}

						if (nameTokens[2] == "float")
						{
							float value = static_cast<float>(std::atof(tokens[2].c_str()));
							material->addFloat(tokens[1], value);
						}
						if (nameTokens[2] == "int")
						{
							int value = std::atoi(tokens[2].c_str());
							material->addInt(tokens[1], value);
						}
					}
				}

				//Creating GameObjects
				if (nameTokens[0] == "root")
				{
					if (nameTokens[1] == "object")
					{
						if (nameTokens.size() < 3)
						{
							GameObject* obj = new GameObject();

							if (tokens.size() > 2)
							{
								std::vector<std::string> compTokens = Util::Split(tokens[2], ',');
								obj->SetPosition({ (float)std::atof(compTokens[0].c_str()), (float)std::atof(compTokens[1].c_str()), (float)std::atof(compTokens[2].c_str()) });
							}
							if (tokens.size() > 3)
							{
								std::vector<std::string> compTokens = Util::Split(tokens[3], ',');
								obj->SetRotation({ (float)(float)std::atof(compTokens[0].c_str()), (float)std::atof(compTokens[1].c_str()), (float)std::atof(compTokens[2].c_str()) });
							}
							if (tokens.size() > 4)
							{
								std::vector<std::string> compTokens = Util::Split(tokens[4], ',');
								obj->SetScale({ (float)std::atof(compTokens[0].c_str()), (float)std::atof(compTokens[1].c_str()), (float)std::atof(compTokens[2].c_str()) });
							}

							root->AddChild(tokens[1], obj);
						}
						else
						{
							int i = 2;
							GameObject* objToAdd = root;
							while (true)
							{
								objToAdd = reinterpret_cast<GameObject*>(objToAdd->getChild(nameTokens[i]));
								i++;

								if (nameTokens.size() < (size_t(i) + 1))
								{
									GameObject* obj = new GameObject();

									if (tokens.size() > 2)
									{
										std::vector<std::string> compTokens = Util::Split(tokens[2], ',');
										obj->SetPosition({ (float)std::atof(compTokens[0].c_str()), (float)std::atof(compTokens[1].c_str()), (float)std::atof(compTokens[2].c_str()) });
									}
									if (tokens.size() > 3)
									{
										std::vector<std::string> compTokens = Util::Split(tokens[3], ',');
										obj->SetRotation({ (float)std::atof(compTokens[0].c_str()), (float)std::atof(compTokens[1].c_str()), (float)std::atof(compTokens[2].c_str()) });
									}
									if (tokens.size() > 4)
									{
										std::vector<std::string> compTokens = Util::Split(tokens[4], ',');
										obj->SetScale({ (float)std::atof(compTokens[0].c_str()), (float)std::atof(compTokens[1].c_str()), (float)std::atof(compTokens[2].c_str()) });
									}

									objToAdd->AddChild(tokens[1], obj);

									break;
								}
							}
						}
					}
				}

				//Adding components to objects
				if (nameTokens[0] == "object")
				{
					auto list = root->getChildren();

					auto index = list.find(nameTokens[1]);

					if (index == list.end())
					{
						PR_LOG_ERROR("GameObject (name: %s) does not exist, or haven't been created in the world file!\n", nameTokens[1].c_str());
						continue;
					}

					GameObject* obj = reinterpret_cast<GameObject*>(list[nameTokens[1]]);

					if (nameTokens[2] == "component")
					{
						if (tokens[1] == "Renderer")
						{
							std::vector<std::string> compTokens = Util::Split(tokens[2], ',');

							VertexBufferHandle vbo = models.at(compTokens[0]);
							ShaderHandle shader;

							MaterialHandle material = materials.at(compTokens[2]);
							PipelineHandle pipeline;

							auto shaderIndex = shaders.find(compTokens[1]);
							if (shaderIndex != shaders.end())
							{
								shader = shaderIndex->second;
							}
							else
							{
								ShaderName name;
								if (compTokens[1] == "pbr")
								{
									name = ShaderName::PBR;
								}

								shader = man->loadShader(name).value();
								shaders.insert(std::make_pair(compTokens[1], shader));
							}

							auto pipelineIndex = pipelines.find(compTokens[0] + "," + compTokens[1]);
							if (pipelineIndex != pipelines.end())
							{
								pipeline = pipelineIndex->second;
							}
							else
							{
								pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, vbo);
								
								if (__FrameworkConfig.api == OpenGL)
								{
									reinterpret_cast<GLGraphicsPipeline*>(pipeline.pointer)->setBackfaceCulling(true);
								}
								else if (__FrameworkConfig.api == Vulkan)
								{
									reinterpret_cast<VKGraphicsPipeline*>(pipeline.pointer)->setBackfaceCulling(true);
								}

								pipelines.emplace(std::make_pair(compTokens[0] + "," + compTokens[1], pipeline));
							}

							RendererComponent* renderer = new RendererComponent(window, manager, pipeline, material);

							obj->AddComponent(tokens[1], renderer);
						}
						if (tokens[1] == "Light")
						{
							std::vector<std::string> compTokens = Util::Split(tokens[2], ';');

							Light* light = new Light();

							std::vector<std::string> posTokens = Util::Split(compTokens[0], ',');
							light->setColour({ (float)std::atof(posTokens[0].c_str()), (float)std::atof(posTokens[1].c_str()), (float)std::atof(posTokens[2].c_str()) });
							light->setIntensity((float)std::atof(compTokens[1].c_str()));
							light->setRadius((float)std::atof(compTokens[2].c_str()));

							obj->AddComponent("Light", light);
						}
					}
					else
					{
						int i = 2;
						GameObject* objToAdd = obj;
						while (true)
						{
							auto list = objToAdd->getChildren();

							auto index = list.find(nameTokens[i]);

							if (index == list.end())
							{
								PR_LOG_ERROR("GameObject (name: %s) does not exist, or haven't been created in the world file!\n", nameTokens[i].c_str());
								continue;
							}

							objToAdd = reinterpret_cast<GameObject*>(list[nameTokens[1]]);
							i++;

							if (nameTokens[i] == "component")
							{
								if (tokens[1] == "Renderer")
								{
									std::vector<std::string> compTokens = Util::Split(tokens[2], ',');

									VertexBufferHandle vbo = models.at(compTokens[0]);
									ShaderHandle shader;

									MaterialHandle material = materials.at(compTokens[2]);
									PipelineHandle pipeline;

									auto shaderIndex = shaders.find(compTokens[1]);
									if (shaderIndex != shaders.end())
									{
										shader = shaderIndex->second;
									}
									else
									{
										ShaderName name;
										if (compTokens[1] == "pbr")
										{
											name = ShaderName::PBR;
										}

										shader = man->loadShader(name).value();
										shaders.insert(std::make_pair(compTokens[1], shader));
									}

									auto pipelineIndex = pipelines.find(compTokens[0] + "," + compTokens[1]);
									if (pipelineIndex != pipelines.end())
									{
										pipeline = pipelineIndex->second;
									}
									else
									{
										pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, vbo);

										if (__FrameworkConfig.api == OpenGL)
										{
											reinterpret_cast<GLGraphicsPipeline*>(pipeline.pointer)->setBackfaceCulling(true);
										}
										else if (__FrameworkConfig.api == Vulkan)
										{
											reinterpret_cast<VKGraphicsPipeline*>(pipeline.pointer)->setBackfaceCulling(true);
										}

										pipelines.emplace(std::make_pair(compTokens[0] + "," + compTokens[1], pipeline));
									}

									RendererComponent* renderer = new RendererComponent(window, manager, pipeline, material);

									obj->AddComponent(tokens[1], renderer);

									break;
								}
								if (tokens[1] == "Light")
								{
									std::vector<std::string> compTokens = Util::Split(tokens[2], ';');

									Light* light = new Light();

									std::vector<std::string> posTokens = Util::Split(compTokens[0], ',');
									light->setColour({ (float)std::atof(posTokens[0].c_str()), (float)std::atof(posTokens[1].c_str()), (float)std::atof(posTokens[2].c_str()) });
									light->setIntensity((float)std::atof(compTokens[1].c_str()));

									obj->AddComponent("Light", light);

									break;
								}
							}
							else
							{
								continue;
							}
						}
					}
				}
			}
		}
		else
		{
			PR_LOG_ERROR("Could not open world file %s!\n", worldFile.c_str());
		}
	}

	void WorldLoader::LoadWorldJSON(const std::string& worldFile, GameObject* root)
	{
		PR_PROFILE("Loading world from .json file");

		std::ifstream file;
		file.open(worldFile.c_str(), std::ios::ate);

		AssetManager* man = manager->getAssetManager();

		if (file.is_open())
		{
			std::string contents;
			size_t size = file.tellg();
			contents.resize(size);
			file.seekg(0);
			file.read(&contents[0], size);

			nlohmann::json file_json = nlohmann::json::parse(contents);
			sceneName = file_json["name"];

			{
				nlohmann::json directories = file_json["directories"];
				directoryModels = directories["models"];
				directoryTextures = directories["textures"];
				directoryConfigs = directories["configs"];
			}

			{
				std::vector<nlohmann::json> models_list = file_json["models"];
				for (auto& model : models_list)
				{
					std::string name = model["name"];
					std::string directory = model["directory"];
					int front_face = model["front-face"];

					FrontFace frontFace = FrontFace::DOUBLE_SIDED;
					if (front_face == 0)
					{
						frontFace = FrontFace::CLOCKWISE;
					}
					else if (front_face == 1)
					{
						frontFace = FrontFace::COUNTER_CLOCKWISE;
					}

					// In case we already loaded this, don't add it to the queue
					std::optional<VertexBufferHandle> handle = man->loadVertexBuffer(std::nullopt, directoryModels + directory, BatchSettings::QueuedLoad);
					if (handle.has_value())
					{
						models.insert(std::make_pair(name, handle.value()));
					}
					else
					{
						meshNames.push_back(name);
						meshDirectories.push_back(directory);
						frontFaces.push_back(frontFace);
					}
				}

				man->getVertexBufferLoader()->ForceLoadQueue();

				size_t count;
				VertexBuffer** pointers = (VertexBuffer**)man->getVertexBufferLoader()->GetLoadedPointers(count);

				if (count != meshNames.size())
				{
					PR_LOG_ERROR("The loaded VertexBuffer count isn't equal to the requested VertexBuffer count!\n");
				}

				for (int i = 0; i < meshNames.size(); i++)
				{
					models.insert(std::make_pair(meshNames[i], man->storeVertexBuffer(pointers[i], directoryModels + meshDirectories[i])));
					pointers[i]->setFrontFace(frontFaces[i]);
				}

				man->getVertexBufferLoader()->FlushPointers();
			}

			{
				std::vector<nlohmann::json> textures_list = file_json["textures"];
				for (auto& texture : textures_list)
				{
					std::string name = texture["name"];
					std::string directory = texture["directory"];

					// In case we already loaded this, don't add it to the queue
					std::optional<TextureHandle> handle = man->loadTexture(directoryTextures + directory, Anisotropic, Repeat, BatchSettings::QueuedLoad);
					if (handle.has_value())
					{
						textures.insert(std::make_pair(name, handle.value()));
					}
					else
					{
						textureNames.push_back(name);
						textureDirectories.push_back(directory);
					}
				}

				man->getTextureLoader()->ForceLoadQueue();

				size_t count;
				Texture** pointers = (Texture**)man->getTextureLoader()->GetLoadedPointers(count);

				if (count != textureNames.size())
				{
					PR_LOG_ERROR("The loaded VertexBuffer count isn't equal to the requested VertexBuffer count!\n");
				}

				for (int i = 0; i < textureNames.size(); i++)
				{
					textures.insert(std::make_pair(textureNames[i], man->storeTexture(pointers[i], directoryTextures + textureDirectories[i])));
				}

				man->getTextureLoader()->FlushPointers();
			}

			{
				std::vector<nlohmann::json> materials_list = file_json["materials"];
				for (auto& element : materials_list)
				{
					std::string nameMat = element["name"];
					std::vector<nlohmann::json> contents = element["contents"];

					Material* material = new Material(man);
					material->setName(nameMat);

					for (auto& content : contents)
					{
						std::string name = content["name"];
						std::string type = content["type"];

						if (type == "texture")
						{
							std::string value = content["value"];
							material->addTexture((name + "Map"), textures[value]);
						}
						else if (type == "vec4")
						{
							std::vector<float> value = content["value"];
							material->addVector4f(name, Vector4f(value[0], value[1], value[2], value[3]));
						}
						else if (type == "vec3")
						{
							std::vector<float> value = content["value"];
							material->addVector3f(name, Vector3f(value[0], value[1], value[2]));
						}
						else if (type == "vec2")
						{
							std::vector<float> value = content["value"];
							material->addVector2f(name, Vector2f(value[0], value[1]));
						}
						else if (type == "float")
						{
							float value = content["value"];
							material->addFloat(name, value);
						}
						else if (type == "int")
						{
							int value = content["value"];
							material->addInt(name, value);
						}
					}

					materials.insert(std::make_pair(nameMat, manager->storeMaterial(material)));
				}
			}

			//Big world objects like sun, atmosphere, clouds, terrain, oceans, etc
			{
				std::vector<nlohmann::json> world_objects = file_json["world_objects"];
				for (auto& world_object : world_objects)
				{
					std::string name = world_object["name"];
					std::string type = world_object["type"];

					if (type == "scattering_atmosphere")
					{
						GameObject* sun = new GameObject();

						std::string sun_name = world_object["sun"];
						if (sun_name == "")
						{
							//sun->setUpdateFunction(&Atmosphere::sunMoveFunction);
							sun->AddComponent(LIGHT_COMPONENT, new Light(Vector3f(1, 0.95f, 0.87f), 100.0f, 50000.0f, true, true));
							Atmosphere::sunMoveFunction(sun, 0.0f);
							root->AddChild("sun", sun);
						}
						else
						{
							PR_LOG_ERROR("ERROR: can't bind existing sun to scattering atmosphere yet!\n");
						}

						Atmosphere* atmosphere = new Atmosphere(window, manager);
						atmosphere->setSun(sun->GetComponent<Light>());
						root->AddChild(name, atmosphere);
					}
					else if (type == "terrain")
					{
						std::string config = world_object["config"];

						Camera* camera = Application::Get().getEngineLayer()->getRenderingEngine()->getCamera();
						Terrain* terrain = new Terrain(window, camera, manager, directoryConfigs + config);
						terrain->UpdateQuadtree();
						root->AddChild(name, terrain);
					}
				}
			}

			{
				nlohmann::json root_contents = file_json["root"];
				std::vector<nlohmann::json> root_children = root_contents["children"];
				for (auto& root_child : root_children)
				{
					std::string name = root_child["name"];
					nlohmann::json transform = root_child["transform"];
					std::vector<nlohmann::json> components = root_child["components"];

					GameObject* object = new GameObject;
					object->setName(name);

					if (transform.contains("position"))
					{
						std::vector<float> position = transform["position"];
						object->SetPosition({ position[0], position[1], position[2] });
					}
					if (transform.contains("rotation"))
					{
						std::vector<float> rotation = transform["rotation"];
						object->SetRotation({ rotation[0], rotation[1], rotation[2] });
					}
					if (transform.contains("scaling"))
					{
						std::vector<float> scaling = transform["scaling"];
						object->SetScale({ scaling[0], scaling[1], scaling[2] });
					}

					for (auto& component : components)
					{
						std::string type = component["type"];

						if (type == "Renderer")
						{
							std::string modelName = component["model"];
							std::string shaderName = component["shader"];
							std::string materialName = component["material"];

							VertexBufferHandle vbo = models.at(modelName);
							ShaderHandle shader;

							MaterialHandle material = materials.at(materialName);
							PipelineHandle pipeline;

							auto shaderIndex = shaders.find(shaderName);
							if (shaderIndex != shaders.end())
							{
								shader = shaderIndex->second;
							}
							else
							{
								ShaderName name;
								if (shaderName == "pbr")
								{
									name = ShaderName::PBR;
								}

								shader = manager->getAssetManager()->loadShader(name).value();
								shaders.insert(std::make_pair(shaderName, shader));
							}

							auto pipelineIndex = pipelines.find(modelName + "," + shaderName);
							if (pipelineIndex != pipelines.end())
							{
								pipeline = pipelineIndex->second;
							}
							else
							{
								pipeline = manager->createPipeline(PipelineTypeHashFlags::Graphics, shader, vbo);

								if (__FrameworkConfig.api == OpenGL)
								{
									reinterpret_cast<GLGraphicsPipeline*>(pipeline.pointer)->setBackfaceCulling(true);
								}
								else if (__FrameworkConfig.api == Vulkan)
								{
									reinterpret_cast<VKGraphicsPipeline*>(pipeline.pointer)->setBackfaceCulling(true);
								}

								pipelines.emplace(std::make_pair(modelName + "," + shaderName, pipeline));
							}

							RendererComponent* rendererComponent = new RendererComponent(window, manager, pipeline, material);
							object->AddComponent(RENDERER_COMPONENT, rendererComponent);
						}
						else if (type == "Light")
						{
							std::vector<float> colour = component["colour"];
							float intensity = component["intensity"];
							float range = component["range"];

							Light* light = new Light(Vector3f(colour[0], colour[1], colour[2]), intensity, range, true, false);
							object->AddComponent(LIGHT_COMPONENT, light);
						}
					}

					root->AddChild(name, object);
				}
			}
		}
		else
		{
			PR_LOG_ERROR("Could not open world file %s!\n", worldFile.c_str());
		}
	}
};
