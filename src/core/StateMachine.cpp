#include "pch.h"
#include "StateMachine.h"


GCStateMachine::GCStateMachine() : m_pCurrentState(nullptr) {}

void GCStateMachine::ChangeState(GCState* newState, void* args)
{
	if (m_pCurrentState)
		m_pCurrentState->Exit();

	m_pCurrentState = newState;
	m_pCurrentState->Enter(this, args);
}

void GCStateMachine::Update(float deltaTime)
{
	if (m_pCurrentState)
		m_pCurrentState->Update(deltaTime);
}