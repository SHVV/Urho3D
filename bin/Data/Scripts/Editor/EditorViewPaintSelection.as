const int PAINT_STEP_UPDATE = 16;
bool EditorPaintSelectionShow = false;
int EditorPaintSelectionUITimeToUpdate = 0;

UIElement@ EditorPaintSelectionUIContainer = null;
BorderImage@ paintSelectionImage = null;

IntVector2 psDefaulSize(96,96);
IntVector2 psCurrentSize = psDefaulSize;
IntVector2 psMinSize(64,64);
IntVector2 psMaxSize(512,512);
IntVector2 psStepSizeChange(16,16);

void CreatePaintSelectionContainer()
{
    if (editorScene is null) return;
    EditorPaintSelectionUIContainer = UIElement();
    EditorPaintSelectionUIContainer.position = IntVector2(0,0);
    EditorPaintSelectionUIContainer.size = IntVector2(graphics.width,graphics.height);
    EditorPaintSelectionUIContainer.priority = -5;
    EditorPaintSelectionUIContainer.focusMode = FM_NOTFOCUSABLE;
    EditorPaintSelectionUIContainer.bringToBack = true;
    EditorPaintSelectionUIContainer.name ="DebugPaintSelectionContainer";
    EditorPaintSelectionUIContainer.temporary = true;
    ui.root.AddChild(EditorPaintSelectionUIContainer);
}

void CreatePaintSelectionTool()
{      
    paintSelectionImage = BorderImage("Icon");
    paintSelectionImage.temporary = true;
    paintSelectionImage.SetFixedSize(psDefaulSize.x,psDefaulSize.y);
    paintSelectionImage.texture = cache.GetResource("Texture2D", "Textures/Editor/SelectionCircle.png");
    paintSelectionImage.imageRect = IntRect(0,0,512,512);
    paintSelectionImage.priority = -5;
    paintSelectionImage.color = Color(1,1,1);
    paintSelectionImage.bringToBack = true;
    paintSelectionImage.enabled = false;
    paintSelectionImage.selected = false;
    paintSelectionImage.visible = true;
    EditorPaintSelectionUIContainer.AddChild(paintSelectionImage);
}

void UpdatePaintSelection()
{
    PaintSelectionCheckKeyboard();
        
    // Early out if are disabled
    if (!EditorPaintSelectionShow) return; 
          
    if (editorScene is null || EditorPaintSelectionUITimeToUpdate > time.systemTime) return;
    
    EditorPaintSelectionUIContainer = ui.root.GetChild("DebugPaintSelectionContainer");
    
    if (EditorPaintSelectionUIContainer is null)
    {
        CreatePaintSelectionContainer();
        CreatePaintSelectionTool();
    }
    
    if (EditorPaintSelectionUIContainer !is null) 
    {
        // Set visibility for all origins
        EditorPaintSelectionUIContainer.visible = EditorPaintSelectionShow;
                 
        if (viewportMode!=VIEWPORT_SINGLE) 
            EditorPaintSelectionUIContainer.visible = false;
            
        if (EditorPaintSelectionShow) 
        {
                IntVector2 mp = input.mousePosition;
                paintSelectionImage.position = IntVector2(mp.x - (psCurrentSize.x * 0.5f), mp.y - (psCurrentSize.y * 0.5f));
        }
    }
    
    EditorPaintSelectionUITimeToUpdate = time.systemTime + PAINT_STEP_UPDATE;
}

void PaintSelectionCheckKeyboard() 
{    
    
    bool key = input.keyPress[KEY_C];    
    
    if (key && ui.focusElement is null)
    {
        EditorPaintSelectionShow = !EditorPaintSelectionShow;
        if (EditorPaintSelectionUIContainer !is null)
            EditorPaintSelectionUIContainer.visible = EditorPaintSelectionShow;
        
            
        if (EditorPaintSelectionShow) 
        {
            // When we start paint selection we change editmode to select
            editMode = EDIT_SELECT;
            // and also we show origins for proper origins update
            ShowOrigins(true);
            toolBarDirty = true;
        }
    }
    else if (EditorPaintSelectionShow && ui.focusElement is null)  
    {
        if (editMode != EDIT_SELECT) 
        {
            EditorPaintSelectionShow = false;
            EditorPaintSelectionUIContainer.visible = false;
        }         
    }
    
    if (input.mouseButtonDown[MOUSEB_RIGHT])
    {
        EditorPaintSelectionShow = false;
        if (EditorPaintSelectionUIContainer !is null)
            EditorPaintSelectionUIContainer.visible = false;
    }    
}

