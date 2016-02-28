const int PROCSKY_STEP_UPDATE = 1000;
const int PROCSKY_STEP_RENDER = 100;
float timeToNextProcSkyUpdate = 0;
float timeToNextProcSkyRender = 0;

int psParam = 256;
float psFOV = 89.5f;
bool procSkyShow = true;
Node@ procSkyNode;
Node@ procSkyLightNode;
Quaternion procSkyLightNodeLastRotation;
Light@ procSkyLight;
Color procSkyLightColor;
Camera@ procSkyCamera;
Skybox@ procSkySkyBox;
Material@ procSkySkyBoxMaterial;
TextureCube@  procSkyCubeTexture;
RenderPath@ procSkyRenderPath;
int procSkyRenderSize = 256;
int procSkyRenderSizeLast = 0;
bool procSkyRenderQueued = false;
const int MAX_CUBEMAP_FACES = 6;
Array<Quaternion> procSkyFaceRotations;
Array<RenderPathCommand> cmd;

Vector3 Kr = Vector3(0.18867780436772762f, 0.4978442963618773f, 0.6616065586417131f); // Absorption profile of air.
float rayleighBrightness = 3.9f;    //3.3f
float mieBrightness = 0.01f;        //0.1f
float spotBrightness = 50.0f;       //50.0f
float scatterStrength = 0.028f;     //0.028f 
float rayleighStrength = 0.139f;    //0.139.0f
float mieStrength = 0.264f;         //0.264f
float rayleighCollectionPower = 0.81f; //0.81f
float mieCollectionPower = 0.39f;   //0.39f
float mieDistribution = 0.63f;      // 0.63f

const String tagLS = "LS";
bool isRenderPathForLSInjected = false;
Texture2D@  LSTextureSunHalo;
Node@ sunOriginNode;

