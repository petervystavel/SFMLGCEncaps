#include "pch.h"
#include "../Render/pch.h"

using namespace DirectX;



Component::Component()
{
	m_pGameObject = nullptr;
	m_globalActive = true;
	m_selfActive = true;
	
	m_created = false;
	
	m_pUpdateNode = nullptr;
	m_pPhysicsNode = nullptr;
	m_pRenderNode = nullptr;
}



void Component::RegisterToManagers()
{
	if ( IsFlagSet( UPDATE ) )
		GC::GetActiveUpdateManager()->RegisterComponent( this );
	
	if ( IsFlagSet( FIXED_UPDATE ) )
		GC::GetActivePhysicManager()->RegisterComponent( this );
	
	if ( IsFlagSet( RENDER ) )
		GC::GetActiveRenderManager()->RegisterComponent(this);
}

void Component::UnregisterFromManagers()
{
	if ( IsFlagSet( UPDATE ) )
	{
		m_pUpdateNode->Delete();
		m_pUpdateNode = nullptr;
	}
	
	if ( IsFlagSet( FIXED_UPDATE ) )
	{
		m_pPhysicsNode->Delete();
		m_pPhysicsNode = nullptr;
	}
	
	if ( IsFlagSet( RENDER ) )
	{
		m_pRenderNode->Delete();
		m_pRenderNode = nullptr;
	}
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Copies the values of this Component into the given Component.
/// 
/// @param pNewComponent The Component that will receive the values.
/// 
/// @note The m_pGameObject won't be passed to the new Component as a GameObject can't have more than one Component of the same ID.
/// @note The m_globalActive won't be passed to the new Component as it doesn't have any GameObject.
/// @note The new Component won't be registered to the Managers as it will be registered the next frame.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Component::CopyTo( Component* pDestination )
{ pDestination->m_selfActive = m_selfActive; }



void Component::Activate()
{
	if( m_selfActive == true )
		return;

	m_selfActive = true;

	if( m_globalActive == false )
		return;

	if( m_created == true )
		return;

	RegisterToManagers();
	OnActivate();
}

void Component::Deactivate()
{
	if ( m_selfActive == false )
		return;
	
	m_selfActive = false;
	
	if ( m_globalActive == false )
	    return;
	
	if ( m_created == false )
	    return;
	
	UnregisterFromManagers();
	OnDeactivate();
}

void Component::ActivateGlobal()
{
	if ( m_globalActive == true )
		return;
	
	m_globalActive = true;
	
	if ( m_selfActive == false )
		return;
	
	if ( m_created == false )
		return;
	
	RegisterToManagers();
	OnActivate();
}

void Component::DeactivateGlobal()
{
	if ( m_globalActive == false )
		return;
	
	m_globalActive = false;
	
	if ( m_selfActive == false )
		return;
	
	if ( m_created == false )
		return;
	
	UnregisterFromManagers();
	OnDeactivate();
}

void Component::SetActive( bool active )
{
	if ( active == true )
	{
		Activate();
		return;
	}
	Deactivate();
}






SpriteRenderer::SpriteRenderer()
{
	GCGraphics* pGraphics = GC::GetActiveRenderManager()->m_pGraphics;

	pGraphics->InitializeGraphicsResourcesStart();
	m_pMesh = pGraphics->CreateMeshColor(GC::GetActiveRenderManager()->m_pPlane).resource;
	pGraphics->InitializeGraphicsResourcesEnd();

	GCShader* shaderColor = pGraphics->CreateShaderColor().resource;
	m_pMaterial = pGraphics->CreateMaterial(shaderColor).resource;
}



void SpriteRenderer::CopyTo( Component* pDestination )
{
	Component::CopyTo( pDestination );
	SpriteRenderer* pSpriteRenderer = static_cast<SpriteRenderer*>( pDestination );
	*(pSpriteRenderer->m_pMesh) = *m_pMesh; //! Need to ask Render if this will work
	*(pSpriteRenderer->m_pMaterial) = *m_pMaterial; //! Need to ask Render if this will work
}

void SpriteRenderer::Render()
{
	GCGraphics* pGraphics = GC::GetActiveRenderManager()->m_pGraphics;
	pGraphics->UpdateWorldConstantBuffer(m_pMaterial, m_pGameObject->m_transform.GetWorldMatrix());
	pGraphics->GetRender()->DrawObject(m_pMesh, m_pMaterial, true);

}



/////////////////////////////////////////////////
/// @brief Set Sprite of a GameObject
/// 
/// @param texturePath name of the sprite file
/// 
/// @note The sprite must be in .dds 
/////////////////////////////////////////////////
void SpriteRenderer::SetSprite(std::string fileName)
{
	GCGraphics* pGraphics = GC::GetActiveRenderManager()->m_pGraphics;

	pGraphics->InitializeGraphicsResourcesStart();
	m_pMesh = pGraphics->CreateMeshTexture(GC::GetActiveRenderManager()->m_pPlane).resource;
	GCTexture* texture = pGraphics->CreateTexture( std::string("../../../src/Textures/") + fileName).resource;
	pGraphics->InitializeGraphicsResourcesEnd();

	ResourceCreationResult<GCShader*> shaderTexture = pGraphics->CreateShaderTexture();
	ResourceCreationResult<GCMaterial*> mat = pGraphics->CreateMaterial(shaderTexture.resource);
	m_pMaterial = mat.resource;
	m_pMaterial->SetTexture(texture);
}






Collider::Collider()
{
	m_trigger = false;
	m_visible = false;
}



void Collider::RegisterToManagers()
{
	Component::RegisterToManagers();
	GC::GetActivePhysicManager()->RegisterCollider( this );
}

void Collider::UnregisterFromManagers()
{
	Component::UnregisterFromManagers();
	m_pColliderNode->Delete();
}



void Collider::CopyTo( Component* pDestination )
{
	Component::CopyTo( pDestination );
	Collider* pCollider = static_cast<Collider*>( pDestination );
	pCollider->m_trigger = m_trigger;
	pCollider->m_visible = m_visible;
	*(pCollider->m_pMesh) = *m_pMesh; //! Need to ask Render if this will work
	*(pCollider->m_pMaterial) = *m_pMaterial; //! Need to ask Render if this will work
}






BoxCollider::BoxCollider()
{
	GCGraphics* pGraphics = GC::GetActiveRenderManager()->m_pGraphics;

	pGraphics->InitializeGraphicsResourcesStart();
	m_pMesh = pGraphics->CreateMeshTexture(GC::GetActiveRenderManager()->m_pPlane).resource;
	GCTexture* texture = pGraphics->CreateTexture("../../../src/Textures/BoxColliderSquare.dds").resource;
	pGraphics->InitializeGraphicsResourcesEnd();

	auto shaderTexture = pGraphics->CreateShaderTexture();
	auto mat = pGraphics->CreateMaterial(shaderTexture.resource);
	m_pMaterial = mat.resource;
	m_pMaterial->SetTexture(texture);
}



void BoxCollider::CopyTo( Component* pDestination )
{ Collider::CopyTo( pDestination ); }

void BoxCollider::Render()
{
	if (m_visible == false)
		return;

	GCGraphics* pGraphics = GC::GetActiveRenderManager()->m_pGraphics;

	pGraphics->UpdateWorldConstantBuffer(m_pMaterial, m_pGameObject->m_transform.GetWorldMatrix());
	pGraphics->GetRender()->DrawObject(m_pMesh, m_pMaterial, true);

}






void CircleCollider::CopyTo( Component* pDestination )
{ Collider::CopyTo( pDestination ); }






RigidBody::RigidBody()
{
	m_velocity.SetZero();
}



void RigidBody::CopyTo( Component* pDestination )
{
	Component::CopyTo( pDestination );
	RigidBody* pRigidBody = static_cast<RigidBody*>( pDestination );
	pRigidBody->m_velocity = m_velocity;
}

void RigidBody::FixedUpdate()
{
	// Apply velocity
	m_pGameObject->m_transform.Translate(m_velocity);		// TODO: Multiply by the fixed delta time
}






void Animator::CopyTo( Component* pDestination )
{ Component::CopyTo( pDestination ); }






void SoundMixer::CopyTo( Component* pDestination )
{ Component::CopyTo( pDestination ); }






Camera::Camera()
{
	m_position.SetZero();
	m_target.SetZero();
	m_up.SetZero();
	
	m_viewWidth = 20.0f;
	m_viewHeight = 20.0f;
	m_nearZ = 1.0f;
	m_farZ = 1000.0f;
	
	m_viewMatrix.SetIdentity();
	m_projectionMatrix.SetIdentity();
}



void Camera::CopyTo( Component* pDestination )
{
	Component::CopyTo( pDestination );
	Camera* pCamera = static_cast<Camera*>( pDestination );
	pCamera->m_nearZ = m_nearZ;
    pCamera->m_farZ = m_farZ;
    pCamera->m_viewWidth = m_viewWidth;
    pCamera->m_viewHeight = m_viewHeight;
}

void Camera::Update()
{
	bool dirty = false;
	
	if ( m_position != m_pGameObject->m_transform.m_position )
	{
		m_position = m_pGameObject->m_transform.m_position;
		dirty = true;
	}
	
	if ( m_up != m_pGameObject->m_transform.m_up )
	{
		m_up = m_pGameObject->m_transform.m_up;
		dirty = true;
	}
	
	if ( dirty == false )
		return;
	
	GC::GetActiveRenderManager()->m_pGraphics->CreateViewProjConstantBuffer( m_pGameObject->m_transform.m_position, m_target, m_pGameObject->m_transform.m_up, 0.0f, 0.0f, m_nearZ, m_farZ, m_viewWidth, m_viewHeight, GC_PROJECTIONTYPE::ORTHOGRAPHIC, m_projectionMatrix, m_viewMatrix );
}


