const bool DEFAULT_SHOW_NAMES_FOR_ALL = false;
const int MAX_ORIGIN = 128;
const int ORIGIN_STEP_UPDATE = 16;
const int ORIGIN_STEP_SCENE_GET_NODES = 3000;
const int NAMES_SIZE = 10;
const StringHash ORIGIN_NODEID_VAR("OriginNodeID");
const Color ORIGIN_COLOR(1.0f,1.0f,1.0f,1.0f);
const Color ORIGIN_COLOR_SELECTED(0.0f,1.0f,1.0f,1.0f);
const Color ORIGIN_COLOR_TEXT(1.0f,1.0f,1.0f,0.3f);
const Color ORIGIN_COLOR_SELECTED_TEXT(1.0f,1.0f,1.0f,1.0f);
const IntVector2 ORIGIN_ICON_SIZE(12,12);
const IntVector2 ORIGIN_ICON_SIZE_SELECTED(15,15);
const float ORIGINS_VISIBLITY_RANGE = 128.0f;

bool extraInfo = false;
bool extraInfoHide = false;
bool showNamesForAll = DEFAULT_SHOW_NAMES_FOR_ALL;
bool EditorOriginShow = true;
bool rebuildSceneOrigins = true;
bool isOriginsHovered = false;

int EditorOriginUITimeToUpdate = 0;
int EditorOriginUITimeToSceneNodeRead = 0;
int prevSelectedID;
int selectedNodeInfoState = 0;
int originHoveredIndex = -1;

UIElement@ EditorOriginUIContainer = null;
Text@ selectedNodeName = null;
BorderImage@ selectedNodeOrigin = null;

Array<BorderImage@> selectedNodeOriginChilds;
Array<Text@> selectedNodeNameChilds;
Array<Node@> originsNodes;
Array<BorderImage@> originsIcons;
Array<Text@> originsNames;

void CreateOriginsContainer()
{
    if (editorScene is null) return;
    EditorOriginUIContainer = UIElement();
    EditorOriginUIContainer.position = IntVector2(0,0);
    EditorOriginUIContainer.size = IntVector2(graphics.width,graphics.height);
    EditorOriginUIContainer.priority = -10;
    EditorOriginUIContainer.focusMode = FM_NOTFOCUSABLE;
    EditorOriginUIContainer.bringToBack = true;
    EditorOriginUIContainer.name ="DebugOriginsContainer";
    EditorOriginUIContainer.temporary = true;
    ui.root.AddChild(EditorOriginUIContainer);
}

void HandleOriginToggled(StringHash eventType, VariantMap& eventData) 
{
    UIElement@ origin = eventData["Element"].GetPtr();
    if (origin is null)
        return;
    
    if (IsSceneOrigin(origin))
    {
        int nodeID = origin.vars[ORIGIN_NODEID_VAR].GetInt();
        if (editorScene !is null) 
        {
            bool goBackAndSelectNodeParent = input.qualifierDown[QUAL_CTRL];
            
            WeakHandle handle = editorScene.GetNode(nodeID);
            if (handle.Get() !is null) {
                Node@ selectedNodeByOrigin = handle.Get();
                if (selectedNodeByOrigin !is null) 
                {
                    if (goBackAndSelectNodeParent)
                        SelectNode(selectedNodeByOrigin.parent, false);
                    else
                        SelectNode(selectedNodeByOrigin, false);
                }
            }
        }
    }
}