void CheckOrCreateProSkyStuff(bool newOpenedScene) 
{
    if (editorScene is null || procSkyNode is null) return;
    
    if (procSkyCamera is null || newOpenedScene) 
    {
        procSkyCamera = procSkyNode.CreateComponent("Camera");
        procSkyCamera.fov = psFOV;
        procSkyCamera.farClip = 50.0f;
        procSkyCamera.nearClip = 0.5f;
        procSkyCamera.aspectRatio = 1.0;
    }
    
    if (procSkyLightNode is null || newOpenedScene) 
    {
        procSkyLightNode = procSkyNode.CreateChild("ProcSkyLightNode");
        procSkyLightNode.worldRotation = Quaternion(40, 180, 0);
        
        if (procSkyLight is null || newOpenedScene) 
        {   
            procSkyLight = procSkyLightNode.CreateComponent("Light");
            procSkyLight.lightType = LIGHT_DIRECTIONAL;
            procSkyLightColor.Clip(true);
            procSkyLightColor.FromHSV(57.0f, 9.9f, 73.0f , 1.0);
            procSkyLight.color = procSkyLightColor;
        }
        
        if (sunOriginNode is null || newOpenedScene) 
        {
            sunOriginNode = procSkyLightNode.CreateChild("sunOriginNode");
            sunOriginNode.position = Vector3(0, 0, -100000);
        }
    }
    
    if (procSkySkyBox is null || newOpenedScene) 
    {
        procSkySkyBox = procSkyNode.CreateComponent("Skybox");
        procSkySkyBox.model = cache.GetResource("Model","Models/Box.mdl");
        procSkySkyBoxMaterial = Material();
        procSkySkyBoxMaterial.SetTechnique(0, cache.GetResource("Technique","Techniques/DiffSkyboxLightScatteringMask.xml"));
        procSkySkyBoxMaterial.cullMode = CULL_NONE;
        procSkySkyBox.material = procSkySkyBoxMaterial;
        
    }
    
    if (procSkyRenderSizeLast != procSkyRenderSize || newOpenedScene)
        if (SetRenderSize(procSkyRenderSize))
            procSkyRenderSizeLast = procSkyRenderSize;
    
    if (procSkyFaceRotations.length < MAX_CUBEMAP_FACES || newOpenedScene)
    {     
        procSkyFaceRotations.Resize(MAX_CUBEMAP_FACES);
        procSkyFaceRotations[FACE_POSITIVE_X] = Quaternion(0, 90, 0);
        procSkyFaceRotations[FACE_NEGATIVE_X] = Quaternion(0, -90, 0);
        procSkyFaceRotations[FACE_POSITIVE_Y] = Quaternion(-90, 0, 0);
        procSkyFaceRotations[FACE_NEGATIVE_Y] = Quaternion(90, 0, 0);
        procSkyFaceRotations[FACE_POSITIVE_Z] = Quaternion(0, 0, 0);
        procSkyFaceRotations[FACE_NEGATIVE_Z] = Quaternion(0, 180, 0);
    }
    
    VariantMap atmoParams;
    atmoParams["Kr"] = Kr;
    atmoParams["RayleighBrightness"] = rayleighBrightness;
    atmoParams["MieBrightness"] = mieBrightness;
    atmoParams["SpotBrightness"] = spotBrightness;
    atmoParams["ScatterStrength"] = scatterStrength;
    atmoParams["RayleighStrength"] = rayleighStrength;
    atmoParams["MieStrength"] = mieStrength;
    atmoParams["RayleighCollectionPower"] = rayleighCollectionPower;
    atmoParams["MieCollectionPower"] = mieCollectionPower;
    atmoParams["MieDistribution"] = mieDistribution;
    atmoParams["LightDir"] = Vector3(0.0f,-1.0f, 0.0f);
    atmoParams["InvProj"] = procSkyCamera.projection.Inverse();
    
    cmd.Resize(MAX_CUBEMAP_FACES);
    for (int i = 0; i < MAX_CUBEMAP_FACES; ++i) 
    {
        cmd[i].enabled = true;
        cmd[i].blendMode = BLEND_REPLACE;
        cmd[i].tag = "ProcSky";
        cmd[i].type = CMD_QUAD;
        cmd[i].sortMode = SORT_BACKTOFRONT;
        cmd[i].pass = "postopaque";
        cmd[i].SetOutput(0, "DiffProcSky", CubeMapFace(i));
        cmd[i].vertexShaderName = "ProcSky";
        cmd[i].vertexShaderDefines = "";
        cmd[i].pixelShaderName = "ProcSky";
        cmd[i].pixelShaderDefines = "";     
        cmd[i].shaderParameters["Kr"] = atmoParams["Kr"];
        cmd[i].shaderParameters["RayleighBrightness"] = atmoParams["RayleighBrightness"];
        cmd[i].shaderParameters["MieBrightness"] = atmoParams["MieBrightness"];
        cmd[i].shaderParameters["SpotBrightness"] = atmoParams["SpotBrightness"];
        cmd[i].shaderParameters["ScatterStrength"] = atmoParams["ScatterStrength"];
        cmd[i].shaderParameters["RayleighStrength"] = atmoParams["RayleighStrength"];
        cmd[i].shaderParameters["MieStrength"] = atmoParams["MieStrength"];
        cmd[i].shaderParameters["RayleighCollectionPower"] = atmoParams["RayleighCollectionPower"];
        cmd[i].shaderParameters["MieDistribution"] = atmoParams["MieDistribution"];
        cmd[i].shaderParameters["LightDir"] = atmoParams["LightDir"];
        cmd[i].shaderParameters["InvProj"] = atmoParams["InvProj"]; 
        cmd[i].shaderParameters["InvViewRot"] = Variant(procSkyFaceRotations[i].rotationMatrix);
        
        procSkyRenderPath.AddCommand(cmd[i]);
    }
    
    //AddRenderPathCmdForLightScattering();
}

