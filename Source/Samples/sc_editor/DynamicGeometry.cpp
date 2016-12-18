////
//// Copyright (c) 2008-2016 the Urho3D project.
////
//// Permission is hereby granted, free of charge, to any person obtaining a copy
//// of this software and associated documentation files (the "Software"), to deal
//// in the Software without restriction, including without limitation the rights
//// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//// copies of the Software, and to permit persons to whom the Software is
//// furnished to do so, subject to the following conditions:
////
//// The above copyright notice and this permission notice shall be included in
//// all copies or substantial portions of the Software.
////
//// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//// THE SOFTWARE.
////
//
//#include <Urho3D/Core/CoreEvents.h>
//#include <Urho3D/Core/Profiler.h>
//#include <Urho3D/Engine/Engine.h>
//#include <Urho3D/Graphics/Camera.h>
//#include <Urho3D/Graphics/Geometry.h>
//#include <Urho3D/Graphics/Graphics.h>
//#include <Urho3D/Graphics/IndexBuffer.h>
//#include <Urho3D/Graphics/Light.h>
//#include <Urho3D/Graphics/Model.h>
//#include <Urho3D/Graphics/Octree.h>
//#include <Urho3D/Graphics/Renderer.h>
//#include <Urho3D/Graphics/RenderPath.h>
//#include <Urho3D/Graphics/StaticModel.h>
//#include <Urho3D/Graphics/Skybox.h>
//#include <Urho3D/Graphics/TextureCube.h>
//#include <Urho3D/Graphics/VertexBuffer.h>
//#include <Urho3D/Graphics/Zone.h>
//#include <Urho3D/Input/Input.h>
//#include <Urho3D/IO/Log.h>
//#include <Urho3D/Resource/ResourceCache.h>
//#include <Urho3D/Scene/Scene.h>
//#include <Urho3D/UI/Font.h>
//#include <Urho3D/UI/Text.h>
//#include <Urho3D/UI/UI.h>
//
//#include "DynamicGeometry.h"
//
//#include <Urho3D/DebugNew.h>
//
////URHO3D_DEFINE_APPLICATION_MAIN(DynamicGeometry)
//
//DynamicGeometry::DynamicGeometry(Context* context) :
//    Sample(context),
//    animate_(true),
//    time_(0.0f)
//{
//}
//
//void DynamicGeometry::Setup()
//{
//  Sample::Setup();
//  engineParameters_["WindowResizable"] = true;
//  engineParameters_["RenderPath"] = "RenderPaths/PBRDeferredHWDepthSHW.xml";
//}
//
//void DynamicGeometry::Start()
//{
//    // Execute base class startup
//    Sample::Start();
//
//    // Create the scene content
//    CreateScene();
//
//    // Create the UI content
//    CreateInstructions();
//
//    // Setup the viewport for displaying the scene
//    SetupViewport();
//
//    // Hook up to the frame update events
//    SubscribeToEvents();
//
//    // Set the mouse mode to use in the sample
//    //Sample::InitMouseMode(MM_RELATIVE);
//    Sample::InitMouseMode(MM_FREE);
//}
//
//void DynamicGeometry::CreateScene()
//{
//    ResourceCache* cache = GetSubsystem<ResourceCache>();
//
//    scene_ = new Scene(context_);
//
//    // Create the Octree component to the scene so that drawable objects can be rendered. Use default volume
//    // (-1000, -1000, -1000) to (1000, 1000, 1000)
//    scene_->CreateComponent<Octree>();
//
//    // Create a Zone for ambient light & fog control
//    Node* zoneNode = scene_->CreateChild("Zone");
//    Zone* zone = zoneNode->CreateComponent<Zone>();
//    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
//    zone->SetFogColor(Color(0.2f, 0.2f, 0.2f));
//    zone->SetFogStart(200.0f);
//    zone->SetFogEnd(300.0f);
//    zone->SetZoneTexture(cache->GetResource<TextureCube>("Textures/Spacebox.xml"));
//
//    // Create a directional light
//    Node* lightNode = scene_->CreateChild("DirectionalLight");
//    lightNode->SetDirection(Vector3(-0.2f, -0.6f, -0.8f)); // The direction vector does not need to be normalized
//    Light* light = lightNode->CreateComponent<Light>();
//    light->SetLightType(LIGHT_DIRECTIONAL);
//    light->SetColor(Color(1.0f, 0.95f, 0.8f));
//    light->SetBrightness(7.0f);
//    light->SetShadowBias(BiasParameters(0.0001f, 0.5f));
//    light->SetShadowCascade(CascadeParameters(2.0f, 10.0f, 50.0f, 250.0f, 0.9f));
//    //light->SetSpecularIntensity(1.5f);
//    light->SetCastShadows(true);
//
//    // Create skybox. The Skybox component is used like StaticModel, but it will be always located at the camera, giving the
//    // illusion of the box planes being far away. Use just the ordinary Box model and a suitable material, whose shader will
//    // generate the necessary 3D texture coordinates for cube mapping
//    Node* skyNode = scene_->CreateChild("Sky");
//    skyNode->SetScale(500.0f); // The scale actually does not matter
//    Skybox* skybox = skyNode->CreateComponent<Skybox>();
//    skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
//    skybox->SetMaterial(cache->GetResource<Material>("Materials/Spacebox.xml"));
//
//    {
//      // Create a floor object, 1000 x 1000 world units. Adjust position so that the ground is at zero Y
//      Node* floorNode = scene_->CreateChild("Floor");
//      floorNode->SetPosition(Vector3(0.0f, 0.0f, -6.0f));
//      floorNode->SetScale(Vector3(22.0f, 22.0f, 1.0f));
//      StaticModel* floorObject = floorNode->CreateComponent<StaticModel>();
//      //floorObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
//      floorObject->SetMaterial(cache->GetResource<Material>("Materials/PBR/Concrete.xml"));
//    }
//
//    // Get the original model and its unmodified vertices, which are used as source data for the animation
//    Model* originalModel = cache->GetResource<Model>("Models/Sphere.mdl");
//    if (!originalModel)
//    {
//        URHO3D_LOGERROR("Model not found, cannot initialize example scene");
//        return;
//    }
//    // Get the vertex buffer from the first geometry's first LOD level
//    VertexBuffer* buffer = originalModel->GetGeometry(0, 0)->GetVertexBuffer(0);
//    const unsigned char* vertexData = (const unsigned char*)buffer->Lock(0, buffer->GetVertexCount());
//    if (vertexData)
//    {
//        unsigned numVertices = buffer->GetVertexCount();
//        unsigned vertexSize = buffer->GetVertexSize();
//        // Copy the original vertex positions
//        for (unsigned i = 0; i < numVertices; ++i)
//        {
//            const Vector3& src = *reinterpret_cast<const Vector3*>(vertexData + i * vertexSize);
//            originalVertices_.Push(src);
//        }
//        buffer->Unlock();
//
//        // Detect duplicate vertices to allow seamless animation
//        vertexDuplicates_.Resize(originalVertices_.Size());
//        for (unsigned i = 0; i < originalVertices_.Size(); ++i)
//        {
//            vertexDuplicates_[i] = i; // Assume not a duplicate
//            for (unsigned j = 0; j < i; ++j)
//            {
//                if (originalVertices_[i].Equals(originalVertices_[j]))
//                {
//                    vertexDuplicates_[i] = j;
//                    break;
//                }
//            }
//        }
//    }
//    else
//    {
//        URHO3D_LOGERROR("Failed to lock the model vertex buffer to get original vertices");
//        return;
//    }
//
//    // Create StaticModels in the scene. Clone the model for each so that we can modify the vertex data individually
//    for (int y = -10; y <= 10; ++y)
//    {
//        for (int x = -10; x <= 10; ++x)
//        {
//          for (int z = -0; z <= 0; ++z) {
//            Node* node = scene_->CreateChild("Object");
//            node->SetPosition(Vector3(x * 2.0f, z * 2.0f, y * 2.0f));
//            //node->SetScale(18);
//            node->SetRotation(Quaternion(0, 90, 90));
//            StaticModel* object = node->CreateComponent<StaticModel>();
//            //SharedPtr<Model> cloneModel = originalModel->Clone();
//            object->SetModel(originalModel);
//            object->SetCastShadows(true);
//            object->SetMaterial(cache->GetResource<Material>("Materials/PBR/MetallicRough5SHW.xml"));
//            //object->SetMaterial(cache->GetResource<Material>("Materials/GS/GSPointsToSpheresMat.xml"));
//            // Store the cloned vertex buffer that we will modify when animating
//            //animatingBuffers_.Push(SharedPtr<VertexBuffer>(cloneModel->GetGeometry(0, 0)->GetVertexBuffer(0)));
//          }
//        }
//    }
//
//    // Finally create one model (pyramid shape) and a StaticModel to display it from scratch
//    // Note: there are duplicated vertices to enable face normals. We will calculate normals programmatically
//    {
//        /*const unsigned numVertices = 18;
//
//        float vertexData[] = {
//            // Position             Normal
//            0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 0.0f,
//            0.5f, -0.5f, 0.5f,      0.0f, 0.0f, 0.0f,
//            0.5f, -0.5f, -0.5f,     0.0f, 0.0f, 0.0f,
//
//            0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 0.0f,
//            -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 0.0f,
//            0.5f, -0.5f, 0.5f,      0.0f, 0.0f, 0.0f,
//
//            0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 0.0f,
//            -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 0.0f,
//            -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 0.0f,
//
//            0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 0.0f,
//            0.5f, -0.5f, -0.5f,     0.0f, 0.0f, 0.0f,
//            -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 0.0f,
//
//            0.5f, -0.5f, -0.5f,     0.0f, 0.0f, 0.0f,
//            0.5f, -0.5f, 0.5f,      0.0f, 0.0f, 0.0f,
//            -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 0.0f,
//
//            0.5f, -0.5f, -0.5f,     0.0f, 0.0f, 0.0f,
//            -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 0.0f,
//            -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 0.0f
//        };
//
//        const unsigned short indexData[] = {
//            0, 1, 2,
//            3, 4, 5,
//            6, 7, 8,
//            9, 10, 11,
//            12, 13, 14,
//            15, 16, 17
//        };
//
//        // Calculate face normals now
//        for (unsigned i = 0; i < numVertices; i += 3)
//        {
//            Vector3& v1 = *(reinterpret_cast<Vector3*>(&vertexData[6 * i]));
//            Vector3& v2 = *(reinterpret_cast<Vector3*>(&vertexData[6 * (i + 1)]));
//            Vector3& v3 = *(reinterpret_cast<Vector3*>(&vertexData[6 * (i + 2)]));
//            Vector3& n1 = *(reinterpret_cast<Vector3*>(&vertexData[6 * i + 3]));
//            Vector3& n2 = *(reinterpret_cast<Vector3*>(&vertexData[6 * (i + 1) + 3]));
//            Vector3& n3 = *(reinterpret_cast<Vector3*>(&vertexData[6 * (i + 2) + 3]));
//
//            Vector3 edge1 = v1 - v2;
//            Vector3 edge2 = v1 - v3;
//            n1 = n2 = n3 = edge1.CrossProduct(edge2).Normalized();
//        }*/
//
//        // Common buffers
//        /*const unsigned numVertices = 4;
//        const unsigned num_line_indexes = 12;
//        
//        float vertexData[] = {
//          0.5f, 0.0f, 0.0f,
//          -0.2f, 0.0f, 0.4f,
//          -0.2f, 0.0f, -0.4f,
//          0.0f, 0.6f, 0.0f
//        };
//
//        const unsigned short indexData[] = {
//          0, 1,
//          0, 2,
//          0, 3,
//          1, 2,
//          1, 3,
//          2, 3
//        };*/
//        const unsigned num_segments = 64;
//        const unsigned numVertices = num_segments * 3;
//        const unsigned num_line_indexes = num_segments * 3 * 3 * 2;
//        
//        float vertexData[numVertices * 3];
//        unsigned short indexData[num_line_indexes];
//
//        float R = 10;
//        float l = 0.8 / 2;
//        float R2 = R - l * sqrt(2);
//
//        for (int i = 0; i < num_segments; ++i) {
//          float angle = 2 * M_PI * i / num_segments;
//          float ca = cos(angle);
//          float sa = sin(angle);
//          int o = i * 3 * 3;
//          vertexData[o + 0] = ca * R;
//          vertexData[o + 1] = sa * R;
//          vertexData[o + 2] = -l;
//
//          vertexData[o + 3] = ca * R;
//          vertexData[o + 4] = sa * R;
//          vertexData[o + 5] = l;
//
//          vertexData[o + 6] = ca * R2;
//          vertexData[o + 7] = sa * R2;
//          vertexData[o + 8] = 0;
//
//          o = i * 3;
//          int o2 = (i == 0 ? (num_segments - 1) : i - 1) * 3;
//          int io = i * 3 * 3 * 2;
//
//          // Lateral
//          indexData[io++] = o;      indexData[io++] = o + 1;
//          indexData[io++] = o + 1;  indexData[io++] = o + 2;
//          indexData[io++] = o + 2;  indexData[io++] = o;
//
//          // Longitudal
//          indexData[io++] = o; indexData[io++] = o2;
//          indexData[io++] = o + 1; indexData[io++] = o2 + 1;
//          indexData[io++] = o + 2; indexData[io++] = o2 + 2;
//
//          // Diagonal
//          if (i & 1) {
//            indexData[io++] = o; indexData[io++] = o2 + 1;
//            indexData[io++] = o + 1; indexData[io++] = o2 + 2;
//            indexData[io++] = o + 2; indexData[io++] = o2;
//          } else {
//            indexData[io++] = o; indexData[io++] = o2 + 2;
//            indexData[io++] = o + 1; indexData[io++] = o2;
//            indexData[io++] = o + 2; indexData[io++] = o2 + 1;
//          }
//        }
//        auto bb = BoundingBox(Vector3(-10.5f, -10.5f, -1.0f), Vector3(10.5f, 10.5f, 1.0f));
//
//        const float r = 0.03f;
//        const int segments = 4;
//        const int sectors = 8;
//        Vector2 profile[] = {
//          {0.0f, 0.0f},
//          {0.2f, r},
//          {0.8f, r},
//          {1.0f, 0.0f}
//        };
//        Vector<Vector3> points;
//        //Vector<Vector3> normals;
//        PODVector<unsigned short> indexes;
//        for (int i = 0; i < segments - 1; ++i) {
//          for (int j = 0; j < sectors; ++j) {
//            float angle = 2 * M_PI * j / sectors;
//            float ca = cos(angle);
//            float sa = sin(angle);
//            Vector3 pos1(ca * profile[i].y_, sa * profile[i].y_, profile[i].x_);
//            Vector3 pos2(ca * profile[i + 1].y_, sa * profile[i + 1].y_, profile[i + 1].x_);
//            float dx = profile[i + 1].x_ - profile[i].x_;
//            float dy = profile[i + 1].y_ - profile[i].y_;
//            Vector3 norm(ca * dx, sa * dx, -dy);
//            points.Push(pos1);
//            points.Push(norm);
//            points.Push(pos2);
//            points.Push(norm);
//            /*normals.Push(norm);
//            normals.Push(norm);*/
//
//            // indexes
//            short int offset1 = i * 2 * sectors + j * 2;
//            int k = (j == 0) ? (sectors - 1) : (j - 1);
//            short int offset2 = i * 2 * sectors + k * 2;
//            indexes.Push(offset2);
//            indexes.Push(offset1 + 1);
//            indexes.Push(offset2 + 1);
//
//            indexes.Push(offset2);
//            indexes.Push(offset1);
//            indexes.Push(offset1 + 1);
//          }
//        }
//
//        Vector<Vector3> baked_points;
//        PODVector<unsigned int> baked_indexes;
//
//        for (int i = 0; i < num_line_indexes; i += 2) {
//          int i1 = indexData[i];
//          int i2 = indexData[i + 1];
//          Vector3 pos1(vertexData[i1 * 3], vertexData[i1 * 3 + 1], vertexData[i1 * 3 + 2]);
//          Vector3 pos2(vertexData[i2 * 3], vertexData[i2 * 3 + 1], vertexData[i2 * 3 + 2]);
//          Vector3 z_axis = pos2 - pos1;
//          Vector3 x_axis = z_axis.CrossProduct(Vector3(1.123, 1.55, 1.654)).Normalized();
//          Vector3 y_axis = z_axis.CrossProduct(x_axis).Normalized();
//          Matrix3x4 trans(
//            x_axis.x_, y_axis.x_, z_axis.x_, pos1.x_,
//            x_axis.y_, y_axis.y_, z_axis.y_, pos1.y_,
//            x_axis.z_, y_axis.z_, z_axis.z_, pos1.z_
//          );
//          Matrix3 norm_trans = trans.ToMatrix3();
//          unsigned int index_offset = baked_points.Size() / 2;
//          for (int j = 0; j < points.Size(); j += 2) {
//            baked_points.Push(trans * points[j]);
//            baked_points.Push(norm_trans * points[j + 1]);
//          }
//          for (int j = 0; j < indexes.Size(); ++j) {
//            baked_indexes.Push(indexes[j] + index_offset);
//          }
//        }
//
//        SharedPtr<VertexBuffer> vb(new VertexBuffer(context_));
//        SharedPtr<IndexBuffer> ib(new IndexBuffer(context_));
//
//        // Shadowed buffer needed for raycasts to work, and so that data can be automatically restored on device loss
//        vb->SetShadowed(true);
//        // We could use the "legacy" element bitmask to define elements for more compact code, but let's demonstrate
//        // defining the vertex elements explicitly to allow any element types and order
//        PODVector<VertexElement> elements;
//        elements.Push(VertexElement(TYPE_VECTOR3, SEM_POSITION));
//        //elements.Push(VertexElement(TYPE_VECTOR3, SEM_NORMAL));
//        vb->SetSize(numVertices, elements);
//        vb->SetData(vertexData);
//        //vb->SetSize(baked_points.Size() / 2, elements);
//        //vb->SetData(baked_points.Buffer());
//
//        ib->SetShadowed(true);
//        ib->SetSize(num_line_indexes, false);
//        ib->SetData(indexData);
//        //ib->SetSize(baked_indexes.Size(), true);
//        //ib->SetData(baked_indexes.Buffer());
//
//        // Node balls
//        SharedPtr<Model> balls_model(new Model(context_));
//        SharedPtr<Geometry> balls_geom(new Geometry(context_));
//
//        balls_geom->SetVertexBuffer(0, vb);
//        //geom->SetIndexBuffer(ib);
//        //geom->SetDrawRange(TRIANGLE_LIST, 0, numVertices);
//        balls_geom->SetDrawRange(POINT_LIST, 0, 0, 0, numVertices);
//
//        balls_model->SetNumGeometries(1);
//        balls_model->SetGeometry(0, 0, balls_geom);
//        balls_model->SetBoundingBox(bb);
//
//        // Though not necessary to render, the vertex & index buffers must be listed in the model so that it can be saved properly
//        /*Vector<SharedPtr<VertexBuffer>> vertexBuffers;
//        Vector<SharedPtr<IndexBuffer>> indexBuffers;
//        vertexBuffers.Push(vb);
//        indexBuffers.Push(ib);
//        // Morph ranges could also be not defined. Here we simply define a zero range (no morphing) for the vertex buffer
//        PODVector<unsigned> morphRangeStarts;
//        PODVector<unsigned> morphRangeCounts;
//        morphRangeStarts.Push(0);
//        morphRangeCounts.Push(0);
//        balls_model->SetVertexBuffers(vertexBuffers, morphRangeStarts, morphRangeCounts);
//        balls_model->SetIndexBuffers(indexBuffers);*/
//
//        /*for (int i = -count_z; i < count_z; ++i) {
//          Node* node = scene_->CreateChild("FromScratchObject");
//          node->SetPosition(Vector3(0.0f, 0.0f, i * l * 2.5f));
//          StaticModel* object = node->CreateComponent<StaticModel>();
//          object->SetModel(balls_model);
//          object->SetMaterial(cache->GetResource<Material>("Materials/GS/GSPointsToSpheresMat.xml"));
//          object->SetCastShadows(true);
//        }*/
//
//        // Beams
//        SharedPtr<Model> beams_model(new Model(context_));
//        SharedPtr<Geometry> beams_geom(new Geometry(context_));
//        beams_geom->SetVertexBuffer(0, vb);
//        beams_geom->SetIndexBuffer(ib);
//        beams_geom->SetDrawRange(LINE_LIST, 0, num_line_indexes);
//        //beams_geom->SetDrawRange(TRIANGLE_LIST, 0, baked_indexes.Size());
//
//        beams_model->SetNumGeometries(1);
//        beams_model->SetGeometry(0, 0, beams_geom);
//        beams_model->SetBoundingBox(bb);
//
//        int count_z = 20;
//        int count_xy = 0;
//        for (int i = -count_z; i < count_z + 1; ++i) {
//          for (int x = -count_xy; x < count_xy + 1; ++x) {
//            for (int y = -count_xy; y < count_xy + 1; ++y) {
//              Vector3 pos = Vector3(x * 22.0f, y * 22.0f, i * l * 2.5f);
//
//              Node* node = scene_->CreateChild("FromScratchObject");
//              node->SetPosition(pos);
//              StaticModel* object = node->CreateComponent<StaticModel>();
//              object->SetModel(balls_model);
//              auto mat = cache->GetResource<Material>("Materials/GS/GSPointsToSpheresMat.xml");
//              mat->Clone();
//              //mat->SetShaderParameter("Radius", 0.1f);
//              object->SetMaterial(mat);
//              object->SetCastShadows(true);
//
//              Node* node2 = scene_->CreateChild("FromScratchObject_2");
//              node2->SetPosition(pos);
//              StaticModel* object2 = node2->CreateComponent<StaticModel>();
//              object2->SetModel(beams_model);
//              object2->SetCastShadows(true);
//              object2->SetMaterial(cache->GetResource<Material>("Materials/GS/GSLinesToBeamsMat.xml"));
//              //object2->SetMaterial(cache->GetResource<Material>("Materials/PBR/MetallicRough5.xml"));
//            }
//          }
//        }
//    }
//
//    // Create the camera
//    cameraNode_ = new Node(context_);
//    cameraNode_->SetPosition(Vector3(0.0f, 2.0f, -20.0f));
//    Camera* camera = cameraNode_->CreateComponent<Camera>();
//    camera->SetFarClip(300.0f);
//}
//
//void DynamicGeometry::CreateInstructions()
//{
//    SetLogoVisible(false); // We need the full rendering window
//    ResourceCache* cache = GetSubsystem<ResourceCache>();
//    Graphics* graphics = GetSubsystem<Graphics>();
//    //Image* icon = cache->GetResource<Image>("Textures/UrhoIcon.png");
//    graphics->SetWindowIcon(0);
//    graphics->SetWindowTitle("Spacecraft Constructor");
//
//    /*ResourceCache* cache = GetSubsystem<ResourceCache>();
//    UI* ui = GetSubsystem<UI>();
//
//    // Construct new Text object, set string to display and font to use
//    Text* instructionText = ui->GetRoot()->CreateChild<Text>();
//    instructionText->SetText(
//        "Use WASD keys and mouse/touch to move\n"
//        "Space to toggle animation"
//    );
//    instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
//    // The text has multiple rows. Center them in relation to each other
//    instructionText->SetTextAlignment(HA_CENTER);
//
//    // Position the text relative to the screen center
//    instructionText->SetHorizontalAlignment(HA_CENTER);
//    instructionText->SetVerticalAlignment(VA_CENTER);
//    instructionText->SetPosition(0, ui->GetRoot()->GetHeight() / 4);*/
//}
//
//void DynamicGeometry::SetupViewport()
//{
//    Renderer* renderer = GetSubsystem<Renderer>();
//    renderer->SetHDRRendering(true);
//    renderer->SetShadowMapSize(2048);
//    renderer->SetDrawShadows(true);
//
//    Engine* engine = GetSubsystem<Engine>();
//    engine->SetMaxFps(0);
//
//    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
//    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
//    renderer->SetViewport(0, viewport);
//
//    // Add post-processing effects appropriate with the example scene
//    ResourceCache* cache = GetSubsystem<ResourceCache>();
//    SharedPtr<RenderPath> effectRenderPath = viewport->GetRenderPath()->Clone();
//    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA3.xml"));
//    //effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/GammaCorrection.xml"));
//    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/AutoExposure.xml"));
//    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
//    //effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/Bloom.xml"));
//
//    viewport->SetRenderPath(effectRenderPath);
//}
//
//void DynamicGeometry::SubscribeToEvents()
//{
//    // Subscribe HandleUpdate() function for processing update events
//    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(DynamicGeometry, HandleUpdate));
//}
//
//void DynamicGeometry::MoveCamera(float timeStep)
//{
//    // Do not move if the UI has a focused element (the console)
//    if (GetSubsystem<UI>()->GetFocusElement())
//        return;
//
//    Input* input = GetSubsystem<Input>();
//
//    // Movement speed as world units per second
//    const float MOVE_SPEED = 20.0f;
//    // Mouse sensitivity as degrees per pixel
//    const float MOUSE_SENSITIVITY = 0.1f;
//    // Mouse sensitivity as units per pixel
//    const float MOUSE_SENSITIVITY_UNITS = 0.02f;
//    // Mouse wheel sensitivity as units per pixel
//    const float MOUSE_WHEEL_SPEED = 0.4f;
//
//    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
//    IntVector2 mouseMove = input->GetMouseMove();
//    if (input->GetMouseButtonDown(MOUSEB_RIGHT)) {
//      yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
//      pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
//      pitch_ = Clamp(pitch_, -90.0f, 90.0f);
//      // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
//      cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
//      cameraNode_->Translate((Vector3::LEFT * mouseMove.x_ + Vector3::UP * mouseMove.y_) * 0.01);
//    }
//
//    if (input->GetMouseButtonDown(MOUSEB_MIDDLE)) {
//      cameraNode_->Translate((Vector3::LEFT * mouseMove.x_ + Vector3::UP * mouseMove.y_) * MOUSE_SENSITIVITY_UNITS);
//    }
//    cameraNode_->Translate((Vector3::FORWARD * input->GetMouseMoveWheel()) * MOUSE_WHEEL_SPEED);
//
//    // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
//    /*if (input->GetKeyDown(KEY_W))
//        cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
//    if (input->GetKeyDown(KEY_S))
//        cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
//    if (input->GetKeyDown(KEY_A))
//        cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
//    if (input->GetKeyDown(KEY_D))
//        cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);*/
//}
//
//void DynamicGeometry::AnimateObjects(float timeStep)
//{
//    URHO3D_PROFILE(AnimateObjects);
//
//    time_ += timeStep * 100.0f;
//
//    // Repeat for each of the cloned vertex buffers
//    /*for (unsigned i = 0; i < animatingBuffers_.Size(); ++i)
//    {
//        float startPhase = time_ + i * 30.0f;
//        VertexBuffer* buffer = animatingBuffers_[i];
//
//        // Lock the vertex buffer for update and rewrite positions with sine wave modulated ones
//        // Cannot use discard lock as there is other data (normals, UVs) that we are not overwriting
//        unsigned char* vertexData = (unsigned char*)buffer->Lock(0, buffer->GetVertexCount());
//        if (vertexData)
//        {
//            unsigned vertexSize = buffer->GetVertexSize();
//            unsigned numVertices = buffer->GetVertexCount();
//            for (unsigned j = 0; j < numVertices; ++j)
//            {
//                // If there are duplicate vertices, animate them in phase of the original
//                float phase = startPhase + vertexDuplicates_[j] * 10.0f;
//                Vector3& src = originalVertices_[j];
//                Vector3& dest = *reinterpret_cast<Vector3*>(vertexData + j * vertexSize);
//                dest.x_ = src.x_ * (1.0f + 0.1f * Sin(phase));
//                dest.y_ = src.y_ * (1.0f + 0.1f * Sin(phase + 60.0f));
//                dest.z_ = src.z_ * (1.0f + 0.1f * Sin(phase + 120.0f));
//            }
//
//            buffer->Unlock();
//        }
//    }*/
//}
//
//void DynamicGeometry::HandleUpdate(StringHash eventType, VariantMap& eventData)
//{
//    using namespace Update;
//
//    // Take the frame time step, which is stored as a float
//    float timeStep = eventData[P_TIMESTEP].GetFloat();
//
//    // Toggle animation with space
//    Input* input = GetSubsystem<Input>();
//    if (input->GetKeyPress(KEY_SPACE))
//        animate_ = !animate_;
//
//    // Move the camera, scale movement with time step
//    MoveCamera(timeStep);
//
//    // Animate objects' vertex data if enabled
//    if (animate_)
//        AnimateObjects(timeStep);
//}
