#ifndef MATERIALXVIEW_EDITOR_H
#define MATERIALXVIEW_EDITOR_H

#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXRender/Util.h>

#include <nanogui/formhelper.h>
#include <nanogui/screen.h>
#include <nanogui/textbox.h>

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
    ng::FloatBox<float>* makeFloatWidget(ng::Widget* container, const std::string& label, mx::ValuePtr value,
                       bool editable, mx::ValuePtr min, mx::ValuePtr max, Viewer* viewer, const std::string& path);

    ng::Widget* _container;
    ng::Window* _formWindow;
    ng::GridLayout* _gridLayout2;
    ng::GridLayout* _gridLayout3;
    bool _visible;
    bool _fileDialogsForImages;
};

#endif // MATERIALXVIEW_EDITOR_H
