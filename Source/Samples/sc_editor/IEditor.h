// SHW Spacecraft editor
//
// Main application class interface for components interaction

#pragma once

class SceneModel;
class SceneView;
class BaseContext;
class ContextToolbar;
class EditorUI;

extern const char* EDITOR_CATEGORY;

class IEditor
{
public:
  /// Construct.
  IEditor() = default;

  /// Destructor
  virtual ~IEditor() = default;

  /// Returns scene model
  virtual SceneModel* model() = 0;

  /// Returns scene view
  virtual SceneView* view() = 0;

  /// Returns EditorUI
  virtual EditorUI* ui() = 0;

  /// Returns context switcher
  virtual ContextToolbar* context_toolbar() = 0;

  /// Set current editor's context
  virtual void set_context(BaseContext* context) = 0;

  /// Set symmetry
  virtual void set_symmetry(int value) = 0;
};
