
// Implementation for the Urho3D engine of:
// Alchemy screen-space ambient obscurance algorithm example
// http://graphics.cs.williams.edu/papers/AlchemyHPG11/
// Scalable Ambient Obscurance
// http://graphics.cs.williams.edu/papers/SAOHPG12/

#include "Scripts/Utilities/Sample.as"

// TODO: find the OpenGL flag
const bool OpenGL = true;
// Number of steps in each controller (high = precise+slow)
const float var_steps = 200.0f;

// Enable ambient occlusion effect
bool ao_enable = true;
// Enable a simple Gaussian blur
bool ao_blur = false;
// Enable a depth aware Gaussian blur
bool ao_blurdepth = true;
// Show ambient occlusion raw values
bool ao_only = false;

// uniforms format: Vector3[val, min, max]

// AO radius in scene units
Vector3 ao_radius = Vector3(1.0f, 0.0f, 4.0f);
// AO intensity (in the paper/demo is divided by radius^6, this is commented out)
Vector3 ao_intensity = Vector3(0.15f, 0.0f, 2.0f);
// Radius scale adjust (not present in the paper/demo, TODO to be fixed)
Vector3 ao_projscale = Vector3(0.3f, 0.0f, 1.0f);
// Self occlusion margin
Vector3 ao_bias = Vector3(0.01f, 0.0f, 0.1f);
// Aux vars
Vector3 ao_var1 = Vector3(0.0f, -1.0f, 1.0f);
Vector3 ao_var2 = Vector3(1.0f, 0.0f, 1.0f);

Text@ infoText;
bool var_changed = true;
bool define_changed = true;
uint commandIndexSaoMain = -1;
uint commandIndexSaoCopy = -1;

void Start()
{
    // Execute the common startup for samples
    SampleStart();
    // Hide logo to show bad occlusion at borders 
    SetLogoVisible(false);

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateInstructions();

    // Setup the viewport for displaying the scene
    SetupViewport();

    // Hook up to the frame update events
    SubscribeToEvents();
}

void CreateScene()
{
    scene_ = Scene();

    // Create the Octree component to the scene. This is required before adding any drawable components, or else nothing will
    // show up. The default octree volume will be from (-1000, -1000, -1000) to (1000, 1000, 1000) in world coordinates; it
    // is also legal to place objects outside the volume but their visibility can then not be checked in a hierarchically
    // optimizing manner
    scene_.CreateComponent("Octree");

    // Create a directional light to the world so that we can see something. The light scene node's orientation controls the
    // light direction; we will use the SetDirection() function which calculates the orientation from a forward direction vector.
    // The light will use default settings (white light, no shadows)
    Node@ lightNode = scene_.CreateChild("DirectionalLight");
    lightNode.direction = Vector3(0.6f, -1.0f, 0.8f); // The direction vector does not need to be normalized
    Light@ light = lightNode.CreateComponent("Light");
    light.lightType = LIGHT_DIRECTIONAL;

    // Create more StaticModel objects to the scene, randomly positioned, rotated and scaled. 
    {
        Vector3 center = Vector3(0.0f, -2.0f, 10.0f);
        Vector3 scale = Vector3(30.0f, 2.0f, 10.0f);
        float scatter = 60.0f;
        for (uint i = 0; i < 50; ++i)
        {
            Node@ node = scene_.CreateChild("Box");
            node.position = center + Vector3(Random(scatter) - scatter*0.5f, Random(scatter)/30.0, Random(scatter) - scatter*0.5f);
            node.rotation = Quaternion(0.0f, Random(360.0f), 0.0f);
            node.Scale( Vector3(0.5f + Random(scale.x), 0.5f + Random(scale.y), 0.5f + Random(scale.z)) );
            StaticModel@ nodeObject = node.CreateComponent("StaticModel");
            nodeObject.model = cache.GetResource("Model", "Models/Box.mdl");
            //nodeObject.material = cache.GetResource("Material", "Materials/StoneTiled.xml");
        }
    }
    
    {
        Vector3 center = Vector3(5.0f, 5.0f, 10.0f);
        float scale = 3.0f;
        float scatter = 8.0f;
        for (uint i = 0; i < 50; ++i)
        {
            Node@ node = scene_.CreateChild("Box");
            node.position = center + Vector3(Random(scatter) - scatter*0.5f, Random(scatter) - scatter*0.5f, Random(scatter) - scatter*0.5f);
            node.rotation = Quaternion(0.0f, Random(360.0f), 0.0f);
            node.SetScale(0.5f + Random(scale));
            StaticModel@ nodeObject = node.CreateComponent("StaticModel");
            nodeObject.model = cache.GetResource("Model", "Models/Box.mdl");
            //nodeObject.material = cache.GetResource("Material", "Materials/StoneTiled.xml");
        }
    }
    
    {
        Vector3 center = Vector3(-5.0f, 5.0f, 10.0f);
        float scatter = 10.0f;
        for (uint i = 0; i < 50; ++i)
        {
            Node@ node = scene_.CreateChild("Sphere");
            node.position = center + Vector3(Random(scatter) - scatter*0.5f, Random(scatter) - scatter*0.5f, Random(scatter) - scatter*0.5f);
            node.rotation = Quaternion(0.0f, Random(360.0f), 0.0f);
            node.SetScale(0.5f + Random(scatter / 3.0f));
            StaticModel@ nodeObject = node.CreateComponent("StaticModel");
            nodeObject.model = cache.GetResource("Model", "Models/Sphere.mdl");
            //nodeObject.material = cache.GetResource("Material", "Materials/StoneTiled.xml");
        }
    }

    // Create a scene node for the camera, which we will move around
    // The camera will use default settings (1000 far clip distance, 45 degrees FOV, set aspect ratio automatically)
    cameraNode = scene_.CreateChild("Camera");
    cameraNode.CreateComponent("Camera");

    // Set an initial position for the camera scene node above the plane
    cameraNode.position = Vector3(0.0f, 5.0f, 0.0f);
}