void AddRenderPathRT(String nameRT) 
{
    RenderTargetInfo rti;
    rti.enabled = true;
    rti.filtered = true;
    rti.size = Vector2(1,1);
    rti.sizeMode = SIZE_VIEWPORTDIVISOR;
    rti.format = GetRGBAFormat();
    rti.tag = tagLS;
    rti.name = nameRT;
    procSkyRenderPath.AddRenderTarget(rti);
}

bool isRTTAlreadyExist(String nameRT) 
{
    //Array<RenderTargetInfo> rttArray = renderPath.renderTargets;
    for (int i=0; i<procSkyRenderPath.numRenderTargets ; i++) 
    {
        //RenderTargetInfo rtt = ;
        if (procSkyRenderPath.renderTargets[i].name == nameRT)
            return true;
    }
   
    return false;
}
void AddRenderPathCopyQuadCmdCopyFramebuffer() 
{
    
}

void AddRenderPathCmdForLightScattering() 
{
    if (isRenderPathForLSInjected) return;
    
    //AddRenderPathRT("SunHalo");
    //AddMRT1InPostopaquePassOutput();
    String RTTlowresRT1Name = "RTT1";
    String RTTlowresRT2Name = "RTT2";
    //AddRenderPathRT(renderPath, RTTlowresRT1Name, tagLS, Vector2(4,4));
    //AddRenderPathRT(renderPath, RTTlowresRT2Name, tagLS, Vector2(4,4));
      
    //RenderPathCommand lsCmd;
    
    //procSkyRenderPath
    isRenderPathForLSInjected = true;
}

void AddMRT1InPostopaquePassOutput() 
{
    for (int i=0; i<procSkyRenderPath.numCommands-1; i++) 
    {
        RenderPathCommand cmd = procSkyRenderPath.commands[i];
        
        if (cmd.pass == "postopaque")
        {
            
            cmd.SetOutput(0, "viewport");
            cmd.SetOutput(1, "SunHalo");
            //renderPath.RemoveCommand(i);
            procSkyRenderPath.commands[i] = cmd;
            //renderPath.commands[i] = cmd;
            //renderPath.commands[i].SetOutput(1, "SunHalo");
            //renderPath.InsertCommand(i, cmd);
            return;
        }   
    }    
}

bool SetRenderSize(int size) 
{
    if (size >= 1) 
    {
        procSkyCubeTexture = TextureCube();
        procSkyCubeTexture.name = "DiffProcSky";
        procSkyCubeTexture.SetSize(size, GetRGBAFormat(), TEXTURE_RENDERTARGET);
        procSkyCubeTexture.filterMode = FILTER_TRILINEAR; //FILTER_BILINEAR;
        procSkyCubeTexture.addressMode[COORD_U] = ADDRESS_CLAMP;
        procSkyCubeTexture.addressMode[COORD_V] = ADDRESS_CLAMP;
        procSkyCubeTexture.addressMode[COORD_W] = ADDRESS_CLAMP;
        cache.AddManualResource(procSkyCubeTexture);
        if (procSkySkyBoxMaterial !is null ) 
            procSkySkyBoxMaterial.textures[TU_DIFFUSE] = procSkyCubeTexture;
        procSkyRenderSize = size;
        return true;
    }
    else 
    {
        MessageBox("ProcSky::SetSize (" + String(size) + ") ignored; requires size >= 1.");
    }
    return false;
}

void SetProcSkyAsEnvCubemapForZonesByTag(String tagZone, bool forAll = false) 
{
	
	Array<Node@> nodesWithZones; 
	if (!forAll)
        nodesWithZones = editorScene.GetChildrenWithTag(tagZone);
	else
        nodesWithZones = editorScene.GetChildrenWithComponent("Zone", true);
	
	if (nodesWithZones.length > 0) 
		for (int i=0 ; i<nodesWithZones.length; i++) 
		{
            Zone@ zone = nodesWithZones[i].GetComponent("Zone");
            if (zone !is null && procSkyCubeTexture !is null) 
            {
                zone.zoneTexture = procSkyCubeTexture;
            }
		}
}

