#pragma once
#include "../core/framework.h"
#include "../Render/pch.h"

#include "GCColor.h"
#include "Map.h"

using namespace DirectX;

// TODO Adding lots of stuff to the components
// TODO Transforms for colliders

class GCGameObject;



enum FLAGS
{
	UPDATE          = 1 << 0,
	FIXED_UPDATE    = 1 << 1,
    RENDER          = 1 << 2,
};
inline FLAGS operator|( FLAGS a, FLAGS b ) { return static_cast<FLAGS>(static_cast<int>(a) | static_cast<int>(b)); }



class Component
{
friend class GCGameObject;
friend class GCUpdateManager;
friend class GCPhysicManager;
friend class GCRenderManager;
public:
    virtual const int GetID() = 0;
    
    void SetActive( bool active ) { m_active = active; }
    bool IsActive() { return m_active; }

    GCGameObject* GetGameObject() { return m_pGameObject; }

protected:
    Component( GCGameObject* pGameObject );
    virtual ~Component() = default;
    
    void Init();
    
    virtual void Start() {}
    virtual void Update() {}
    virtual void FixedUpdate() {}
    virtual void Render() {}
    virtual void Destroy() {}
    
    virtual FLAGS GetFlags() = 0;

    bool IsFlagSet( FLAGS flag ) { return ( GetFlags() & flag ) != 0; }

protected:
    inline static int componentCount = 0;
    GCGameObject* m_pGameObject;
    bool m_active;
    
    GCListNode<Component*>* m_pUpdateNode;
    GCListNode<Component*>* m_pPhysicsNode;
    GCListNode<Component*>* m_pRenderNode;

};



class SpriteRenderer : public Component
{
friend class GCGameObject;
friend class GCUpdateManager;
friend class GCPhysicManager;
friend class GCRenderManager;
public:
    static const int GetIDStatic() { return m_ID; }
    const int GetID() override { return m_ID; }
    
    
    void SetSprite( std::string texturePath );
    void SetColor(); 
    
    void GetSprite() {};
    GCColor& GetColor() { return m_color; }

protected:
	SpriteRenderer( GCGameObject* pGameObject );
    ~SpriteRenderer() override {}

    void Render() override;
    void Destroy() override {}
    
    FLAGS GetFlags() override { return RENDER; }

protected:
    inline static const int m_ID = ++Component::componentCount;
    GCColor m_color;

    GCMesh* m_pMesh;
    GCMaterial* m_pMaterial;

    XMMATRIX* m_worldMatrix;

};



class Collider : public Component
{
friend class GCGameObject;
friend class GCUpdateManager;
friend class GCPhysicManager;
friend class GCRenderManager;

public:
    Collider( GCGameObject* pGameObject );
    ~Collider() override {}

    void SetTrigger( bool trigger ) { m_trigger = trigger; }
    void SetVisible( bool showCollider ) { m_visible = showCollider; }
    
    bool IsTrigger() { return m_trigger; }
    bool IsVisible() { return m_visible; }

protected:
    FLAGS GetFlags() override { return FIXED_UPDATE | RENDER; }

protected:
    bool m_trigger;
    bool m_visible;

    GCMesh* m_pMesh;
    GCMaterial* m_pMaterial;

    XMMATRIX* m_worldMatrix;

};



class BoxCollider : public Collider
{
friend class GCGameObject;
friend class GCUpdateManager;
friend class GCPhysicManager;
friend class GCRenderManager;
public:
    static const int GetIDStatic() { return m_ID; }
    const int GetID() override { return m_ID; }

    GCVEC2 GetSize() { return m_size; }
    void SetSize( GCVEC2 size ) { m_size = size; }

protected:
    BoxCollider(GCGameObject* pGameObject);
    ~BoxCollider() override {}

    void FixedUpdate() override {}
    void Render() override;
    void Destroy() override {}

protected:
    inline static const int m_ID = ++Component::componentCount;
    GCVEC2 m_size;

};