void SelectOriginsByPaintSelection(IntVector2 curPos, float brushRadius, bool isOperationAddToSelection = true) 
{
    if (!EditorPaintSelectionShow || EditorPaintSelectionUIContainer is null) return;
    
    if (isOperationAddToSelection) 
    {
        for (int i=0; i < originsNodes.length; i++) 
        {
            Vector3 v1(originsIcons[i].position.x, originsIcons[i].position.y, 0);
            Vector3 v2(curPos.x - ORIGINOFFSETICON.x, curPos.y - ORIGINOFFSETICON.y, 0); 
            
            float distance = (v1 - v2).length;
            bool isThisOriginInCircle = distance < brushRadius ? true : false;
            int nodeID = originsIcons[i].vars[ORIGIN_NODEID_VAR].GetInt();
            
            if (isThisOriginInCircle) 
            {    
                WeakHandle handle = editorScene.GetNode(nodeID);
                if (handle.Get() !is null) 
                {
                    Node@ node = handle.Get();
                    if (node !is null && isThisNodeOneOfSelected(node) == false) 
                    {
                        SelectNode(node, true);
                    }
                }
            }
            
        }
    }
    else // Deselect origins 
    {
        for (int i=0; i < originsNodes.length; i++) 
        {
            Vector3 v1(originsIcons[i].position.x, originsIcons[i].position.y, 0);
            Vector3 v2(curPos.x - ORIGINOFFSETICON.x, curPos.y - ORIGINOFFSETICON.y, 0); 
            
            float distance = (v1 - v2).length;
            bool isThisOriginInCircle = distance < brushRadius ? true : false;
            int nodeID = originsIcons[i].vars[ORIGIN_NODEID_VAR].GetInt();
            
            if (isThisOriginInCircle) 
            {    
                WeakHandle handle = editorScene.GetNode(nodeID);
                if (handle.Get() !is null) 
                {
                    Node@ node = handle.Get();
                    if (node !is null && isThisNodeOneOfSelected(node) == true) 
                    {
                        DeselectNode(node);
                    }
                }
            }
            
        }    
    }
}

int GetIndexInSelectedNodesArray(int nodeID) 
{
    if (!selectedNodes.empty)
    {
        for (int i=0; i < selectedNodes.length; i++) 
        {
            if (selectedNodes[i].id == nodeID)
                return i;
        }
    }   

    return -1;
}

void HandlePaintSelectionMouseMove(StringHash eventType, VariantMap& eventData)
{
    if (!EditorPaintSelectionShow || EditorPaintSelectionUIContainer is null) return;
        
    int x = eventData["X"].GetInt();
    int y = eventData["Y"].GetInt();
    float r = (psCurrentSize.x * 0.5);
    
    IntVector2 mousePos(x,y);
    if (input.mouseButtonDown[MOUSEB_LEFT] && input.qualifierDown[QUAL_ALT] != true)
        SelectOriginsByPaintSelection(mousePos, r, true);
    else if (input.mouseButtonDown[MOUSEB_LEFT] && input.qualifierDown[QUAL_ALT] == true) 
    {
        SelectOriginsByPaintSelection(mousePos, r, false);
    } 
}

void HandlePaintSelectionWheel(StringHash eventType, VariantMap& eventData)
{
    if (!EditorPaintSelectionShow || EditorPaintSelectionUIContainer is null) return;
    
    int wheelValue = eventData["Wheel"].GetInt();
    
    if (wheelValue != 0)
    {
        if (wheelValue > 0)
        {
            
            psCurrentSize = psCurrentSize - psStepSizeChange;
            psCurrentSize = IntVector2(Max(psCurrentSize.x, psMinSize.x), Max(psCurrentSize.y, psMinSize.y)); 
        }
        else if (wheelValue < 0)
        {
            psCurrentSize = psCurrentSize + psStepSizeChange;
            psCurrentSize = IntVector2(Min(psCurrentSize.x, psMaxSize.x), Min(psCurrentSize.y, psMaxSize.y)); 
        }
        
        paintSelectionImage.SetFixedSize(psCurrentSize.x, psCurrentSize.y);
    }
}

void HandlePaintSelectionOriginHoverBegin(StringHash eventType, VariantMap& eventData)
{
    UIElement@ origin = eventData["Element"].GetPtr();
    if (origin is null)
        return;
    
    int OriginArrayID = eventData["Id"].GetInt();
    int NodeID = eventData["NodeId"].GetInt();
        
}

void HandlePaintSelectionOriginHoverEnd(StringHash eventType, VariantMap& eventData)
{
    UIElement@ origin = eventData["Element"].GetPtr();
    if (origin is null)
        return;
    
    int OriginArrayID = eventData["Id"].GetInt();
    int NodeID = eventData["NodeId"].GetInt();
   
}