void ProcSkyCheckKeys()
{
    if (input.keyDown[KEY_Y])
        SetProcSkyAsEnvCubemapForZonesByTag("tagProcSky", true);
}

void UpdateViewProcSky() 
{
    // Early out
    if (editorScene is null || !procSkyShow) return;
    
    // Check for container and other procSky stuff (1sec delay)
    if (timeToNextProcSkyUpdate < time.systemTime) 
    {
    
		procSkyNode = editorScene.GetChild("ProcSkyContainer", true);

		if (procSkyNode is null) 
		{
			procSkyNode = editorScene.CreateChild("ProcSkyContainer", LOCAL);
			//procSkyNode.temporary = true;
			procSkyRenderPath = activeViewport.viewport.renderPath;
			CheckOrCreateProSkyStuff(true);
			
		}
        
        timeToNextProcSkyUpdate = time.systemTime + PROCSKY_STEP_UPDATE;
    }
    
    // 32 ms
    if ((timeToNextProcSkyRender < time.systemTime) || (procSkyLightNodeLastRotation != procSkyLightNode.worldRotation)) 
    {
		ProcSkyCheckKeys();
		
		if (procSkyRenderPath !is null) 
		{
			
			SetRenderQueued(true);
        }
        
        procSkyLightNodeLastRotation = procSkyLightNode.worldRotation;
        timeToNextProcSkyRender = time.systemTime + PROCSKY_STEP_RENDER;
    } 
    else 
    {
		if (procSkyRenderQueued)
			SetRenderQueued(false);
    }
    
    
    // Each frame update stuff
    UpdateShaderParameters(procSkyRenderPath);
    
} 

void UpdateShaderParameters(RenderPath@ renderPath) 
{
	if (renderPath is null) return;
	
    Vector3 lightDir;
    if (procSkyLightNode !is null) 
    {
        lightDir = -procSkyLightNode.worldDirection;
        renderPath.shaderParameters["LightDir"] = Variant(lightDir);
    }
        
    renderPath.shaderParameters["Kr"] = Variant(Kr);
    renderPath.shaderParameters["RayleighBrightness"] = Variant(rayleighBrightness);
    renderPath.shaderParameters["MieBrightness"] = Variant(mieBrightness);
    renderPath.shaderParameters["SpotBrightness"] = Variant(spotBrightness);
    renderPath.shaderParameters["ScatterStrength"] = Variant(scatterStrength);
    renderPath.shaderParameters["RayleighStrength"] = Variant(rayleighStrength);
    renderPath.shaderParameters["MieStrength"] = Variant(mieStrength);
    renderPath.shaderParameters["RayleighCollectionPower"] = Variant(rayleighCollectionPower);
    renderPath.shaderParameters["MieDistribution"] = Variant(mieDistribution);
    renderPath.shaderParameters["InvProj"] = Variant(procSkyCamera.projection.Inverse());
    
    Vector2 SunScreen = activeViewport.viewport.camera.WorldToScreenPoint(sunOriginNode.worldPosition);
    SunScreen.y = 1.0 - SunScreen.y;
    //Matrix4 translate;
    //translate.SetTranslation(Vector3(0,0,-1000000));
    //Vector4 t = translate * Vector4(0,0,0,1);
    //Vector3 SunOrigin;
    //SunOrigin = procSkyLightNode.worldRotation.rotationMatrix.Inverse()  * Vector3(t.x, t.y, t.z);
    //Vector2 SunScreen = activeViewport.viewport.camera.WorldToScreenPoint(SunOrigin);
    
    renderPath.shaderParameters["LightPositionOnScreen"] = Variant(SunScreen);
}

void SetRenderQueued(bool isQueued = true) 
{
    if (procSkyRenderQueued == isQueued || procSkyRenderPath is null) return;
    procSkyRenderPath.SetEnabled("ProcSky", isQueued);
    procSkyRenderQueued = isQueued;
    
}

void ProcSkyPostRenderUpdate()
{
    if (!procSkyRenderQueued) return;
    
    //
}