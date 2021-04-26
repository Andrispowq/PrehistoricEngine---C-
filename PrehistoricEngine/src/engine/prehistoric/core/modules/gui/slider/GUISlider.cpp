#include "Includes.hpp"
#include "GUISlider.h"

namespace Prehistoric
{
	VertexBufferHandle GUISlider::slider_vbo = { nullptr, -1 };
	PipelineHandle GUISlider::slider_pipeline = { nullptr, -1 };

	GUISlider::GUISlider(Window* window, AssembledAssetManager* manager, float minValue, float maxValue, Vector3f colour, float* data, size_t dataSize, bool visible)
		: GUIElement(window, manager, colour, data, dataSize, visible), minValue(minValue), maxValue(maxValue), progress(0.5f)
	{
		background = new GUIElement(window, manager, colour, data, dataSize, visible);
		type = GUIType::Slider;

		if (slider_vbo.handle == -1)
		{
			ShaderHandle shader;

			slider_vbo = manager->getAssetManager()->storeVertexBuffer(ModelFabricator::CreateQuad(window));
			slider_vbo->setFrontFace(FrontFace::DOUBLE_SIDED);

			if (FrameworkConfig::api == OpenGL)
			{
				shader = manager->getAssetManager()->loadShader(ShaderName::Gui).value();
				slider_pipeline = manager->storePipeline(new GLGraphicsPipeline(window, manager->getAssetManager(), shader, vbo));
			}
			else if (FrameworkConfig::api == Vulkan)
			{
				//shader = manager->getAssetManager()->loadShader("gui").value();
				//pipeline = manager->storePipeline(new VKGraphicsPipeline(window, manager->getAssetManager(), shader, vbo));
			}
		}

		const Transform& tr = getWorldTransform();
		Vector3f pos = tr.getPosition();
		Vector3f scale = tr.getScaling();

		Vector3f min = pos - scale;
		Vector3f max = pos + scale;

		Vector3f progress_centre = { min.x + (max.x - min.x) * progress, pos.y, pos.z };
		Vector3f scaling = { scale.x / 15.0f, scale.y, 1 };
		slider_position = { progress_centre.x, progress_centre.y };
		slider_scale = { scaling.x, scaling.y };
		slider_transform = Matrix4f::Transformation(progress_centre, 0, scaling);

		RendererComponent* renderer = new RendererComponent(window, manager, slider_pipeline, manager->storeMaterial(nullptr));
		renderer->setPriority(RenderPriority::_2D);

		AddComponent(RENDERER_COMPONENT + std::string("_slider"), renderer);
	}

	GUISlider::~GUISlider()
	{
		delete background;
	}

	void GUISlider::PreUpdate(CoreEngine* engine)
	{
		//The data for the GUISlider MUST be a float, which we are controlling
		float* _data = (float*)data;

		Vector2f slider_min = slider_position - slider_scale;
		Vector2f slider_max = slider_position + slider_scale;

		bool in = false;
		if (dragged)
			in = inside(InputInstance.getCursorPosition());
		else
			in = inside(InputInstance.getCursorPosition(), slider_min, slider_max);

		if (InputInstance.IsButtonHeld(PR_MOUSE_BUTTON_LEFT) && in)
		{
			dragged = true;

			if (cursorOldPositionX == -1)
			{
				cursorOldPositionX = InputInstance.getCursorPosition().x;
			}
			else
			{
				float newPosition = InputInstance.getCursorPosition().x;
				if (newPosition == cursorOldPositionX)
					return;

				float diff = newPosition - cursorOldPositionX;
				float diff_ndc = diff / (float)window->getWidth();

				const Transform& tr = getWorldTransform();
				Vector3f pos = tr.getPosition();
				Vector3f scale = tr.getScaling();

				Vector3f min = pos - (scale / 2.0f);
				Vector3f max = pos + (scale / 2.0f);

				float plus_prog = diff_ndc / scale.x;
				float new_val = progress + plus_prog;
				if (new_val < 0.0f)
					progress = 0.0f;
				else if (new_val > 1.0f)
					progress = 1.0f;
				else
					progress = new_val;

				cursorOldPositionX = newPosition;
			}
		}
		else
		{
			dragged = false;
			cursorOldPositionX = -1;
		}

		*_data = minValue + (maxValue - minValue) * progress;

		const Transform& tr = getWorldTransform();
		Vector3f pos = tr.getPosition();
		Vector3f scale = tr.getScaling();

		background->SetPosition(pos);
		background->SetScale(scale);

		Vector3f min = pos - scale;
		Vector3f max = pos + scale;
		
		Vector3f progress_centre = { min.x + (max.x - min.x) * progress, pos.y, pos.z };
		Vector3f scaling = { min(scale.x / 15.0f, 0.125f / 15.0f), scale.y, 1 };
		slider_position = { progress_centre.x, progress_centre.y };
		slider_scale = { scaling.x, scaling.y };
		slider_transform = Matrix4f::Transformation(progress_centre, 0, scaling);
	}

	void GUISlider::PreRender(Renderer* renderer)
	{
		background->PreRender(renderer);
		GameObject::PreRender(renderer);
	}
};
