/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
// Original file Copyright Crytek GMBH or its affiliates, used under license.

// Description : ObjectManager definition.


#ifndef CRYINCLUDE_EDITOR_OBJECTS_OBJECTMANAGER_H
#define CRYINCLUDE_EDITOR_OBJECTS_OBJECTMANAGER_H
#pragma once

#include "IObjectManager.h"
#include "BaseObject.h"
#include "SelectionGroup.h"

#include <AzCore/std/smart_ptr/unique_ptr.h>

// forward declarations.
class CAITerritoryObject;
class CAIWaveObject;
class CGizmoManager;
class CCameraObject;
class CEntityObject;
class CObjectArchive;
class CObjectLayer;
class CObjectClassDesc;
enum class ImageRotationDegrees;


namespace AZ
{
    namespace LegacyConversion
    {
        class Converter;
    }
}

//////////////////////////////////////////////////////////////////////////
// Helper class to signal when we are exportin a level to game
//////////////////////////////////////////////////////////////////////////
class CObjectManagerLevelIsExporting
{
public:
    CObjectManagerLevelIsExporting()
    {
        GetIEditor()->GetObjectManager()->SetExportingLevel(true);
    }

    ~CObjectManagerLevelIsExporting()
    {
        GetIEditor()->GetObjectManager()->SetExportingLevel(false);
    }
};

//////////////////////////////////////////////////////////////////////////
// Array of editor objects.
//////////////////////////////////////////////////////////////////////////
class CBaseObjectsCache
{
public:
    int GetObjectCount() const { return m_objects.size(); }
    CBaseObject* GetObject(int nIndex) const { return m_objects[nIndex]; }
    void AddObject(CBaseObject* pObject) { m_objects.push_back(pObject); }
    void ClearObjects() { m_objects.clear(); }
    void Reserve(int nCount) { m_objects.reserve(nCount); }

    /// Checksum is used as a dirty flag.  
    unsigned int GetSerialNumber() { return m_serialNumber; }
    void SetSerialNumber(unsigned int serialNumber) { m_serialNumber = serialNumber; }
private:
    //! List of objects that was displayed at last frame.
    std::vector<_smart_ptr<CBaseObject> > m_objects;
    unsigned int m_serialNumber = 0;
};

/*!
 *  CObjectManager is a singleton object that
 *  manages global set of objects in level.
 */
