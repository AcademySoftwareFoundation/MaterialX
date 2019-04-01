#include <MaterialXView/Editor.h>

#include <MaterialXView/Viewer.h>

#include <nanogui/button.h>
#include <nanogui/combobox.h>
#include <nanogui/layout.h>

namespace {

class EditorFormHelper : public ng::FormHelper
{
  public:
    explicit EditorFormHelper(ng::Screen *screen) : ng::FormHelper(screen) { }
    ~EditorFormHelper() { }

    void setPreGroupSpacing(int val) { mPreGroupSpacing = val; }
    void setPostGroupSpacing(int val) { mPostGroupSpacing = val; }
    void setVariableSpacing(int val) { mVariableSpacing = val; }
};

} // anonymous namespace

//
// PropertyEditor methods
//

PropertyEditor::PropertyEditor() :
    _visible(false),
    _form(nullptr),
    _formWindow(nullptr),
    _fileDialogsForImages(true)
{
}

void PropertyEditor::create(Viewer& parent)
{
    ng::Window* parentWindow = parent.getWindow();
    if (!_form)
    {
        EditorFormHelper* form = new EditorFormHelper(&parent);
        form->setPreGroupSpacing(2);
        form->setPostGroupSpacing(2);
        form->setVariableSpacing(2);
        _form = form;
    }

    // Remove the window associated with the form.
    // This is done by explicitly creating and owning the window
    // as opposed to having it being done by the form
    ng::Vector2i previousPosition(15, parentWindow->height() + 60);
    if (_formWindow)
    {
        for (int i = 0; i < _formWindow->childCount(); i++)
        {
            _formWindow->removeChild(i);
        }
        // We don't want the property editor to move when
        // we update it's contents so cache any previous position
        // to use when we create a new window.
        previousPosition = _formWindow->position();
        parent.removeChild(_formWindow);
    }

    _formWindow = new ng::Window(&parent, "Property Editor");
    ng::AdvancedGridLayout* layout = new ng::AdvancedGridLayout({ 10, 0, 10, 0 }, {});
    layout->setMargin(2);
    layout->setColStretch(2, 0);
    if (previousPosition.x() < 0)
        previousPosition.x() = 0;
    if (previousPosition.y() < 0)
        previousPosition.y() = 0;
    _formWindow->setPosition(previousPosition);
    _formWindow->setVisible(_visible);
    _formWindow->setLayout(layout);
    _form->setWindow(_formWindow);
}

