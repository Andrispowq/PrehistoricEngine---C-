#ifndef INPUT_H
#define INPUT_H

#include "engine/prehistoric/core/util/Includes.hpp"

#include "engine/config/FrameworkConfig.h"
#include "Window.h"

#define InputInstance Input::getInstance()

#define MAX_NUM_JOYSTICKS 2

class Input
{
public:
	static Input& getInstance();
	static void DeleteInstance();

	virtual bool Init(Window* window) const = 0;
	virtual bool Update() = 0;

	bool IsKeyPushed(const InputCode& key) const { return std::find(pushedKeys.begin(), pushedKeys.end(), (uint32_t)key) != pushedKeys.end(); }
	bool IsKeyHeld(const InputCode& key) const { return std::find(keysHolding.begin(), keysHolding.end(), (uint32_t)key) != keysHolding.end(); }
	bool IsKeyReleased(const InputCode& key) const { return std::find(releasedKeys.begin(), releasedKeys.end(), (uint32_t)key) != releasedKeys.end(); }

	bool IsButtonPushed(const InputCode& button) const { return std::find(pushedButtons.begin(), pushedButtons.end(), (uint32_t)button) != pushedButtons.end(); }
	bool IsButtonHeld(const InputCode& button) const { return std::find(buttonsHolding.begin(), buttonsHolding.end(), (uint32_t)button) != buttonsHolding.end(); }
	bool IsButtonReleased(const InputCode& button) const { return std::find(releasedButtons.begin(), releasedButtons.end(), (uint32_t)button) != releasedButtons.end(); }

	bool IsJoystickButtonPushed(const InputCode& key, const JoystickID& joystick) const;
	float getJoystickAxisOffset(const InputCode& axis, const JoystickID& joystick) const;

	bool IsPause() const { return pause; }
	void setPause(bool pause) { Input::pause = pause; }

	Vector2f getCursorPosition() const { return cursorPosition; }
	void setCursorPosition(const Vector2f& cursorPosition) { Input::cursorPosition = cursorPosition; }

	virtual void setCursorPositionOnScreen(Window* window, const Vector2f& cursorPosition) = 0;

	Vector2f getLockedCursorPosition() const { return lockedCursorPosition; }
	void setLockedCursorPosition(const Vector2f& lockedCursorPosition) { Input::lockedCursorPosition = lockedCursorPosition; }

	float getScrollOffset() const { return scrollOffset; }
	void setScrollOffset(float scrollOffset) { Input::scrollOffset = scrollOffset; }

	inline std::vector<int> getPushedKeys() const { return pushedKeys; }
	inline std::vector<int> getKeysHolding() const { return keysHolding; }
	inline std::vector<int> getReleasedKeys() const { return releasedKeys; }

	inline std::vector<int> getPushedButtons() const { return pushedButtons; }
	inline std::vector<int> getButtonsHolding() const { return buttonsHolding; }
	inline std::vector<int> getReleasedButtons() const { return releasedButtons; }

	inline void setPushedKeys(const std::vector<int>& pushedKeys) { this->pushedKeys = pushedKeys; }
	inline void setKeysHolding(const std::vector<int>& keysHolding) { this->keysHolding = keysHolding; }
	inline void setReleasedKeys(const std::vector<int>& releasedKeys) { this->releasedKeys = releasedKeys; }

	inline void setPushedButtons(const std::vector<int>& pushedButtons) { this->pushedButtons = pushedButtons; }
	inline void setButtonsHolding(const std::vector<int>& buttonsHolding) { this->buttonsHolding = buttonsHolding; }
	inline void setReleasedButtons(const std::vector<int>& releasedButtons) { this->releasedButtons = releasedButtons; }
protected:
	Input() : scrollOffset(0.0f), pause(false) {};
	virtual ~Input() {}
protected:
	static Input* instance;

	std::vector<int> pushedKeys;
	std::vector<int> keysHolding;
	std::vector<int> releasedKeys;

	std::vector<int> pushedButtons;
	std::vector<int> buttonsHolding;
	std::vector<int> releasedButtons;

	std::vector<int> joystickButtons[MAX_NUM_JOYSTICKS];
	std::vector<float> joystickAxes[MAX_NUM_JOYSTICKS];

	Vector2f cursorPosition;
	Vector2f lockedCursorPosition;
	float scrollOffset;

	bool pause;
};

#endif