void CreateInstructions()
{
    Font@ font = cache.GetResource("Font", "Fonts/Anonymous Pro.ttf");
    float space = ui.root.height / 12.0;
    
    // Construct new Text object, set string to display and font to use
    Text@ titleText = ui.root.CreateChild("Text");
    titleText.text = "Urho3D - Realtime ambient occlusion " + graphics.width + "x" + graphics.height;
    titleText.SetFont(font, 15);
    titleText.color = Color(0.0f, 0.0f, 1.0f);

    // Position the text relative to the top-left corner
    titleText.horizontalAlignment = HA_LEFT;
    titleText.verticalAlignment = VA_TOP;
    titleText.SetPosition(space, space);

    infoText = ui.root.CreateChild("Text");
    infoText.text = "info";
    infoText.SetFont(font, 15);
    infoText.color = Color(0.0f, 0.0f, 1.0f);

    // Position the text relative to the top-left corner
    infoText.horizontalAlignment = HA_LEFT;
    infoText.verticalAlignment = VA_TOP;
    infoText.SetPosition(space, space * 2.0f);
}

void SetupViewport()
{
    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen. We need to define the scene and the camera
    // at minimum. Additionally we could configure the viewport screen size and the rendering path (eg. forward / deferred) to
    // use, but now we just use full screen and default render path configured in the engine command line options
    Viewport@ viewport = Viewport(scene_, cameraNode.GetComponent("Camera"));
    renderer.viewports[0] = viewport;

    // Load deferred SAO render path
    RenderPath@ renderPath = RenderPath();
    renderPath.Load( cache.GetResource("XMLFile", "RenderPaths/DeferredSAO.xml") );
    viewport.set_renderPath(renderPath);

    // Search and save AO commands indices
    for (uint i = 0; i < renderPath.numCommands; ++i)
    {
        const RenderPathCommand command = renderPath.commands[i];
        if (command.tag == "SAO_main")
            commandIndexSaoMain = i;
        if (command.tag == "SAO_copy")
            commandIndexSaoCopy = i;
    }
}

bool VarChange(int keyDown, int keyUp, Vector3& var)
{
    // var = [value, min, max]
    
    float step = (var.z - var.y) / var_steps;
    if (input.keyDown[keyDown])
    {
        var.x -= step;
        if (var.x < var.y)
            var.x = var.y;
        return true;
    }
    if (input.keyDown[keyUp])
    {
        var.x += step;
        if (var.x > var.z)
            var.x = var.z;
        return true;
    }
    return false;
}

