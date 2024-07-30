#pragma once
#include "../engine/pch.h"

CREATE_SCRIPT_START( Start )

public : 
    GCKeyboardInputManager* pKeyboard;
    GCMouseInputManager* pMouse;
    void Start() override;
    void Update() override;

CREATE_SCRIPT_END
