#include "pch.h"
#include "Scene.h"
#include "SceneManager.h"
#include "GameObject.h"
#include "Components.h"


GCScene::GCScene()
{
	m_active = false;
}


// <summary>
// Destroys the scene and removes it from the SceneManager.
// Freeing up memory and resources associated with it.
// </summary>
void GCScene::Destroy()
{
	GCSceneManager::DestroyScene( this );
}


// <summary>
// Loads the scene.
// This function is called when the scene is to be loaded.
// It informs the SceneManager to load the scene.
// </summary>
void GCScene::Load()
{
	GCSceneManager::LoadScene( this );
}


// <summary> 
// Unloads the scene.
// This function is called when the scene is to be unloaded.
// It informs the SceneManager to unload the scene.
// This function is typically used when switching between scenes to free up memory and resources.
// </summary>
void GCScene::Unload()
{
	GCSceneManager::UnloadScene( this );
}




// <summary>
// Updates the scene.
// This function iterates through all the game objects in the scene and calls their Update function.
// The Update function is responsible for updating the game object's state and behavior.
// </summary>
void GCScene::Update()
{
	for ( GCListNode<GCGameObject*>* pGameObjectNode = m_gameObjectsList.GetFirstNode(); pGameObjectNode != m_gameObjectsList.GetLastNode(); pGameObjectNode = pGameObjectNode->GetNext() )
		pGameObjectNode->GetData()->Update();
}


// <summary>
// Renders all game objects with a SpriteRenderer Component in the scene.
// This function iterates through all the game objects in the scene and calls their SpriteRenderer's Render function.
// The Render function is responsible for drawing the game object on the screen.
// </summary>
void GCScene::Render()
{
	SpriteRenderer* pSpriteRenderer;
	for ( GCListNode<GCGameObject*>* pGameObjectNode = m_gameObjectsList.GetFirstNode(); pGameObjectNode != m_gameObjectsList.GetLastNode(); pGameObjectNode = pGameObjectNode->GetNext() )
	{
		pSpriteRenderer = pGameObjectNode->GetData()->GetComponent<SpriteRenderer>();
		if ( pSpriteRenderer != nullptr )
		    pSpriteRenderer->Render();
	}
}




// <summary>
// Creates a new game object and adds it to the scene.
// This function creates a new game object with the specified name, activation status, tag, and layer.
// The game object is then added to the end of the game objects list in the scene.
// </summary>

// <param name="name"> The name of the game object. Default value is "GameObject". </param>
// <param name="active"> The activation status of the game object. Default value is true.</param>
// <param name="tag"> The tag of the game object. Default value is an empty string.</param>
// <param nam="layer"> The layer of the game object. Default value is 0.</param>
GCGameObject* GCScene::CreateGameObject( const char* name = "GameObject", bool active = true, const char* tag = "", int layer = 0 )
{
	GCGameObject* pGameObject = new GCGameObject( this, name, nullptr, active, tag, layer );
	m_gameObjectsList.PushBack( pGameObject );
	pGameObject->SetNode( m_gameObjectsList.GetLastNode() );
	return pGameObject;
}


// <summary> 
// Destroys a game object and removes it from the scene.
// This function removes the specified game object from the scene's list of game objects.
// It also deletes the memory allocated for the game object.
// </summary>
// <param name="pGameObject"> A pointer to the game object to be destroyed. </param>
void GCScene::DestroyGameObject( GCGameObject* pGameObject )
{
	GCListNode<GCGameObject*>* pGameObjectNode = pGameObject->GetNode();
	m_gameObjectsList.DeleteNode( pGameObjectNode );
	delete pGameObject;
}

// <summary>
// Finds a game object by its name in the scene.
// This function iterates through all the game objects in the scene and checks if their names match the specified name.
// If a match is found, the function returns a pointer to the game object. If no match is found, the function returns nullptr.
// </summary>
// <param name="name"> The name of the game object to find.</param>
GCGameObject* GCScene::FindGameObjectByName( const char* name )
{
	GCGameObject* pGameObject;
	for ( GCListNode<GCGameObject*>* pGameObjectNode = m_gameObjectsList.GetFirstNode(); pGameObjectNode!= m_gameObjectsList.GetLastNode(); pGameObjectNode = pGameObjectNode->GetNext() )
	{
		pGameObject = pGameObjectNode->GetData();
		if ( pGameObject->GetName() == name )
			return pGameObject;
	}
}

// <summary>
// Finds a game object by its ID in the scene.
// This function iterates through all the game objects in the scene and checks if their IDs match the specified ID.
// If a match is found, the function returns a pointer to the game object. If no match is found, the function returns nullptr.
// </summary>
// <param name="ID"> The ID of the game object to find. </param>
GCGameObject* GCScene::FindGameObjectByID( int ID )
{
	GCGameObject* pGameObject;
	for ( GCListNode<GCGameObject*>* pGameObjectNode = m_gameObjectsList.GetFirstNode(); pGameObjectNode != m_gameObjectsList.GetLastNode(); pGameObjectNode = pGameObjectNode->GetNext() )
	{
		pGameObject = pGameObjectNode->GetData();
		if ( pGameObject->GetID() == ID )
            return pGameObject;
	}
}


// <summary>
// Duplicates a game object in the scene.
// This function creates a new game object with the same properties as the specified game object.
// The new game object is added to the end of the game objects list in the scene.
// </summary>
// <param name="pGameObject"> A pointer to the game object to be duplicated. </param>
void GCScene::DuplicateGameObject( GCGameObject* pGameObject )
{
	CreateGameObject( pGameObject->GetName(), pGameObject->IsActive(), pGameObject->GetTag(), pGameObject->GetLayer() );
}


// <summary>
// Removes a game object from the scene.
// This function removes the specified game object from the scene's list of game objects.
// It does not delete the memory allocated for the game object.
// This function does not delete the memory allocated for the game object.
//       It is the responsibility of the caller to delete the game object if necessary.
// </summary>
// <param name="pGameObject"> A pointer to the game object to be removed. </param>
GCGameObject* GCScene::RemoveGameObjectFromScene( GCGameObject* pGameObject )
{
	GCListNode<GCGameObject*>* pGameObjectNode = pGameObject->GetNode();
    m_gameObjectsList.DeleteNode( pGameObjectNode );
	return pGameObject;
}

// <summary>
// Moves a game object from the current scene to another specified scene.
// This function removes the specified game object from the current scene's list of game objects
// and adds it to the end of the game objects list in the specified scene.
// This function does not delete the memory allocated for the game object.
// It is the responsibility of the caller to delete the game object if necessary.
// This function assumes that the specified scene and game object exist and are valid.
// No error checking is performed to ensure that these conditions are met.
// </summary>
// <param name="pScene"> A pointer to the scene to which the game object should be moved. </param>
// <param name="pGameObject"> A pointer to the game object to be moved. </param>
void GCScene::MoveGameObjectToScene( GCScene* pScene, GCGameObject* pGameObject )
{ 
	RemoveGameObjectFromScene( pGameObject );
	pScene->m_gameObjectsList.PushBack( pGameObject ); 
	pGameObject->SetNode( pScene->m_gameObjectsList.GetLastNode() );
}


// TODO PREFAB