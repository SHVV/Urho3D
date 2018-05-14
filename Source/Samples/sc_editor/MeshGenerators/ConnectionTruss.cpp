// SHW Spacecraft editor
//
// Connection truss generation function.

#include "ConnectionTruss.h"

// Editor Includes
#include "../Core/MeshGenerator.h"
#include "../Core/Polyline2.h"
#include "../Model/DynamicModel.h"
#include "../Model/Voxel1DAttachableSurface.h"
#include "../Model/ProceduralUnit.h"

#include <Urho3D\Scene\Scene.h>

/// Generator name
String ConnectionTruss::s_name = "ConnectionTruss";

/// Nodes list parameter ID
ParameterID ConnectionTruss::s_node_ids = 0;      // External 

/// Length in meters parameter ID
ParameterID ConnectionTruss::s_length = 0;        // Internal

/// Size of connection nodes parameter ID
ParameterID ConnectionTruss::s_cell_size = 3;  // Internal

/// Truss segments parameter ID
ParameterID ConnectionTruss::s_segments = 1;      // Both

/// Truss curvature parameter ID
ParameterID ConnectionTruss::s_curvature = 2;     // Both

/// Default constructor
ConnectionTruss::ConnectionTruss()
  : MeshGenerationFunction(s_name)
{
  // Set parameters
  add_parameter(s_node_ids, VariantVector(), pfTEMPORARY | pfNOT_EDITABLE, "Node IDs", "List of reference node IDs");
  add_parameter(s_segments, 4, pfINTERACTIVE, "Sides", "Number of truss sides", 3, 12);
  add_parameter(s_curvature, 0.0, pfNONE, "Curvature", "Truss curvature", 0.0, 1.0);
}

/// Destructor
ConnectionTruss::~ConnectionTruss()
{
}

/// Generate mesh geometry, basing on parameters
MeshGeometry* ConnectionTruss::generate(const Parameters& parameters)
{
  // Parameters
  float cell_size = parameters[s_cell_size].GetFloat();
  float length = parameters[s_length].GetFloat();
  int segments = parameters[s_segments].GetUInt();
  float curvature = parameters[s_curvature].GetFloat();
  // TODO: add rest parameters

  // Recalculate cell size based on cells count
  int cells = Ceil(length / cell_size);
  cell_size = length / cells;

  float radius = cell_size / sin(M_PI / segments) / 2;
  
  Polyline2 profile;
  PolylineSegment seg;

  seg.m_lateral_material_id = 0;
  seg.m_longitudal_material_id = 0;
  seg.m_node_material_id = 0;
  seg.m_plate_material_id = -1;

  seg.m_node_radius = cell_size / 25;
  seg.m_smooth_vertex = false;
  seg.m_smooth_segment = false;

  seg.m_pos = Vector2(-cell_size * cells * 0.5, 0);
  profile.segments().Push(seg);

  for (int i = 1; i < cells; ++i) {
    seg.m_pos = Vector2((-cells * 0.5 + i) * cell_size, radius);
    profile.segments().Push(seg);
  }

  seg.m_pos = Vector2(cell_size * cells * 0.5, 0);
  profile.segments().Push(seg);

  MeshGeometry* geometry = generator()->lathe(
    profile, 
    segments,
    //ttTRIANGLE
    ttDIAMOND,
    M_PI / segments,
    M_PI / segments
  );
  float scale = Min(cell_size * cells, radius * 2) * 1.3;
  for (int i = 0; i < geometry->vertices().Size(); ++i) {
    geometry->set_scale(i, scale);
    geometry->set_vertex_flags(i, mgfATTACHABLE | mgfVISIBLE);
  }
  for (int i = 0; i < geometry->edges().Size(); ++i) {
    if (geometry->edges()[i].secondary) {
      geometry->set_edge_flags(i, mgfVISIBLE);
    } else {
      geometry->set_edge_flags(i, mgfATTACHABLE | mgfVISIBLE);
    }
  }
  for (int i = 0; i < geometry->polygons().Size(); ++i) {
    geometry->set_polygon_flags(i, mgfATTACHABLE);
  }
  return geometry;
}

/// Update procedural unit guts
void ConnectionTruss::update_unit(
  const Parameters& parameters,
  ProceduralUnit* unit
)
{
  // First parameter should be variant vector
  if (parameters[s_node_ids].GetType() != VariantType::VAR_VARIANTVECTOR) {
    return;
  };
  
  const VariantVector& node_ids = parameters[s_node_ids].GetVariantVector();
  // Should have two nodes
  if (node_ids.Size() == 2) {
    Scene* scene = unit->GetScene();
    Node* node1 = scene->GetNode(node_ids[0].GetUInt());
    Node* node2 = scene->GetNode(node_ids[1].GetUInt());

    // Check both nodes
    if (!node1 || !node2) {
      return;
    }
    // Caclulate and fill parameters for mesh function
    Parameters mesh_parameters;
    // Calculate length
    float length = (node1->GetWorldPosition() - node2->GetWorldPosition()).Length();

    // TODO: get cell size from connections
    int cells = 15;
    float cell_size = length / cells;
    // Quantize length to reduce rounding errors
    float step = cell_size / 100;
    int step_log = floor(log2(step));
    step = pow(2, step_log);
    length = Ceil(length / step) * step;
    cell_size = length / cells;
    // Fill parameters
    mesh_parameters[s_length] = length;
    mesh_parameters[s_cell_size] = cell_size;
    mesh_parameters[s_segments] = parameters[s_segments];
    mesh_parameters[s_curvature] = parameters[s_curvature];

    MeshBuffer* mesh_buffer = generator()->generate_buffer(name(), mesh_parameters);
    if (mesh_buffer) {
      DynamicModel* dynamic_model = unit->get_component<DynamicModel>();
      dynamic_model->mesh_buffer(mesh_buffer);
    }

    // Create voxel 1D attachable surface
    auto* surface = unit->get_component<Voxel1DAttachableSurface>();
    surface->initialize(cell_size, (cells & 1) ? (cell_size * 0.5) : 0);
  }
  // TODO: add some marker for one node mode
}