void UpdateOrigins()
{
    // Early out if Origins are disabled
    if (!EditorOriginShow) return; 
    
    CheckKeyboardQualifers();
        
    if (editorScene is null || EditorOriginUITimeToUpdate > time.systemTime) return;
    
    EditorOriginUIContainer = ui.root.GetChild("DebugOriginsContainer");
    
    // Since editor not clear UIs then do loading new scenes, this creation called once per Editor's starting event
    // for other scenes we use the same container
    if (EditorOriginUIContainer is null)
    {
        CreateOriginsContainer();
    }
    
    if (EditorOriginUIContainer !is null) 
    {
        // Set visibility for all origins
        EditorOriginUIContainer.visible = EditorOriginShow;
                 
        if (viewportMode!=VIEWPORT_SINGLE) 
            EditorOriginUIContainer.visible = false;
        
        // Forced read nodes for some reason:
        if ((originsNodes.length < 1) || rebuildSceneOrigins || (EditorOriginUITimeToSceneNodeRead < time.systemTime)) 
        {
            originsNodes = editorScene.GetChildren(true);
            
            // If we are hot have free origins icons in arrays, resize x 2
            if (originsIcons.length < originsNodes.length) 
            {
                EditorOriginUIContainer.RemoveAllChildren();
                originsIcons.Clear();
                originsNames.Clear();
                
                originsIcons.Resize(originsNodes.length * 2);
                originsNames.Resize(originsNodes.length * 2);
                
                if (originsIcons.length > 0)
                {
                    for (int i=0; i < originsIcons.length; i++)
                    {
                        CreateOrigin(i, false);
                    }
                }
            }
            
            // If this rebuild pass after new scene loading reset flag
            if (rebuildSceneOrigins) 
                rebuildSceneOrigins = false;
            
            // if this grab pass or else update time
            EditorOriginUITimeToSceneNodeRead =  time.systemTime + ORIGIN_STEP_SCENE_GET_NODES;
        }
        
        if (originsNodes.length > 0)
        {   
            // Get selected node for feeding proper arrray's UIElements with slyte colorig and additional info on ALT
            Node@ selectedNode = null;
            if (selectedNodes.length > 0) 
            {
                selectedNode = selectedNodes[0];
            } 
            else if (selectedComponents.length > 0)
            {
                selectedNode = selectedComponents[0].node;
            }
            
            // Update existed origins (every 10 ms)
            if (originsNodes.length > 0 )
            {
                for (int i=0; i < originsNodes.length; i++)
                {
                    Vector3 eyeDir = originsNodes[i].worldPosition - cameraNode.worldPosition;
                    float distance = (eyeDir).length;
                    eyeDir.Normalize();
                    Vector3 cameraDir = (cameraNode.worldRotation * Vector3(0.0f, 0.0f, 1.0f)).Normalized();
                    float angleCameraDirVsDirToNode = eyeDir.DotProduct(cameraDir);
                    
                    // if node in range and in camera view (clip back sibe)
                    if (distance < ORIGINS_VISIBLITY_RANGE && angleCameraDirVsDirToNode > 0.7f)
                    {
                        // turn on origin and move
                        MoveOrigin(i, true);
                        
                        if (originsNodes[i] is selectedNode) 
                        {
                            ShowSelectedNodeOrigin(selectedNode, i);
                            originsNames[i].visible = true;
                        }
                        else 
                        { 
                            if (showNamesForAll || (isOriginsHovered && originHoveredIndex == i)) 
                                originsNames[i].text = NodeInfo(originsNodes[i], selectedNodeInfoState);
                        }
                    }
                    else 
                    {
                        // turn-off origin
                        VisibilityOrigin(i, false);
                    }
                }
                
                // Hide non used origins
                for (int j=originsNodes.length; j < originsIcons.length; j++)
                {
                    VisibilityOrigin(j, false);
                }
            }  
        }
    }
    
    
    EditorOriginUITimeToUpdate = time.systemTime + ORIGIN_STEP_UPDATE;
}

void ShowSelectedNodeOrigin(Node@ node, int index) 
{
    if (node !is null)
    {
        // just keep node's text and node's origin icon position in actual view
        Viewport@ vp = activeViewport.viewport;
        Vector2 sp = activeViewport.camera.WorldToScreenPoint(node.worldPosition); 
        originsIcons[index].position = IntVector2(10+int(vp.rect.left + sp.x * vp.rect.right), -5 + int(vp.rect.top + sp.y* vp.rect.bottom));
        originsIcons[index].position = IntVector2(int(vp.rect.left + sp.x * vp.rect.right) - 7, int(vp.rect.top + sp.y* vp.rect.bottom) - 7);
        originsNames[index].color = ORIGIN_COLOR_SELECTED_TEXT;
        originsIcons[index].color = ORIGIN_COLOR_SELECTED;
        originsIcons[index].SetFixedSize(ORIGIN_ICON_SIZE_SELECTED.x,ORIGIN_ICON_SIZE_SELECTED.y);
        
        // if we need to update info for some reason
        if (extraInfo || extraInfoHide || (prevSelectedID != node.id))
        {
            if (prevSelectedID != node.id)
            {
                // NEW ORIGIN SELECTION HAS: INIT DEFAULTS 
                // save node ID to prevent update info each frame for same selected node
                prevSelectedID = node.id;
                selectedNodeInfoState = 0;
                originsIcons[index].vars[ORIGIN_NODEID_VAR] = node.id;
                //showNamesForAll = DEFAULT_SHOW_NAMES_FOR_ALL;
            }  
            
            Array<Component@> components = node.GetComponents(); 
            Array<String> componentsShortInfo;
            Array<String> componentsDetailInfo;
            componentsShortInfo.Resize(components.length);
            componentsDetailInfo.Resize(components.length);
            
            // Add std info node name + tags
            originsNames[index].text = NodeInfo(node, selectedNodeInfoState) + "\n";

            // Add short info and detail info
            for (int i=0; i < components.length; i++ )
            {
                // Get SM, but also works well for AnimatedModel
                StaticModel@ sm = cast<StaticModel>(components[i]);
                if (sm !is null) 
                {
                    // Process info for Model 
                    CollectStaticModelInfo(sm, componentsShortInfo[i], componentsDetailInfo[i], selectedNodeInfoState);                            
                    originsNames[index].text = originsNames[index].text + 
                    (selectedNodeInfoState > 0 ? componentsShortInfo[i] +"\n" : "") + (selectedNodeInfoState > 1 ? componentsDetailInfo[i]: ""); 
                    continue;
                }
                
                // TODO: Add info view for other components      
            }                    
        }
    }
}