class CircleCollider : public Collider
{
friend class GCGameObject;
friend class GCUpdateManager;
friend class GCPhysicManager;
friend class GCRenderManager;
public:
    static const int GetIDStatic() { return m_ID; }
    const int GetID() override { return m_ID; }
    
    float GetRadius() { return m_radius; }
    void SetRadius( float radius ) { m_radius = radius; }

protected:
    CircleCollider( GCGameObject* pGameObject ) : Collider( pGameObject ) {};
    ~CircleCollider() override {}
    
    void FixedUpdate() override {}
    void Render() override {}
    void Destroy() override {}

protected:
    inline static const int m_ID = ++Component::componentCount;
    float m_radius;

};



class RigidBody : public Component
{
friend class GCGameObject;
friend class GCUpdateManager;
friend class GCPhysicManager;
friend class GCRenderManager;
public:
    static const int GetIDStatic() { return m_ID; }
    const int GetID() override { return m_ID; }
    
    void AddForce( GCVEC2 force ) {}

protected:
    RigidBody( GCGameObject* pGameObject );
    ~RigidBody() override {}
    
    void FixedUpdate() override;
    void Destroy() override {}
    
    FLAGS GetFlags() override { return FIXED_UPDATE; }

protected:
    inline static const int m_ID = ++Component::componentCount;
    GCVEC3 m_velocity;

};



class Animator : public Component
{
friend class GCGameObject;
friend class GCUpdateManager;
friend class GCPhysicManager;
friend class GCRenderManager;
public:
    static const int GetIDStatic() { return m_ID; }
    const int GetID() override { return m_ID; }

protected:
	Animator( GCGameObject* pGameObject ) : Component( pGameObject ) {};
    ~Animator() override {}
    
    void Update() override {}
    void Destroy() override {}
    
    FLAGS GetFlags() override { return UPDATE; }

protected:
    inline static const int m_ID = ++Component::componentCount;

};



class SoundMixer : public Component
{
friend class GCGameObject;
friend class GCUpdateManager;
friend class GCPhysicManager;
friend class GCRenderManager;
public:
    static const int GetIDStatic() { return m_ID; }
    const int GetID() override { return m_ID; }

protected:
	SoundMixer( GCGameObject* pGameObject ) : Component( pGameObject ) {};
    ~SoundMixer() override {}
    
    void Update() override {}
    void Destroy() override {}
    
    FLAGS GetFlags() override { return UPDATE; }

protected:
    inline static const int m_ID = ++Component::componentCount;

};



class Script : public Component
{
friend class GCGameObject;
friend class GCUpdateManager;
friend class GCPhysicManager;
friend class GCRenderManager;

protected:
    Script( GCGameObject* pGameObject ) : Component( pGameObject ) {};
    virtual ~Script() = default;
    
    virtual void OnTriggerEnter( Collider* collider ) = 0;
    virtual void OnTriggerStay( Collider* collider ) = 0;
    virtual void OnTriggerExit( Collider* collider ) = 0;
    
    FLAGS GetFlags() override { return UPDATE | FIXED_UPDATE; }

protected:
    inline static int scriptCount = (1<<15)-1;

};

#define CREATE_SCRIPT_START( CLASS_NAME ) \
    class Script##CLASS_NAME : public Script \
    { \
    public: \
        static const int GetIDStatic() { return m_ID; } \
        const int GetID() override { return m_ID; } \
     \
    protected: \
        Script##CLASS_NAME() = default; \
        ~Script##CLASS_NAME() {} \
         \
        /*void Start() override; \
        void Update() override; \
        void FixedUpdate() override; \
        void Destroy() override; \
         \
        void OnTriggerEnter( Collider* collider ) override; \
        void OnTriggerStay( Collider* collider ) override; \
        void OnTriggerExit( Collider* collider ) override;*/ \
     \
    protected: \
        inline static const int m_ID = ++Script::scriptCount; \
     \
    private:

#define CREATE_SCRIPT_END };


