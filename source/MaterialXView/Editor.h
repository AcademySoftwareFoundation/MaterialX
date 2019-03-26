#ifndef MATERIALXVIEW_EDITOR_H
#define MATERIALXVIEW_EDITOR_H

#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <nanogui/formhelper.h>
#include <nanogui/screen.h>

namespace mx = MaterialX;
namespace ng = nanogui;

class Viewer;

struct EditorItem
{
    std::string label;
    mx::ShaderPort* variable = nullptr;
    mx::UIProperties ui;
};

class PropertyEditor 
{
  public:
    PropertyEditor();
    void updateContents(Viewer* viewer);

    bool visible() const
    {
        return _visible;
    }

    void setVisible(bool value)
    {
        if (value != _visible)
        {
            _visible = value;
            _formWindow->setVisible(_visible);
        }
    }

  protected:
    void create(Viewer& parent);
    void addItemToForm(const EditorItem& item, const std::string& group,
                       ng::FormHelper& form, Viewer* viewer);
      
    bool _visible;
    ng::FormHelper* _form;
    ng::Window* _formWindow;
    bool _fileDialogsForImages;
};

#endif // MATERIALXVIEW_EDITOR_H