class CObjectManager
    : public IObjectManager
{
public:
    //! Selection functor callback.
    //! Callback function must return a boolean value.
    //! Return true if selection should proceed, or false to abort object selection.
    CObjectManager();
    ~CObjectManager();

    void RegisterObjectClasses();

    CBaseObject* NewObject(CObjectClassDesc* cls, CBaseObject* prev = 0, const QString& file = "", const char* newObjectName = nullptr);
    CBaseObject* NewObject(const QString& typeName, CBaseObject* prev = 0, const QString& file = "", const char* newEntityName = nullptr);

    void    DeleteObject(CBaseObject* obj);
    void    DeleteSelection(CSelectionGroup* pSelection);
    void    DeleteAllObjects();
    CBaseObject* CloneObject(CBaseObject* obj);

    void BeginEditParams(CBaseObject* obj, int flags);
    void EndEditParams(int flags = 0);
    // Hides all transform manipulators.
    void HideTransformManipulators();

    //! Get number of objects manager by ObjectManager (not contain sub objects of groups).
    int     GetObjectCount() const;

    //! Get array of objects, managed by manager (not contain sub objects of groups).
    //! @param layer if 0 get objects for all layers, or layer to get objects from.
    void GetObjects(CBaseObjectsArray& objects, const CObjectLayer* layer = 0) const;
    void GetObjects(DynArray<CBaseObject*>& objects, const CObjectLayer* layer = 0) const;

    //! Get array of objects that pass the filter.
    //! @param filter The filter functor, return true if you want to get the certain obj, return false if want to skip it.
    void    GetObjects(CBaseObjectsArray& objects, BaseObjectFilterFunctor const& filter) const;

    //! Get all camera sources in object manager
    void    GetCameras(std::vector<CCameraObject*>& objects);

    //! Update objects.
    void    Update();

    //! Display objects on display context.
    void    Display(DisplayContext& dc);

    //! Called when selecting without selection helpers - this is needed since
    //! the visible object cache is normally not updated when not displaying helpers.
    void ForceUpdateVisibleObjectCache(DisplayContext& dc);

    //! Check intersection with objects.
    //! Find intersection with nearest to ray origin object hit by ray.
    //! If distance tollerance is specified certain relaxation applied on collision test.
    //! @return true if hit any object, and fills hitInfo structure.
    bool HitTest(HitContext& hitInfo);

    //! Check intersection with an object.
    //! @return true if hit, and fills hitInfo structure.
    bool HitTestObject(CBaseObject* obj, HitContext& hc);

    //! Send event to all objects.
    //! Will cause OnEvent handler to be called on all objects.
    void    SendEvent(ObjectEvent event);

    //! Send event to all objects within given bounding box.
    //! Will cause OnEvent handler to be called on objects within bounding box.
    void    SendEvent(ObjectEvent event, const AABB& bounds);

    //////////////////////////////////////////////////////////////////////////
    //! Find object by ID.
    CBaseObject* FindObject(REFGUID guid) const;
    //////////////////////////////////////////////////////////////////////////
    //! Find object by name.
    CBaseObject* FindObject(const QString& sName) const;
    //////////////////////////////////////////////////////////////////////////
    //! Find objects of given type.
    void FindObjectsOfType(const QMetaObject* pClass, std::vector<CBaseObject*>& result) override;
    void FindObjectsOfType(ObjectType type, std::vector<CBaseObject*>& result) override;
    //////////////////////////////////////////////////////////////////////////
    //! Find objects which intersect with a given AABB.
    virtual void FindObjectsInAABB(const AABB& aabb, std::vector<CBaseObject*>& result) const;


    //////////////////////////////////////////////////////////////////////////
    // Find object from in game physical entity.
    CBaseObject* FindPhysicalObjectOwner(struct IPhysicalEntity* pPhysicalEntity);

    //////////////////////////////////////////////////////////////////////////
    //! Find BaseObject who is owner of specified animation node.
    CBaseObject*    FindAnimNodeOwner(CTrackViewAnimNode* pNode) const;

    //////////////////////////////////////////////////////////////////////////
    // Operations on objects.
    //////////////////////////////////////////////////////////////////////////
    //! Makes object visible or invisible.
    void HideObject(CBaseObject* obj, bool hide);
    //! Shows the last hidden object based on hidden ID
    void ShowLastHiddenObject();
    //! Freeze object, making it unselectable.
    void FreezeObject(CBaseObject* obj, bool freeze);
    //! Unhide all hidden objects.
    void UnhideAll();
    //! Unfreeze all frozen objects.
    void UnfreezeAll();

    //////////////////////////////////////////////////////////////////////////
    // Object Selection.
    //////////////////////////////////////////////////////////////////////////
    bool    SelectObject(CBaseObject* obj, bool bUseMask = true);
    void    UnselectObject(CBaseObject* obj);

    //! Select objects within specified distance from given position.
    //! Return number of selected objects.
    int SelectObjects(const AABB& box, bool bUnselect = false);

    virtual void SelectEntities(std::set<CEntityObject*>& s);

    int MoveObjects(const AABB& box, const Vec3& offset, ImageRotationDegrees rotation, bool bIsCopy = false);

    //! Selects/Unselects all objects within 2d rectangle in given viewport.
    void SelectObjectsInRect(CViewport* view, const QRect& rect, bool bSelect);
    void FindObjectsInRect(CViewport* view, const QRect& rect, std::vector<GUID>& guids);

    //! Clear default selection set.
    //! @Return number of objects removed from selection.
    int ClearSelection();

    //! Deselect all current selected objects and selects object that were unselected.
    //! @Return number of selected objects.
    int InvertSelection();

    //! Get current selection.
    CSelectionGroup*    GetSelection() const { return m_currSelection; };
    //! Get named selection.
    CSelectionGroup*    GetSelection(const QString& name) const;
    // Get selection group names
    void GetNameSelectionStrings(QStringList& names);
    //! Change name of current selection group.
    //! And store it in list.
    void    NameSelection(const QString& name);
    //! Set one of name selections as current selection.
    void    SetSelection(const QString& name);
    void    RemoveSelection(const QString& name);
    //! Checks for changes to the current selection and makes adjustments accordingly
    void    CheckAndFixSelection() override;

    bool IsObjectDeletionAllowed(CBaseObject* pObject);

    //! Delete all objects in selection group.
    void DeleteSelection();

    uint32  ForceID() const{return m_ForceID; }
    void    ForceID(uint32 FID){m_ForceID = FID; }

    //! Generates uniq name base on type name of object.
    QString GenerateUniqueObjectName(const QString& typeName);
    //! Register object name in object manager, needed for generating uniq names.
    void RegisterObjectName(const QString& name);
    //! Decrease name number and remove if it was last in object manager, needed for generating uniq names.
    void UpdateRegisterObjectName(const QString& name);
    //! Enable/Disable generating of unique object names (Enabled by default).
    //! Return previous value.
    bool EnableUniqObjectNames(bool bEnable);

    //! Register XML template of runtime class.
    void    RegisterClassTemplate(const XmlNodeRef& templ);
    //! Load class templates for specified directory,
    void    LoadClassTemplates(const QString& path);

    //! Registers the ObjectManager's console variables.
    void RegisterCVars();

    //! Find object class by name.
    CObjectClassDesc* FindClass(const QString& className);
    void    GetClassCategories(QStringList& categories);
    void    GetClassCategoryToolClassNamePairs(std::vector< std::pair<QString, QString> >& categoryToolClassNamePairs) override;
    void    GetClassTypes(const QString& category, QStringList& types);

    //! Export objects to xml.
    //! When onlyShared is true ony objects with shared flags exported, overwise only not shared object exported.
    void    Export(const QString& levelPath, XmlNodeRef& rootNode, bool onlyShared);
    void    ExportEntities(XmlNodeRef& rootNode);

    //! Serialize Objects in manager to specified XML Node.
    //! @param flags Can be one of SerializeFlags.
    void    Serialize(XmlNodeRef& rootNode, bool bLoading, int flags = SERIALIZE_ALL);

    void SerializeNameSelection(XmlNodeRef& rootNode, bool bLoading);

    //! Load objects from object archive.
    //! @param bSelect if set newly loaded object will be selected.
    void LoadObjects(CObjectArchive& ar, bool bSelect);

    //! Delete from Object manager all objects without SHARED flag.
    void    DeleteNotSharedObjects();
    //! Delete from Object manager all objects with SHARED flag.
    void    DeleteSharedObjects();

    bool AddObject(CBaseObject* obj);
    void RemoveObject(CBaseObject* obj);
    void ChangeObjectId(REFGUID oldId, REFGUID newId);
    void NotifyPrefabObjectChanged(CBaseObject* pObject);
    bool IsDuplicateObjectName(const QString& newName) const
    {
        return FindObject(newName) ? true : false;
    }
    void ShowDuplicationMsgWarning(CBaseObject* obj, const QString& newName, bool bShowMsgBox) const;
    void ChangeObjectName(CBaseObject* obj, const QString& newName);

    //! Convert object of one type to object of another type.
    //! Original object is deleted.
    bool ConvertToType(CBaseObject* pObject, const QString& typeName);

    //! Set new selection callback.
    //! @return previous selection callback.
    IObjectSelectCallback* SetSelectCallback(IObjectSelectCallback* callback);

    // Enables/Disables creating of game objects.
    void SetCreateGameObject(bool enable) { m_createGameObjects = enable; };
    //! Return true if objects loaded from xml should immidiatly create game objects associated with them.
    bool IsCreateGameObjects() const { return m_createGameObjects; };

    //////////////////////////////////////////////////////////////////////////
    //! Get access to gizmo manager.
    IGizmoManager* GetGizmoManager();

    //////////////////////////////////////////////////////////////////////////
    //! Get acess to object layers manager.
    CObjectLayerManager* GetLayersManager() const { return m_pLayerManager; }

    //////////////////////////////////////////////////////////////////////////
    //! Invalidate visibily settings of objects.
    void InvalidateVisibleList();

    //////////////////////////////////////////////////////////////////////////
    // ObjectManager notification Callbacks.
    //////////////////////////////////////////////////////////////////////////
    void AddObjectEventListener(const EventCallback& cb);
    void RemoveObjectEventListener(const EventCallback& cb);

    //////////////////////////////////////////////////////////////////////////
    // Used to indicate starting and ending of objects loading.
    //////////////////////////////////////////////////////////////////////////
    void StartObjectsLoading(int numObjects);
    void EndObjectsLoading();

    //////////////////////////////////////////////////////////////////////////
    // Gathers all resources used by all objects.
    void GatherUsedResources(CUsedResources& resources, CObjectLayer* pLayer = 0);

    //////////////////////////////////////////////////////////////////////////
    IStatObj* GetGeometryFromObject(CBaseObject* pObject);
    ICharacterInstance* GetCharacterFromObject(CBaseObject* pObject);

    // Called when object gets modified.
    void OnObjectModified(CBaseObject* pObject, bool bDelete, bool boModifiedTransformOnly);

    void UnregisterNoExported();
    void RegisterNoExported();

    virtual bool IsEntityAssignedToSelectedTerritory(CEntityObject* pEntity);
    virtual bool IsEntityAssignedToSelectedWave     (CEntityObject* pEntity);

    virtual bool IsLightClass(CBaseObject* pObject);

    virtual void SelectAssignedEntities();
    virtual size_t NumberOfAssignedEntities();

    virtual void FindAndRenameProperty2(const char* property2Name, const QString& oldValue, const QString& newValue);
    virtual void FindAndRenameProperty2If(const char* property2Name, const QString& oldValue, const QString& newValue, const char* otherProperty2Name, const QString& otherValue);

    virtual void AssignLayerIDsToRenderNodes();

    void ResolveMissingObjects();
    void ResolveMissingMaterials();

    class CObjectPhysicsManager* GetPhysicsManager()
    { return m_pPhysicsManager; }

    bool IsReloading() const { return m_bInReloading; }
    void SetSkipUpdate(bool bSkipUpdate) override { m_bSkipObjectUpdate = bSkipUpdate; }

    void SetExportingLevel(bool bExporting) override { m_bLevelExporting = bExporting; }
    bool IsExportingLevelInprogress() const override { return m_bLevelExporting; }

    int GetAxisHelperHitRadius() const override { return m_axisHelperHitRadius; }

private:
    friend CObjectArchive;
    friend class CBaseObject;
    /** Creates and serialize object from xml node.
    @param objectNode Xml node to serialize object info from.
    @param pUndoObject Pointer to deleted object for undo.
    */
    CBaseObject* NewObject(CObjectArchive& archive, CBaseObject* pUndoObject, bool bMakeNewId);

    //! Update visibility of all objects.
    void UpdateVisibilityList();
    //! Get array of all objects in manager.
    void GetAllObjects(TBaseObjects& objects) const;

    void UnselectCurrent();
    void SelectCurrent();
    void SetObjectSelected(CBaseObject* pObject, bool bSelect);

    // Recursive functions potentially taking into child objects into account
    void SelectObjectInRect(CBaseObject* pObj, CViewport* view, HitContext hc, bool bSelect);
    void HitTestObjectAgainstRect(CBaseObject* pObj, CViewport* view, HitContext hc, std::vector<GUID>& guids);

    void SaveRegistry();
    void LoadRegistry();

    void NotifyObjectListeners(CBaseObject* pObject, CBaseObject::EObjectListenerEvent event);

    void FindDisplayableObjects(DisplayContext& dc, bool bDisplay);

    void UpdateAttachedEntities();

private:
    typedef std::map<GUID, CBaseObjectPtr, guid_less_predicate> Objects;
    Objects m_objects;
    typedef std::unordered_map<AZ::u32, CBaseObjectPtr> ObjectsByNameCrc;
    ObjectsByNameCrc m_objectsByName;

    typedef std::map<QString, CSelectionGroup*> TNameSelectionMap;
    TNameSelectionMap m_selections;

    //! Used for forcing IDs of "GetEditorObjectID" of PreFabs, as they used to have random IDs on each load
    uint32  m_ForceID;

    //! Array of currently visible objects.
    TBaseObjects m_visibleObjects;
    
    // this number changes whenever visibility is invalidated.  Viewports can use it to keep track of whether they need to recompute object
    // visibility.
    unsigned int m_visibilitySerialNumber = 1; 
    unsigned int m_lastComputedVisibility = 0; // when the object manager itself last updated visibility (since it also has a cache)
    int m_lastHideMask = 0;

    float m_maxObjectViewDistRatio;

    //////////////////////////////////////////////////////////////////////////
    // Selection.
    //! Current selection group.
    CSelectionGroup* m_currSelection;
    int m_nLastSelCount;
    bool m_bSelectionChanged;
    IObjectSelectCallback* m_selectCallback;
    bool m_bLoadingObjects;

    //! Default selection.
    CSelectionGroup m_defaultSelection;

    CBaseObjectPtr m_currEditObject;
    bool m_bSingleSelection;

    bool m_createGameObjects;
    bool m_bGenUniqObjectNames;

    // Object manager also handles Gizmo manager.
    CGizmoManager* m_gizmoManager;

    friend CObjectLayerManager;
    //! Manager of object layers.
    CObjectLayerManager* m_pLayerManager;

    //////////////////////////////////////////////////////////////////////////
    // Loading progress.
    CWaitProgress* m_pLoadProgress;
    int m_loadedObjects;
    int m_totalObjectsToLoad;
    //////////////////////////////////////////////////////////////////////////

    class CObjectPhysicsManager* m_pPhysicsManager;

    //////////////////////////////////////////////////////////////////////////
    // Numbering for names.
    //////////////////////////////////////////////////////////////////////////
    typedef std::map<QString, std::set<uint16>, stl::less_stricmp<QString> > NameNumbersMap;
    NameNumbersMap m_nameNumbersMap;

    //////////////////////////////////////////////////////////////////////////
    // Listeners.
    std::list<EventCallback> m_objectEventListeners;

    bool m_bExiting;

    // Convenience access to territory and wave objects in m_objects
    std::set<CAITerritoryObject*>   m_aiTerritoryObjects;
    std::set<CAIWaveObject*>        m_aiWaveObjects;

    std::set<CEntityObject*> m_setEntitiesAssignedToSelectedTerritory;
    std::set<CEntityObject*> m_setEntitiesAssignedToSelectedWave;
    void RefreshEntitiesAssignedToSelectedTnW();

    std::unordered_set<CEntityObject*> m_animatedAttachedEntities;

    bool m_isUpdateVisibilityList;

    uint64 m_currentHideCount;

    bool m_bInReloading;
    bool m_bSkipObjectUpdate;
    bool m_bLevelExporting;

    AZStd::unique_ptr<AZ::LegacyConversion::Converter> m_converter;

    int m_axisHelperHitRadius = 20;
};

#endif // CRYINCLUDE_EDITOR_OBJECTS_OBJECTMANAGER_H