void PropertyEditor::addItemToForm(const mx::UIPropertyItem& item, const std::string& group,
                                   ng::FormHelper& form, Viewer* viewer)
{
    const mx::UIProperties& ui = item.ui;
    mx::ValuePtr value = item.variable->getValue();
    const std::string& label = item.label;
    const std::string& path = item.variable->getPath();
    mx::ValuePtr min = ui.uiMin;
    mx::ValuePtr max = ui.uiMax;
    const mx::StringVec& enumeration = ui.enumeration;
    const std::vector<mx::ValuePtr> enumValues = ui.enumerationValues;

    if (!value)
    {
        return;
    }

    if (!group.empty())
    {
        form.addGroup(group);
    }

    // Integer input. Can map to a combo box if an enumeration
    if (value->isA<int>())
    {
        int v = value->asA<int>();

        // Create a combo box. The items are the enumerations in order.
        if (v < (int) enumeration.size())
        {
            std::string enumValue = enumeration[v];

            ng::ComboBox* comboBox = new ng::ComboBox(form.window(), {""});
            comboBox->setItems(enumeration);
            comboBox->setSelectedIndex(v);
            comboBox->setFontSize(form.widgetFontSize());
            form.addWidget(label, comboBox);
            comboBox->setCallback([path, viewer, enumValues](int v)
            {
                MaterialPtr material = viewer->getSelectedMaterial();
                mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
                if (uniform)
                {
                    material->getShader()->bind();
                    if (v < (int) enumValues.size())
                    {
                        material->getShader()->setUniform(uniform->getName(), enumValues[v]->asA<int>());
                    }
                    else
                    {
                        material->getShader()->setUniform(uniform->getName(), v);
                    }
                }
            });
        }
        else
        {
            nanogui::detail::FormWidget<int, std::true_type>* intVar =
                form.addVariable(label, v, true);
            intVar->setSpinnable(true);
            intVar->setCallback([path, viewer](int v)
            {
                MaterialPtr material = viewer->getSelectedMaterial();
                if (material)
                {
                    mx::ShaderPort* uniform = material->findUniform(path);
                    if (uniform)
                    {
                        material->getShader()->bind();
                        material->getShader()->setUniform(uniform->getName(), v);
                    }
                }
            });
        }
    }

    // Float widget
    else if (value->isA<float>())
    {
        float v = value->asA<float>();
        nanogui::detail::FormWidget<float, std::true_type>* floatVar =
            form.addVariable(label, v, true);
        floatVar->setSpinnable(true);
        if (min)
            floatVar->setMinValue(min->asA<float>());
        if (max)
            floatVar->setMaxValue(max->asA<float>());
        floatVar->setCallback([path, viewer](float v)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            if (material)
            {
                mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
                if (uniform)
                {
                    material->getShader()->bind();
                    material->getShader()->setUniform(uniform->getName(), v);
                }                
            }
        });
    }

    // Color2 input
    else if (value->isA<mx::Color2>())
    {
        mx::Color2 v = value->asA<mx::Color2>();
        ng::Color c;
        c.r() = v[0];
        c.g() = v[1];
        c.b() = 0.0f;
        c.w() = 1.0f;
        nanogui::detail::FormWidget<nanogui::Color, std::true_type>* colorVar =
            form.addVariable(label, c, true);
        colorVar->setFinalCallback([path, viewer, colorVar](const ng::Color &c)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
            if (uniform)
            {
                material->getShader()->bind();
                ng::Vector2f v;
                v.x() = c.r();
                v.y() = c.g();
                material->getShader()->setUniform(uniform->getName(), v);
                ng::Color c2 = c;
                c2.b() = 0.0f;
                c2.w() = 1.0f;
                colorVar->setValue(c2);
            }
        });
    }

    // Color3 input. Can map to a combo box if an enumeration
    else if (value->isA<mx::Color3>())
    {
        // Determine if there is an enumeration for this
        mx::Color3 color = value->asA<mx::Color3>();
        int index = -1;
        if (enumeration.size() && enumValues.size())
        {
            index = 0;
            for (size_t i = 0; i < enumValues.size(); i++)
            {
                if (enumValues[i]->asA<mx::Color3>() == color)
                {
                    index = static_cast<int>(i);
                    break;
                }
            }
        }

        // Create a combo box. The items are the enumerations in order.
        if (index >= 0)
        {
            ng::ComboBox* comboBox = new ng::ComboBox(form.window(), { "" });
            comboBox->setItems(enumeration);
            comboBox->setSelectedIndex(index);
            comboBox->setFontSize(form.widgetFontSize());
            form.addWidget(label, comboBox);
            comboBox->setCallback([path, enumValues, viewer](int index)
            {
                MaterialPtr material = viewer->getSelectedMaterial();
                mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
                if (uniform)
                {
                    material->getShader()->bind();
                    if (index < (int) enumValues.size())
                    {
                        mx::Color3 c = enumValues[index]->asA<mx::Color3>();
                        ng::Vector3f v;
                        v.x() = c[0];
                        v.y() = c[1];
                        v.z() = c[2];
                        material->getShader()->setUniform(uniform->getName(), v);
                    }
                }
            });
        }
        else
        {
            mx::Color3 v = value->asA<mx::Color3>();
            ng::Color c;
            c.r() = v[0];
            c.g() = v[1];
            c.b() = v[2];
            c.w() = 1.0;
            nanogui::detail::FormWidget<nanogui::Color, std::true_type>* colorVar =
                form.addVariable(label, c, true);
            colorVar->setFinalCallback([path, viewer](const ng::Color &c)
            {
                MaterialPtr material = viewer->getSelectedMaterial();
                mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
                if (uniform)
                {
                    material->getShader()->bind();
                    ng::Vector3f v;
                    v.x() = c.r();
                    v.y() = c.g();
                    v.z() = c.b();
                    material->getShader()->setUniform(uniform->getName(), v);
                }
            });
        }
    }

    // Color4 input
    else if (value->isA<mx::Color4>())
    {
        mx::Color4 v = value->asA<mx::Color4>();
        ng::Color c;
        c.r() = v[0];
        c.g() = v[1];
        c.b() = v[2];
        c.w() = v[3];
        nanogui::detail::FormWidget<nanogui::Color, std::true_type>* colorVar =
            form.addVariable(label, c, true);
        colorVar->setFinalCallback([path, viewer](const ng::Color &c)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
            if (uniform)
            {
                material->getShader()->bind();
                ng::Vector4f v;
                v.x() = c.r();
                v.y() = c.g();
                v.z() = c.b();
                v.w() = c.w();
                material->getShader()->setUniform(uniform->getName(), v);
            }
        });
    }

    // Vec 2 widget
    else if (value->isA<mx::Vector2>())
    {
        mx::Vector2 v = value->asA<mx::Vector2>();
        nanogui::detail::FormWidget<float, std::true_type>* v1 =
            form.addVariable(label + ".x", v[0], true);
        nanogui::detail::FormWidget<float, std::true_type>* v2 =
            form.addVariable(label + ".y", v[1], true);
        v1->setCallback([v2, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
            if (uniform)
            {
                material->getShader()->bind();
                ng::Vector2f v;
                v.x() = f;
                v.y() = v2->value();
                material->getShader()->setUniform(uniform->getName(), v);
            }
        });
        v1->setSpinnable(true);
        v2->setCallback([v1, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
            if (uniform)
            {
                material->getShader()->bind();
                ng::Vector2f v;
                v.x() = v1->value();
                v.y() = f;
                material->getShader()->setUniform(uniform->getName(), v);
            }
        });
        v2->setSpinnable(true);
    }

    // Vec 3 input
    else if (value->isA<mx::Vector3>())
    {
        mx::Vector3 v = value->asA<mx::Vector3>();
        nanogui::detail::FormWidget<float, std::true_type>* v1 = 
            form.addVariable(label + ".x", v[0], true);
        nanogui::detail::FormWidget<float, std::true_type>* v2 =
            form.addVariable(label + ".y", v[1], true);
        nanogui::detail::FormWidget<float, std::true_type>* v3 =
            form.addVariable(label + ".z", v[2], true);
        v1->setCallback([v2, v3, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
            if (uniform)
            {
                material->getShader()->bind();
                ng::Vector3f v;
                v.x() = f;
                v.y() = v2->value();
                v.z() = v3->value();
                material->getShader()->setUniform(uniform->getName(), v);
            }
        });
        v1->setSpinnable(true);
        v2->setCallback([v1, v3, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
            if (uniform)
            {
                material->getShader()->bind();
                ng::Vector3f v;
                v.x() = v1->value();
                v.y() = f;
                v.z() = v3->value();
                material->getShader()->setUniform(uniform->getName(), v);
            }
        });
        v2->setSpinnable(true);
        v3->setCallback([v1, v2, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
            if (uniform)
            {
                material->getShader()->bind();
                ng::Vector3f v;
                v.x() = v1->value();
                v.y() = v2->value();
                v.z() = f;
                material->getShader()->setUniform(uniform->getName(), v);
            }
        });
        v3->setSpinnable(true);
    }

    // Vec 4 input
    else if (value->isA<mx::Vector4>())
    {
        mx::Vector4 v = value->asA<mx::Vector4>();
        nanogui::detail::FormWidget<float, std::true_type>* v1 =
            form.addVariable(label + ".x", v[0], true);
        nanogui::detail::FormWidget<float, std::true_type>* v2 =
            form.addVariable(label + ".y", v[1], true);
        nanogui::detail::FormWidget<float, std::true_type>* v3 =
            form.addVariable(label + ".z", v[2], true);
        nanogui::detail::FormWidget<float, std::true_type>* v4 =
            form.addVariable(label + ".w", v[3], true);
        v1->setCallback([v2, v3, v4, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
            if (uniform)
            {
                material->getShader()->bind();
                ng::Vector4f v;
                v.x() = f;
                v.y() = v2->value();
                v.z() = v3->value();
                v.w() = v4->value();
                material->getShader()->setUniform(uniform->getName(), v);
            }
        });
        v1->setSpinnable(true);
        v2->setCallback([v1, v3, v4, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
            if (uniform)
            {
                material->getShader()->bind();
                ng::Vector4f v;
                v.x() = v1->value();
                v.y() = f;
                v.z() = v3->value();
                v.w() = v4->value();
                material->getShader()->setUniform(uniform->getName(), v);
            }
        });
        v2->setSpinnable(true);
        v3->setCallback([v1, v2, v4, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
            if (uniform)
            {
                material->getShader()->bind();
                ng::Vector4f v;
                v.x() = v1->value();
                v.y() = v2->value();
                v.z() = f;
                v.w() = v4->value();
                material->getShader()->setUniform(uniform->getName(), v);
            }
        });
        v3->setSpinnable(true);
        v4->setCallback([v1, v2, v3, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
            if (uniform)
            {
                material->getShader()->bind();
                ng::Vector4f v;
                v.x() = v1->value();
                v.y() = v2->value();
                v.z() = v3->value();
                v.w() = f;
                material->getShader()->setUniform(uniform->getName(), v);
            }
        });
        v4->setSpinnable(true);
    }

    // String
    else if (value->isA<std::string>())
    {
        std::string v = value->asA<std::string>();
        if (!v.empty())
        {
            if (_fileDialogsForImages && item.variable->getType() == MaterialX::Type::FILENAME)
            {
                ng::Button* buttonVar = new ng::Button(form.window(), mx::FilePath(v).getBaseName());
                form.addWidget(label, buttonVar);
                buttonVar->setFontSize(form.widgetFontSize()-1);
                buttonVar->setCallback([buttonVar, path, viewer]()
                {
                    MaterialPtr material = viewer->getSelectedMaterial();
                    mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
                    if (uniform)
                    {
                        if (uniform->getType() == MaterialX::Type::FILENAME)
                        {
                            const mx::GLTextureHandlerPtr handler = viewer->getImageHandler();
                            if (handler)
                            {
                                mx::StringSet extensions;
                                handler->supportedExtensions(extensions);
                                std::vector<std::pair<std::string, std::string>> filetypes;
                                for (auto extension : extensions)
                                {
                                    filetypes.push_back(std::make_pair(extension, extension));
                                }
                                std::string filename = ng::file_dialog(filetypes, false);
                                if (!filename.empty())
                                {
                                    uniform->setValue(mx::Value::createValue<std::string>(filename));
                                    buttonVar->setCaption(mx::FilePath(filename).getBaseName());
                                    viewer->performLayout();
                                }
                            }
                        }
                    }
                });
            }
            else
            {
                nanogui::detail::FormWidget<std::string, std::true_type>* stringVar =
                    form.addVariable(label, v, true);
                stringVar->setCallback([path, viewer](const std::string &v)
                {
                    MaterialPtr material = viewer->getSelectedMaterial();
                    mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
                    if (uniform)
                    {
                        if (uniform->getType() == MaterialX::Type::FILENAME)
                        {
                            const std::string& filename = viewer->getSearchPath().find(v);
                            uniform->setValue(mx::Value::createValue<std::string>(filename));
                        }
                        else
                        {
                            uniform->setValue(mx::Value::createValue<std::string>(v));
                        }
                    }
                });
            }
        }
    }
}