void MoveCamera(float timeStep)
{
    // Do not move if the UI has a focused element (the console)
    if (ui.focusElement !is null)
        return;

    // Movement speed as world units per second
    float MOVE_SPEED = 20.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;

    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
    IntVector2 mouseMove = input.mouseMove;
    yaw += MOUSE_SENSITIVITY * mouseMove.x;
    pitch += MOUSE_SENSITIVITY * mouseMove.y;
    pitch = Clamp(pitch, -90.0f, 90.0f);

    // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
    cameraNode.rotation = Quaternion(pitch, yaw, 0.0f);

    // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
    // Use the Translate() function (default local space) to move relative to the node's orientation.
    if (input.keyDown[KEY_SHIFT])
        MOVE_SPEED *= 0.05f;
    if (input.keyDown['W'])
        cameraNode.Translate(Vector3(0.0f, 0.0f, 1.0f) * MOVE_SPEED * timeStep);
    if (input.keyDown['S'])
        cameraNode.Translate(Vector3(0.0f, 0.0f, -1.0f) * MOVE_SPEED * timeStep);
    if (input.keyDown['A'])
        cameraNode.Translate(Vector3(-1.0f, 0.0f, 0.0f) * MOVE_SPEED * timeStep);
    if (input.keyDown['D'])
        cameraNode.Translate(Vector3(1.0f, 0.0f, 0.0f) * MOVE_SPEED * timeStep);

    // Ambient occlusion controllers
    if (input.keyPress['V'])
    {
        ao_enable = !ao_enable;
        var_changed = true;
    }

    if (input.keyPress['B'])
    {
        ao_blur = !ao_blur;
        ao_blurdepth = false;
        var_changed = true;
    }

    if (input.keyPress['N'])
    {
        ao_blurdepth = !ao_blurdepth;
        ao_blur = false;
        var_changed = true;
    }

    if (input.keyPress['M'])
    {
        ao_only = !ao_only;
        var_changed = true;
        define_changed = true;
    }   

    var_changed = var_changed || VarChange('F', 'R', ao_radius);
    var_changed = var_changed || VarChange('G', 'T', ao_projscale);
    var_changed = var_changed || VarChange('H', 'Y', ao_intensity);
    var_changed = var_changed || VarChange('J', 'U', ao_bias);

    var_changed = var_changed || VarChange('K', 'I', ao_var1);
    var_changed = var_changed || VarChange('L', 'O', ao_var2);

    // Apply controllers if they changed
    if (var_changed)
    {
        var_changed = false;
        Camera@ camera = cameraNode.GetComponent("Camera");
        
        infoText.text = 
            "ao enable(V) = " + ao_enable +
            "\nao blur(B) = " + ao_blur +
            "\nao blurdepth(N) = " + ao_blurdepth +
            "\nao only(M) = " + ao_only +
            "\nradius(R/F) = " + ao_radius.x +
            "\nproj scale(T/G) = " + ao_projscale.x +
            "\nintensity(Y/H) = " + ao_intensity.x +
            "\nbias(U/J) = " + ao_bias.x +
            "\naux " + ao_var1.x + " " + ao_var2.x;

        RenderPath@ renderPath = renderer.viewports[0].renderPath;
        
        renderPath.SetEnabled("SAO_copy", ao_enable);
        renderPath.SetEnabled("BlurGaussian", ao_blur);
        renderPath.SetEnabled("BlurGaussianDepth", ao_blurdepth);
        
        if (define_changed && commandIndexSaoCopy != -1)
        {
            define_changed = false;
            // TODO: avoid replace?
            RenderPathCommand command = renderPath.commands[commandIndexSaoCopy];
            if (ao_only)
                command.pixelShaderDefines = "AO_ONLY";
            else
                command.pixelShaderDefines = "";
            renderPath.RemoveCommand(commandIndexSaoCopy);
            renderPath.InsertCommand(commandIndexSaoCopy, command);
        }   

        renderPath.shaderParameters["Radius"] = Variant(ao_radius.x);
        renderPath.shaderParameters["ProjScale2"] = Variant(ao_projscale.x);
        //renderPath.shaderParameters["IntensityDivR6"] = Variant( ao_intensity.x / Pow(ao_radius.x, 6.0f) );
        renderPath.shaderParameters["IntensityDivR6"] = Variant( ao_intensity.x );
        renderPath.shaderParameters["Bias"] = Variant(ao_bias.x);
        
        renderPath.shaderParameters["Var1"] = Variant(ao_var1.x);
        renderPath.shaderParameters["Var2"] = Variant(ao_var2.x);
    }
}

void SubscribeToEvents()
{
    // Subscribe HandleUpdate() function for processing update events
    SubscribeToEvent("Update", "HandleUpdate");
    
    SubscribeToEvent("RenderUpdate", "HandleRenderUpdate");
}

void HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    // Take the frame time step, which is stored as a float
    float timeStep = eventData["TimeStep"].GetFloat();

    // Move the camera, scale movement with time step
    MoveCamera(timeStep);
}

void HandleRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    Camera@ camera = cameraNode.GetComponent("Camera");
    Viewport@ viewport = renderer.viewports[0];
    RenderPath@ renderPath = viewport.renderPath;
    
    // Projection vector: used to reconstruct pixels positions
    Matrix4 p = camera.get_projection();
    Vector4 projInfo;
    if (OpenGL)
    {
        // OpenGL
        projInfo = Vector4( 2.0f / p.m00,
                            2.0f / p.m11,
                            -(1.0f + p.m02) / p.m00,
                            -(1.0f + p.m12) / p.m11 );
    }
    else
    {
        // DirectX
        projInfo = Vector4( 2.0f / p.m00,
                           -2.0f / p.m11,
                           -(1.0f + p.m02 + 1.0f / graphics.width) / p.m00,
                            (1.0f - p.m12 + 1.0f / graphics.height) / p.m11 );
    }
    renderPath.set_shaderParameters("ProjInfo", Variant(projInfo));

    // Projection scale: used to scale raidius (TODO: not correct and needs ProjScale2 to fix it)
    float viewSize = 2.0f * camera.get_halfViewSize();
    renderPath.set_shaderParameters("ProjScale", Variant(graphics.height / viewSize));
    // TODO: viewport.get_rect is zero?
    //IntRect viewRect = viewport.get_rect();
    //renderPath.set_shaderParameters("ProjScale", Variant(viewRect.height / viewSize));
    
    // View matrix: used to rotate normals (we can also reconstruct normals in the shader)
    Matrix3 v = camera.get_view().ToMatrix3();
    renderPath.set_shaderParameters("View", Variant(v));
}

// Create XML patch instructions for screen joystick layout specific to this sample app
String patchInstructions = "";
