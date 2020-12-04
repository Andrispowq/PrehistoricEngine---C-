#ifndef RENDERER_H
#define RENDERER_H

#include "engine/prehistoric/common/framework/Window.h"
#include "engine/prehistoric/core/node/movement/Camera.h"

#include "engine/prehistoric/common/rendering/pipeline/Pipeline.h"

class RenderableComponent;

class Renderer
{
public:
	Renderer(Window* window, Camera* camera);
	virtual ~Renderer() {}

	virtual void PrepareRendering() = 0;
	virtual void EndRendering() = 0;

	virtual void Render() = 0;

	virtual CommandBuffer* getDrawCommandBuffer() const { return nullptr; }

	void AddModel(RenderableComponent* renderable);
	void AddLight(Light* light);

	Window* getWindow() const { return window; }
	Camera* getCamera() const { return camera; }

	inline std::vector<Light*> getLights() const { return lights; }
	inline Light* getSun() const { return sun; }

	inline bool isWireframeMode() const { return wireframeMode; }
	inline void setWireframeMode(bool wire) { this->wireframeMode = wire; }

protected:
	Window* window;
	Camera* camera;

	//The rendering context
	std::unordered_map<Pipeline*, std::vector<RenderableComponent*>> models_3d;
	std::unordered_map<Pipeline*, std::vector<RenderableComponent*>> models_transparency;
	std::unordered_map<Pipeline*, std::vector<RenderableComponent*>> models_2d;

	std::vector<Light*> lights;
	Light* sun = nullptr; //Not owned

	bool wireframeMode;
};

#endif