void PropertyEditor::updateContents(Viewer* viewer)
{
    create(*viewer);

    MaterialPtr material = viewer->getSelectedMaterial();
    mx::TypedElementPtr materialElement = material ? material->getElement() : nullptr;
    if (!materialElement)
    {
        return;
    }

    mx::DocumentPtr contentDocument = viewer->getCurrentDocument();
    if (!contentDocument)
    {
        return;
    }

    const MaterialX::VariableBlock* publicUniforms = material->getPublicUniforms();
    if (publicUniforms)
    {
        mx::UIPropertyGroup groups;
        mx::UIPropertyGroup unnamedGroups;
        const std::string pathSeparator(":");
        mx::createUIPropertyGroups(*publicUniforms, contentDocument, materialElement,
                                    pathSeparator, groups, unnamedGroups); 

        std::string previousFolder;
        for (auto it = groups.begin(); it != groups.end(); ++it)
        {
            const std::string& folder = it->first;
            const mx::UIPropertyItem& item = it->second;
            addItemToForm(item, (previousFolder == folder) ? mx::EMPTY_STRING : folder, *_form, viewer);
            previousFolder.assign(folder);
        }
        if (!unnamedGroups.empty())
        {
            _form->addGroup("Other");
        }
        for (auto it2 = unnamedGroups.begin(); it2 != unnamedGroups.end(); ++it2)
        {
            addItemToForm(it2->second, mx::EMPTY_STRING, *_form, viewer);
        }
    }
    viewer->performLayout();
}
