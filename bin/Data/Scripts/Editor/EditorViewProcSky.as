
const int PROCSKY_STEP_UPDATE = 16;
float timeToNextProcSkyUpdate = 0;


int psParam = 256;
float psFOV = 89.5f;

bool procSkyShow = true;
Node@ procSkyNode;
Node@ procSkyLightNode;
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
float rayleighBrightness = 3.3f;
float mieBrightness = 0.1f;
float spotBrightness = 50.0f;
float scatterStrength = 0.028f;
float rayleighStrength = 0.139f;
float mieStrength = 0.264f;
float rayleighCollectionPower = 0.81f;
float mieCollectionPower = 0.39f;
float mieDistribution = 0.63f;

void CheckOrCreateProSkyStuff() 
{
    if (editorScene is null || procSkyNode is null) return;
    
    if (procSkyCamera is null) 
    {
        procSkyCamera = procSkyNode.CreateComponent("Camera");
        procSkyCamera.fov = psFOV;
        procSkyCamera.farClip = 50.0f;
        procSkyCamera.nearClip = 0.5f;
        procSkyCamera.aspectRatio = 1.0;
    }
    
    if (procSkyLightNode is null) 
    {
        procSkyLightNode = procSkyNode.CreateChild("ProcSkyLightNode");
        if (procSkyLight is null) 
        {
            procSkyLight = procSkyLightNode.CreateComponent("Light");
            procSkyLight.lightType = LIGHT_DIRECTIONAL;
            procSkyLightColor.FromHSV(57.0f, 9.9f, 73.0f , 1.0);
            procSkyLight.color = procSkyLightColor; 
        }   
    }
    
    if (procSkySkyBox is null) 
    {
        procSkySkyBox = procSkyNode.CreateComponent("Skybox");
        procSkySkyBox.model = cache.GetResource("Model","Models/Box.mdl");
        procSkySkyBoxMaterial = Material();
        procSkySkyBoxMaterial.SetTechnique(0, cache.GetResource("Technique","Techniques/DiffSkybox.xml"));
        procSkySkyBoxMaterial.cullMode = CULL_NONE;
        procSkySkyBox.material = procSkySkyBoxMaterial;
        
    }
    
    if (procSkyRenderSizeLast != procSkyRenderSize)
        if (SetRenderSize(procSkyRenderSize))
            procSkyRenderSizeLast = procSkyRenderSize;
    
    if (procSkyFaceRotations.length < MAX_CUBEMAP_FACES)
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
        {
            
            procSkySkyBoxMaterial.textures[TU_DIFFUSE] = procSkyCubeTexture;
        }
        procSkyRenderSize = size;
        return true;
    }
    else 
    {
        MessageBox("1");
            // LOGWARNING("ProcSky::SetSize (" + String(size) + ") ignored; requires size >= 1.");
    }
    return false;
}

void UpdateViewProcSky() 
{
    // Early out
    if (!procSkyShow) return;
    
    if (editorScene is null || timeToNextProcSkyUpdate > time.systemTime) return;
    
    procSkyNode = editorScene.GetChild("ProcSkyContainer", true);

    // Get RenderPath
    if (procSkyRenderPath !is activeViewport.viewport.renderPath) 
    {
        procSkyRenderPath = activeViewport.viewport.renderPath;
        //MessageBox("Get RenderPath");
    }
    
    if (procSkyNode is null) 
    {
        procSkyNode = editorScene.CreateChild("ProcSkyContainer", LOCAL);
        //procSkyNode.temporary = true;
        CheckOrCreateProSkyStuff();
        SetRenderQueued(true);
    }
    
    // update
    UpdateShaderParameters();
    
    
    timeToNextProcSkyUpdate = time.systemTime + PROCSKY_STEP_UPDATE;
} 

void UpdateShaderParameters() 
{
    Vector3 lightDir;
    if (procSkyLightNode !is null) 
    {
        lightDir = -procSkyLightNode.worldDirection;
        procSkyRenderPath.shaderParameters["LightDir"] = Variant(lightDir);
    }
    
    
    //for (int i = 0; i < MAX_CUBEMAP_FACES; ++i) 
    //{
    //    cmd[i].shaderParameters["LightDir"] = Variant(lightDir);
    //    cmd[i].shaderParameters["Kr"] = Variant(Kr);
    //    cmd[i].shaderParameters["RayleighBrightness"] = Variant(rayleighBrightness);
    //    cmd[i].shaderParameters["MieBrightness"] = Variant(mieBrightness);
    //    cmd[i].shaderParameters["SpotBrightness"] = Variant(spotBrightness);
    //    cmd[i].shaderParameters["ScatterStrength"] = Variant(scatterStrength);
    //    cmd[i].shaderParameters["RayleighStrength"] = Variant(rayleighStrength);
    //    cmd[i].shaderParameters["MieStrength"] = Variant(mieStrength);
    //    cmd[i].shaderParameters["RayleighCollectionPower"] = Variant(rayleighCollectionPower);
    //    cmd[i].shaderParameters["MieDistribution"] = Variant(mieDistribution);
    //    cmd[i].shaderParameters["InvProj"] = Variant(procSkyCamera.projection.Inverse());
    //}
    
    procSkyRenderPath.shaderParameters["Kr"] = Variant(Kr);
    procSkyRenderPath.shaderParameters["RayleighBrightness"] = Variant(rayleighBrightness);
    procSkyRenderPath.shaderParameters["MieBrightness"] = Variant(mieBrightness);
    procSkyRenderPath.shaderParameters["SpotBrightness"] = Variant(spotBrightness);
    procSkyRenderPath.shaderParameters["ScatterStrength"] = Variant(scatterStrength);
    procSkyRenderPath.shaderParameters["RayleighStrength"] = Variant(rayleighStrength);
    procSkyRenderPath.shaderParameters["MieStrength"] = Variant(mieStrength);
    procSkyRenderPath.shaderParameters["RayleighCollectionPower"] = Variant(rayleighCollectionPower);
    procSkyRenderPath.shaderParameters["MieDistribution"] = Variant(mieDistribution);
    procSkyRenderPath.shaderParameters["InvProj"] = Variant(procSkyCamera.projection.Inverse());

    
    
}

void SetRenderQueued(bool isQueued = true) 
{
    if (procSkyRenderQueued == isQueued || procSkyRenderPath is null) return;
    procSkyRenderPath.SetEnabled("ProcSky", isQueued);
    procSkyRenderQueued = isQueued;
    
}

void ProcSkyPostRenderUpdate()
{
    //SetRenderQueued(false);
}