void CreateOrigin(int index, bool isVisible = false) 
{
    if (originsIcons.length < index) return;
                
    originsIcons[index] = BorderImage("Icon");
    originsIcons[index].temporary = true;
    originsIcons[index].SetFixedSize(ORIGIN_ICON_SIZE.x,ORIGIN_ICON_SIZE.y);
    originsIcons[index].texture = cache.GetResource("Texture2D", "Textures/Editor/EditorIcons.png");
    originsIcons[index].imageRect = IntRect(0,0,14,14);
    originsIcons[index].priority = -300;
    originsIcons[index].color = ORIGIN_COLOR;
    originsIcons[index].bringToBack = true;
    originsIcons[index].enabled = true;
    originsIcons[index].selected = true;
    originsIcons[index].visible = isVisible;
    EditorOriginUIContainer.AddChild(originsIcons[index]);
    
    originsNames[index] = Text();
    originsNames[index].visible = false;
    originsNames[index].SetFont(cache.GetResource("Font", "Fonts/BlueHighway.ttf"), NAMES_SIZE);
    originsNames[index].color = ORIGIN_COLOR_TEXT;
    //originsNames[index].textEffect = TE_STROKE;
    originsNames[index].temporary = true;
    originsNames[index].bringToBack = true;
    originsNames[index].priority = -200;    
    originsNames[index].enabled = false;
    
    EditorOriginUIContainer.AddChild(originsNames[index]);
}

void MoveOrigin(int index, bool isVisible = false) 
{ 
    if (originsIcons.length < index) return;   
    if (originsIcons[index] is null) return;
    if (originsNodes[index].temporary)
    {
        originsIcons[index].visible = false;
        originsNames[index].visible = false;
        return;
    }
    
    Viewport@ vp = activeViewport.viewport;
    Vector2 sp = activeViewport.camera.WorldToScreenPoint(originsNodes[index].worldPosition); 
    
    originsIcons[index].SetFixedSize(ORIGIN_ICON_SIZE.x,ORIGIN_ICON_SIZE.y);
    originsIcons[index].color = ORIGIN_COLOR;
    originsIcons[index].position = IntVector2(int(vp.rect.left + sp.x * vp.rect.right) - 7, int(vp.rect.top + sp.y* vp.rect.bottom) - 7);
    originsIcons[index].visible = isVisible;
    originsIcons[index].vars[ORIGIN_NODEID_VAR] = originsNodes[index].id;
    
    originsNames[index].position = IntVector2(10+int(vp.rect.left + sp.x * vp.rect.right), -5 + int(vp.rect.top + sp.y* vp.rect.bottom));
    
    if (isOriginsHovered && originHoveredIndex == index)
    {
        originsNames[index].visible = true;
        originsNames[index].color = ORIGIN_COLOR_SELECTED_TEXT;
    }
    else 
    {
        originsNames[index].visible = showNamesForAll ? isVisible : false;
        originsNames[index].color = ORIGIN_COLOR_TEXT;
    }
}

void VisibilityOrigin(int index, bool isVisible = false) 
{
    originsIcons[index].visible = isVisible;
    originsNames[index].visible = isVisible;
}

bool IsSceneOrigin(UIElement@ element) 
{
    if (originsIcons.length < 1) return false;
    
    for (int i=0; i < originsIcons.length; i++) 
    {
        if (element is originsIcons[i])
        {
            originHoveredIndex = i;
            return true;
        }
    }
    
    originHoveredIndex = -1;
    return false;
}

void CheckKeyboardQualifers() 
{    
    // if pressed alt we inc state for info
    //extraInfo = input.qualifierPress[QUAL_ALT];
    extraInfo = input.keyPress[KEY_ALT];    
    if (extraInfo) 
        if (selectedNodeInfoState < 3) selectedNodeInfoState += 1;
    
    // if pressed ctrl we reset info state
    extraInfoHide = input.qualifierDown[QUAL_CTRL];
    if (extraInfoHide)
        selectedNodeInfoState = 0;
    
    bool hideChildName = input.qualifierPress[QUAL_SHIFT];
    if (hideChildName)
        showNamesForAll =!showNamesForAll;
    
}

