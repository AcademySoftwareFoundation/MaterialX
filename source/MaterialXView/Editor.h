#ifndef MATERIALXVIEW_EDITOR_H
#define MATERIALXVIEW_EDITOR_H

#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXRender/Util.h>

#include <nanogui/formhelper.h>
#include <nanogui/screen.h>

namespace mx = MaterialX;
namespace ng = nanogui;

class Viewer;

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
    void addItemToForm(const mx::UIPropertyItem& item, const std::string& group,
                       ng::Widget* container, Viewer* viewer, bool editable);
      
    bool _visible;
    ng::Widget* _container;
    ng::Window* _formWindow;
    bool _fileDialogsForImages;
};

#endif // MATERIALXVIEW_EDITOR_H
