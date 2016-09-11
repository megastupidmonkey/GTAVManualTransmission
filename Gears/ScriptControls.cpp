#include "ScriptControls.hpp"

#include <Windows.h>
#include "../../ScriptHookV_SDK/inc/natives.h"
#include "../../ScriptHookV_SDK/inc/enums.h"

ScriptControls::ScriptControls(): wheelState(nullptr),
                                  buttonState(0) {
	Wheelptr = new WheelInput();
	controller = new XboxController(1);
}

ScriptControls::~ScriptControls() {
}

void ScriptControls::UpdateValues(InputDevices prevInput) {
	if (controller->IsConnected()) {
		buttonState = controller->GetState().Gamepad.wButtons;
		controller->UpdateButtonChangeStates();
	}

	if (Wheelptr->IsConnected()) {
		wheelState = Wheelptr->GetState();
		Wheelptr->UpdateButtonChangeStates();
		SteerVal = wheelState->lX;
	}

	switch (prevInput) {
		case Keyboard:
			ThrottleVal = (IsKeyPressed(KBControl[static_cast<int>(KeyboardControlType::Throttle)]) ? 1.0f : 0.0f);
			BrakeVal = (IsKeyPressed(KBControl[static_cast<int>(KeyboardControlType::Brake)]) ? 1.0f : 0.0f);
			ClutchVal = (IsKeyPressed(KBControl[static_cast<int>(KeyboardControlType::Clutch)]) ? 1.0f : 0.0f);
			break;
		case Controller:
			ThrottleVal = controller->GetAnalogValue(controller->StringToButton(ControlXbox[static_cast<int>(ControllerControlType::Throttle)]), buttonState);
			BrakeVal = controller->GetAnalogValue(controller->StringToButton(ControlXbox[static_cast<int>(ControllerControlType::Brake)]), buttonState);
			ClutchVal = controller->GetAnalogValue(controller->StringToButton(ControlXbox[static_cast<int>(ControllerControlType::Clutch)]), buttonState);
			break;
		case Wheel:
			ThrottleVal = 1.0f - static_cast<float>(WheelDI->JoyState.lY) / 65535.0f;
			BrakeVal = 1.0f - static_cast<float>(WheelDI->JoyState.lRz) / 65535.0f;
			ClutchVal = 1.0f - static_cast<float>(WheelDI->JoyState.rglSlider[1]) / 65535.0f;
			break;
		default: break;
	}

	AccelValGTA = CONTROLS::GET_CONTROL_VALUE(0, ControlVehicleAccelerate);
	AccelValGTAf = (AccelValGTA - 127) / 127.0f;
}

// Limitation: Only works for hardcoded input types. Currently throttle.
ScriptControls::InputDevices ScriptControls::GetLastInputDevice(InputDevices previousInput) {
	if (IsKeyJustPressed(KBControl[static_cast<int>(KeyboardControlType::Throttle)], KeyboardControlType::Throttle) ||
		IsKeyPressed(KBControl[static_cast<int>(KeyboardControlType::Throttle)])) {
		return Keyboard;
	}
	if (controller->IsButtonJustPressed(controller->StringToButton(ControlXbox[static_cast<int>(ControllerControlType::Throttle)]), buttonState) ||
		controller->IsButtonPressed(controller->StringToButton(ControlXbox[static_cast<int>(ControllerControlType::Throttle)]), buttonState)) {
		return Controller;
	}
	if (Wheelptr->IsConnected() &&
		wheelState != nullptr &&
		1.0f - static_cast<float>(wheelState->lY) / 65535.0f > 0.5f) {
		return Wheel;
	}
	return previousInput;
}

/*
 * Keyboard section
 */

bool ScriptControls::IsKeyPressed(int key) {
	if (GetAsyncKeyState(key) & 0x8000)
		return true;
	return false;
}

bool ScriptControls::IsKeyJustPressed(int key, KeyboardControlType control) {
	KBControlCurr[static_cast<int>(control)] = (GetAsyncKeyState(key) & 0x8000) != 0;

	// raising edge
	if (KBControlCurr[static_cast<int>(control)] && !KBControlPrev[static_cast<int>(control)]) {
		KBControlPrev[static_cast<int>(control)] = KBControlCurr[static_cast<int>(control)];
		return true;
	}

	KBControlPrev[static_cast<int>(control)] = KBControlCurr[static_cast<int>(control)];
	return false;
}

bool ScriptControls::ButtonJustPressed(KeyboardControlType control) {
	if (IsKeyJustPressed(KBControl[static_cast<int>(control)], control))
		return true;
	return false;
}

/*
* Controller section
*/

bool ScriptControls::ButtonJustPressed(ControllerControlType control) {
	if (!controller->IsConnected())
		return false;
	if (controller->IsButtonJustPressed(controller->StringToButton(ControlXbox[static_cast<int>(control)]), buttonState))
		return true;
	return false;
}

bool ScriptControls::ButtonReleased(ControllerControlType control) {
	if (!controller->IsConnected())
		return false;
	if (controller->IsButtonJustReleased(controller->StringToButton(ControlXbox[static_cast<int>(control)]), buttonState))
		return true;
	return false;
}

bool ScriptControls::ButtonHeld(ControllerControlType control) {
	if (!controller->IsConnected())
		return false;
	if (controller->WasButtonHeldForMs(controller->StringToButton(ControlXbox[static_cast<int>(control)]), buttonState, CToggleTime))
		return true;
	return false;
}

bool ScriptControls::ButtonIn(ControllerControlType control) {
	if (!controller->IsConnected())
		return false;
	if (controller->IsButtonPressed(controller->StringToButton(ControlXbox[static_cast<int>(control)]), buttonState))
		return true;
	return false;
}

/*
 * Wheel section
 */

bool ScriptControls::ButtonReleased(WheelControlType control) {
	if (!Wheelptr->IsConnected() ||
		WheelControl[static_cast<int>(control)] == -1) {
		return false;
	}
	if (Wheelptr->IsButtonJustReleased(WheelControl[static_cast<int>(control)]))
		return true;
	return false;
}

bool ScriptControls::ButtonJustPressed(WheelControlType control) {
	if (!Wheelptr->IsConnected() ||
		WheelControl[static_cast<int>(control)] == -1) {
		return false;
	}
	if (Wheelptr->IsButtonJustPressed(WheelControl[static_cast<int>(control)]))
		return true;
	return false;
}

bool ScriptControls::ButtonIn(WheelControlType control) {
	if (!Wheelptr->IsConnected() ||
		WheelControl[static_cast<int>(control)] == -1) {
		return false;
	}
	if (Wheelptr->IsButtonPressed(WheelControl[static_cast<int>(control)]))
		return true;
	return false;
}