String NodeInfo(Node& node, int st) 
{
    String result = "";
    if (node !is editorScene)
    {
        if (node.name.empty)
            result = "Node";
        else
            result = node.name;
    
        // Add node's tags if wey are exist
        if (st > 0 && node.tags.length > 0)
        {
            result = result + "\n[";
            for (int i=0;i<node.tags.length; i++) 
            {
                result = result + " " + node.tags[i];  
            }
            result = result + " ] ";
        }
    }
    else
        result = "Scene Origin";
    
    return result;
}

void CollectStaticModelInfo(StaticModel@ sm, String &fillShortInfo, String &fillDetailInfo, int st)
{
    Model@ m = sm.model;
    if (m !is null)
    {
        fillShortInfo = "    " + m.name;
        int modelFacesTotal = 0;
        for (int gIndex=0; gIndex<m.numGeometries; gIndex++)
        {
            int gIndexLodLevel = m.numGeometryLodLevels[gIndex]; // кол-во лодов геометрии
            fillDetailInfo = fillDetailInfo + "        Geometry: " + String(gIndex) + " [ ";
            
            // Add Vertex format to Geometry line
            Geometry@ g = m.GetGeometry(gIndex, 0);
            if (g !is null)
            {
                String vf = VertexFormatToString(g.vertexElementMasks[0]);
            
                fillDetailInfo = fillDetailInfo + vf + " ]" + "\n";
            }
            modelFacesTotal += g.indexCount / 3; // Get LOD0 Total faces
            
            // Add LODs Info lines
            for (int lodIndex = 0; lodIndex < gIndexLodLevel; lodIndex++)
            {
                Geometry@ g = m.GetGeometry(gIndex, lodIndex);
                if (g !is null)
                {
                    for (int vb=0; vb < g.numVertexBuffers; vb++)
                    {   
                        /// TODO: Add AS bingings for Model.lodBias for math actual LOD distance (lodDist = g.lodDistance * model.lodBias)
                        fillDetailInfo = fillDetailInfo + "            LOD: " + String(lodIndex) + " Distance: "+ String(g.lodDistance) + " numFaces: " + String((g.indexCount / 3)) + "\n";  
                    }                       
                }
            }
        }
        
        if (st <= 1)
            fillShortInfo = fillShortInfo + " LOD0 faces:" + String(modelFacesTotal);
    }
} 

String VertexFormatToString(int vertexMask) 
{
    String result = "";
    //int vertexMask = g.vertexElementMasks[0];
    if ((MASK_POSITION) & vertexMask != 0)
        result = result + "POS ";
    if ((MASK_NORMAL) & vertexMask != 0)
        result = result + "NORMAL ";
    if ((MASK_COLOR) & vertexMask != 0)
        result = result + "VCOL ";
    if ((MASK_TEXCOORD1) & vertexMask != 0)
        result = result + "UV1 ";
    if ((MASK_TEXCOORD2) & vertexMask != 0)
        result = result + "UV2 ";
    if ((MASK_CUBETEXCOORD1) & vertexMask != 0)
        result = result + "CUBEUV1 ";
    if ((MASK_CUBETEXCOORD2) & vertexMask != 0)
        result = result + "CUBEUV2 ";
    if ((MASK_TANGENT) & vertexMask != 0)
        result = result + "TANGENT ";
    if ((MASK_BLENDWEIGHTS) & vertexMask != 0)
        result = result + "BLENDWEIGHTS ";
    if ((MASK_BLENDINDICES) & vertexMask != 0)
        result = result + "BLENDINDICES ";
    if ((MASK_OBJECTINDEX) & vertexMask != 0)
        result = result + "OBJECTINDEX ";
    
    return result;
}

void HandleSceneLoadedForOrigins() 
{
    rebuildSceneOrigins = true;
}

void HandleOriginsHoverBegin(StringHash eventType, VariantMap& eventData)
{
    UIElement@ origin = eventData["Element"].GetPtr();
    if (origin is null)
        return;
    
    if (IsSceneOrigin(origin))
    {
        isOriginsHovered = true;
    }
}

void HandleOriginsHoverEnd(StringHash eventType, VariantMap& eventData)
{
    UIElement@ origin = eventData["Element"].GetPtr();
    if (origin is null)
        return;
    
    if (IsSceneOrigin(origin))
    {
        isOriginsHovered = false;
    }    
}