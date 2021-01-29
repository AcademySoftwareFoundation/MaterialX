#include <MaterialXView/Editor.h>

#include <MaterialXView/Viewer.h>

#include <nanogui/button.h>
#include <nanogui/colorpicker.h>
#include <nanogui/colorwheel.h>
#include <nanogui/combobox.h>
#include <nanogui/layout.h>
#include <nanogui/slider.h>
#include <nanogui/textbox.h>
#include <nanogui/vscrollpanel.h>

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

// Custom color picker with numeric entry and feedback.
//
class EditorColorPicker : public ng::ColorPicker
{
  public:
    EditorColorPicker(ng::Widget *parent, const ng::Color& color) :
        ng::ColorPicker(parent, color)
    {
        ng::Popup *popup = this->popup();
        ng::Widget *floatGroup = new ng::Widget(popup);
        auto layout =
            new ng::GridLayout(ng::Orientation::Horizontal, 2,
                ng::Alignment::Middle, 2, 2);
        layout->setColAlignment({ ng::Alignment::Fill, ng::Alignment::Fill });
        layout->setSpacing(1, 1);
        floatGroup->setLayout(layout);

        const std::vector<std::string> colorLabels = { "Red", "Green", "Blue", "Alpha" };
        for (size_t i = 0; i < colorLabels.size(); i++)
        {
            new ng::Label(floatGroup, colorLabels[i]);
            _colorWidgets[i] = new ng::FloatBox<float>(floatGroup, color[i]);
            _colorWidgets[i]->setEditable(true);
            _colorWidgets[i]->setAlignment(ng::TextBox::Alignment::Right);
            _colorWidgets[i]->setFixedSize(ng::Vector2i(70, 20));
            _colorWidgets[i]->setFontSize(15);
            _colorWidgets[i]->setSpinnable(true);
            _colorWidgets[i]->setCallback([this](float)
            {
                ng::Color value(_colorWidgets[0]->value(), _colorWidgets[1]->value(), _colorWidgets[2]->value(), _colorWidgets[3]->value());
                mColorWheel->setColor(value);
                mPickButton->setBackgroundColor(value);
                mPickButton->setTextColor(value.contrastingColor());
            });
        }

        // The color wheel does not handle alpha properly, so only
        // overwrite RGB in the callback.
        mCallback = [this](const ng::Color &value) {
            _colorWidgets[0]->setValue(value[0]);
            _colorWidgets[1]->setValue(value[1]);
            _colorWidgets[2]->setValue(value[2]);
        };
    }

  protected:
    // Additional numeric entry / feedback widgets
    ng::FloatBox<float>* _colorWidgets[4];
};

} // anonymous namespace

//
// PropertyEditor methods
//

PropertyEditor::PropertyEditor() :
    _window(nullptr),
    _container(nullptr),
    _gridLayout2(nullptr),
    _gridLayout3(nullptr),
    _visible(false),
    _fileDialogsForImages(true)
{
}

void PropertyEditor::create(Viewer& parent)
{
    ng::Window* parentWindow = parent.getWindow();

    // Remove the window associated with the form.
    // This is done by explicitly creating and owning the window
    // as opposed to having it being done by the form
    ng::Vector2i previousPosition(15, parentWindow->height());
    if (_window)
    {
        for (int i = 0; i < _window->childCount(); i++)
        {
            _window->removeChild(i);
        }
        // We don't want the property editor to move when
        // we update it's contents so cache any previous position
        // to use when we create a new window.
        previousPosition = _window->position();
        parent.removeChild(_window);
    }

    if (previousPosition.x() < 0)
        previousPosition.x() = 0;
    if (previousPosition.y() < 0)
        previousPosition.y() = 0;

    _window = new ng::Window(&parent, "Property Editor");
    _window->setLayout(new ng::GroupLayout());
    _window->setPosition(previousPosition);
    _window->setVisible(_visible);

    ng::VScrollPanel *scroll_panel = new ng::VScrollPanel(_window);
    scroll_panel->setFixedHeight(300);
    _container = new ng::Widget(scroll_panel);
    _container->setLayout(new ng::GroupLayout(1, 1, 1, 1));

    // 2 cell layout for label plus value pair.
    _gridLayout2 = new ng::GridLayout(ng::Orientation::Horizontal, 2,
                                      ng::Alignment::Minimum, 2, 2);
    _gridLayout2->setColAlignment({ ng::Alignment::Minimum, ng::Alignment::Maximum });

    // 3 cell layout for label plus widget value pair.
    _gridLayout3 = new ng::GridLayout(ng::Orientation::Horizontal, 3,
        ng::Alignment::Minimum, 2, 2);
    _gridLayout3->setColAlignment({ ng::Alignment::Minimum, ng::Alignment::Maximum, ng::Alignment::Maximum });
}

void PropertyEditor::addItemToForm(const mx::UIPropertyItem& item, const std::string& group,
                                   ng::Widget* container, Viewer* viewer, bool editable)
{
    const mx::UIProperties& ui = item.ui;
    mx::ValuePtr value = item.variable->getValue();
    if (!value)
    {
        return;
    }

    std::string label = item.label;
    const std::string& unit = item.variable->getUnit();
    if (!unit.empty())
    {
        label += std::string(" (") + unit + std::string(")");
    }
    const std::string& path = item.variable->getPath();
    const mx::StringVec& enumeration = ui.enumeration;
    const std::vector<mx::ValuePtr> enumValues = ui.enumerationValues;

    if (!group.empty())
    {
        ng::Widget* twoColumns = new ng::Widget(container);
        twoColumns->setLayout(_gridLayout2);
        ng::Label* groupLabel =  new ng::Label(twoColumns, group);
        groupLabel->setFontSize(20);
        groupLabel->setFont("sans-bold");
        new ng::Label(twoColumns, "");
    }

    // Integer input. Can map to a combo box if an enumeration
    if (value->isA<int>())
    {
        const size_t INVALID_INDEX = std::numeric_limits<size_t>::max();
        auto indexInEnumeration = [&value, &enumValues, &enumeration]()
        {
            size_t index = 0;
            for (auto& enumValue: enumValues)
            {
                if (value->getValueString() == enumValue->getValueString())
                {
                    return index;
                }
                index++;
            }
            index = 0;
            for (auto& enumName: enumeration)
            {
                if (value->getValueString() == enumName)
                {
                    return index;
                }
                index++;
            }
            return std::numeric_limits<size_t>::max(); // INVALID_INDEX;
        };

        // Create a combo box. The items are the enumerations in order.
        const size_t valueIndex = indexInEnumeration();
        if (INVALID_INDEX != valueIndex)
        {
            ng::Widget* twoColumns = new ng::Widget(container);
            twoColumns->setLayout(_gridLayout2);

            new ng::Label(twoColumns, label);
            ng::ComboBox* comboBox = new ng::ComboBox(twoColumns, {""});
            comboBox->setEnabled(editable);
            comboBox->setItems(enumeration);
            comboBox->setSelectedIndex(static_cast<int>(valueIndex));
            comboBox->setFixedSize(ng::Vector2i(100, 20));
            comboBox->setFontSize(15);
            comboBox->setCallback([path, viewer, enumeration, enumValues](int index)
            {
                MaterialPtr material = viewer->getSelectedMaterial();
                if (index >= 0 && static_cast<size_t>(index) < enumValues.size())
                {
                    material->modifyUniform(path, enumValues[index]);
                }
                else if (index >= 0 && static_cast<size_t>(index) < enumeration.size())
                {
                    material->modifyUniform(path, mx::Value::createValue(index), enumeration[index]);
                }
            });
        }
        else
        {
            ng::Widget* twoColumns = new ng::Widget(container);
            twoColumns->setLayout(_gridLayout2);

            new ng::Label(twoColumns, label);
            auto intVar = new ng::IntBox<int>(twoColumns);
            intVar->setFixedSize(ng::Vector2i(100, 20));
            intVar->setFontSize(15);
            intVar->setEditable(editable);
            intVar->setSpinnable(editable);
            intVar->setCallback([intVar, path, viewer](int /*unclamped*/)
            {
                MaterialPtr material = viewer->getSelectedMaterial();
                if (material)
                {
                    // https://github.com/wjakob/nanogui/issues/205
                    material->modifyUniform(path, mx::Value::createValue(intVar->value()));
                }
            });
            if (ui.uiMin)
            {
                intVar->setMinValue(ui.uiMin->asA<int>());
            }
            if (ui.uiMax)
            {
                intVar->setMaxValue(ui.uiMax->asA<int>());
            }
            if (ui.uiStep)
            {
                intVar->setValueIncrement(ui.uiStep->asA<int>());
            }
            intVar->setValue(value->asA<int>());
        }
    }

    // Float widget
    else if (value->isA<float>())
    {
        ng::Widget* threeColumns = new ng::Widget(container);
        threeColumns->setLayout(_gridLayout3);
        ng::FloatBox<float>* floatBox = createFloatWidget(threeColumns, label, value->asA<float>(), &ui, [viewer, path](float value)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            if (material)
            {
                material->modifyUniform(path, mx::Value::createValue(value));            
            }
        });
        floatBox->setFixedSize(ng::Vector2i(100, 20));
        floatBox->setEditable(editable);
    }

    // Boolean widget
    else if (value->isA<bool>())
    {
        ng::Widget* twoColumns = new ng::Widget(container);
        twoColumns->setLayout(_gridLayout2);

        bool v = value->asA<bool>();
        new ng::Label(twoColumns, label);
        ng::CheckBox* boolVar = new ng::CheckBox(twoColumns, "");
        boolVar->setChecked(v);
        boolVar->setFontSize(15);
        boolVar->setCallback([path, viewer](bool v)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            if (material)
            {
                material->modifyUniform(path, mx::Value::createValue((float) v));
            }
        });
    }

    // Color3 input. Can map to a combo box if an enumeration
    else if (value->isA<mx::Color3>())
    {
        ng::Widget* twoColumns = new ng::Widget(container);
        twoColumns->setLayout(_gridLayout2);

        // Determine if there is an enumeration for this
        mx::Color3 color = value->asA<mx::Color3>();
        int index = -1;
        if (!enumeration.empty() && !enumValues.empty())
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
            ng::ComboBox* comboBox = new ng::ComboBox(twoColumns, { "" });
            comboBox->setEnabled(editable);
            comboBox->setItems(enumeration);
            comboBox->setSelectedIndex(index);
            comboBox->setFontSize(15);
            comboBox->setCallback([path, enumValues, viewer](int index)
            {
                MaterialPtr material = viewer->getSelectedMaterial();
                if (material)
                {
                    if (index < (int) enumValues.size())
                    {
                        material->modifyUniform(path, enumValues[index]);
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
            
            new ng::Label(twoColumns, label);
            auto colorVar = new EditorColorPicker(twoColumns, c);
            colorVar->setFixedSize({ 100, 20 });
            colorVar->setFontSize(15);
            colorVar->setFinalCallback([path, viewer](const ng::Color &c)
            {
                MaterialPtr material = viewer->getSelectedMaterial();
                mx::Vector3 v;
                v[0] = c.r();
                v[1] = c.g();
                v[2] = c.b();
                material->modifyUniform(path, mx::Value::createValue(v));
            });
        }
    }

    // Color4 input
    else if (value->isA<mx::Color4>())
    {
        ng::Widget* twoColumns = new ng::Widget(container);
        twoColumns->setLayout(_gridLayout2);

        new ng::Label(twoColumns, label);
        mx::Color4 v = value->asA<mx::Color4>();
        ng::Color c;
        c.r() = v[0];
        c.g() = v[1];
        c.b() = v[2];
        c.w() = v[3];
        auto colorVar = new EditorColorPicker(twoColumns, c);
        colorVar->setFixedSize({ 100, 20 });
        colorVar->setFontSize(15);
        colorVar->setFinalCallback([path, viewer](const ng::Color &c)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            if (material)
            {
                mx::Vector4 v;
                v[0] = c.r();
                v[1] = c.g();
                v[2] = c.b();
                v[3] = c.w();
                material->modifyUniform(path, mx::Value::createValue(v));
            }
        });
    }

    // Vec 2 widget
    else if (value->isA<mx::Vector2>())
    {
        ng::Widget* twoColumns = new ng::Widget(container);
        twoColumns->setLayout(_gridLayout2);

        mx::Vector2 v = value->asA<mx::Vector2>();
        new ng::Label(twoColumns, label + ".x");
        auto v1 = new ng::FloatBox<float>(twoColumns, v[0]);
        v1->setFixedSize({ 100, 20 });
        v1->setFontSize(15);
        new ng::Label(twoColumns, label + ".y");
        auto v2 = new ng::FloatBox<float>(twoColumns, v[1]);
        v2->setFixedSize({ 100, 20 });
        v2->setFontSize(15);
        v1->setCallback([v2, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            if (material)
            {
                mx::Vector2 v;
                v[0] = f;
                v[1] = v2->value();
                material->modifyUniform(path, mx::Value::createValue(v));
            }
        });
        v1->setSpinnable(editable);
        v1->setEditable(editable);
        v2->setCallback([v1, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            if (material)
            {
                mx::Vector2 v;
                v[0] = v1->value();
                v[1] = f;
                material->modifyUniform(path, mx::Value::createValue(v));
            }
        });
        v2->setSpinnable(editable);
        v2->setEditable(editable);
    }

    // Vec 3 input
    else if (value->isA<mx::Vector3>())
    {
        ng::Widget* twoColumns = new ng::Widget(container);
        twoColumns->setLayout(_gridLayout2);

        mx::Vector3 v = value->asA<mx::Vector3>();
        new ng::Label(twoColumns, label + ".x");
        auto v1 = new ng::FloatBox<float>(twoColumns, v[0]);
        v1->setFixedSize({ 100, 20 });
        v1->setFontSize(15);
        new ng::Label(twoColumns, label + ".y");
        auto v2 = new ng::FloatBox<float>(twoColumns, v[1]);
        v2->setFixedSize({ 100, 20 });
        v2->setFontSize(15);
        new ng::Label(twoColumns, label + ".z");
        auto v3 = new ng::FloatBox<float>(twoColumns, v[2]);
        v3->setFixedSize({ 100, 20 });
        v3->setFontSize(15);

        v1->setCallback([v2, v3, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            if (material)
            {
                mx::Vector3 v;
                v[0] = f;
                v[1] = v2->value();
                v[2] = v3->value();
                material->modifyUniform(path, mx::Value::createValue(v));
            }
        });
        v1->setSpinnable(editable);
        v1->setEditable(editable);
        v2->setCallback([v1, v3, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            if (material)
            {
                mx::Vector3 v;
                v[0] = v1->value();
                v[1] = f;
                v[2] = v3->value();
                material->modifyUniform(path, mx::Value::createValue(v));
            }
        });
        v2->setSpinnable(editable);
        v2->setEditable(editable);
        v3->setCallback([v1, v2, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            if (material)
            {
                mx::Vector3 v;
                v[0] = v1->value();
                v[1] = v2->value();
                v[2] = f;
                material->modifyUniform(path, mx::Value::createValue(v));
            }
        });
        v3->setSpinnable(editable);
        v3->setEditable(editable);
    }

    // Vec 4 input
    else if (value->isA<mx::Vector4>())
    {
        ng::Widget* twoColumns = new ng::Widget(container);
        twoColumns->setLayout(_gridLayout2);

        mx::Vector4 v = value->asA<mx::Vector4>();
        new ng::Label(twoColumns, label + ".x");
        auto v1 = new ng::FloatBox<float>(twoColumns, v[0]);
        v1->setFixedSize({ 100, 20 });
        v1->setFontSize(15);
        new ng::Label(twoColumns, label + ".y");
        auto v2 = new ng::FloatBox<float>(twoColumns, v[1]);
        v2->setFixedSize({ 100, 20 });
        v1->setFontSize(15);
        new ng::Label(twoColumns, label + ".z");
        auto v3 = new ng::FloatBox<float>(twoColumns, v[2]);
        v3->setFixedSize({ 100, 20 });
        v1->setFontSize(15);
        new ng::Label(twoColumns, label + ".w");
        auto v4 = new ng::FloatBox<float>(twoColumns, v[3]);
        v4->setFixedSize({ 100, 20 });
        v1->setFontSize(15);

        v1->setCallback([v2, v3, v4, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            if (material)
            {
                mx::Vector4 v;
                v[0] = f;
                v[1] = v2->value();
                v[2] = v3->value();
                v[3] = v4->value();
                material->modifyUniform(path, mx::Value::createValue(v));
            }
        });
        v1->setSpinnable(editable);
        v2->setCallback([v1, v3, v4, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            if (material)
            {
                mx::Vector4 v;
                v[0] = v1->value();
                v[1] = f;
                v[2] = v3->value();
                v[3] = v4->value();
                material->modifyUniform(path, mx::Value::createValue(v));
            }
        });
        v2->setSpinnable(editable);
        v2->setEditable(editable);
        v3->setCallback([v1, v2, v4, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            if (material)
            {
                mx::Vector4 v;
                v[0] = v1->value();
                v[1] = v2->value();
                v[2] = f;
                v[3] = v4->value();
                material->modifyUniform(path, mx::Value::createValue(v));
            }
        });
        v3->setSpinnable(editable);
        v3->setEditable(editable);
        v4->setCallback([v1, v2, v3, path, viewer](float f)
        {
            MaterialPtr material = viewer->getSelectedMaterial();
            if (material)
            {
                mx::Vector4 v;
                v[0] = v1->value();
                v[1] = v2->value();
                v[2] = v3->value();
                v[3] = f;
                material->modifyUniform(path, mx::Value::createValue(v));
            }
        });
        v4->setSpinnable(editable);
        v4->setEditable(editable);
    }

    // String
    else if (value->isA<std::string>())
    {
        std::string v = value->asA<std::string>();
        if (!v.empty())
        {
            ng::Widget* twoColumns = new ng::Widget(container);
            twoColumns->setLayout(_gridLayout2);

            if (item.variable->getType() == mx::Type::FILENAME)
            {
                new ng::Label(twoColumns, label);
                ng::Button* buttonVar = new ng::Button(twoColumns, mx::FilePath(v).getBaseName());
                buttonVar->setEnabled(editable);
                buttonVar->setFontSize(15);
                buttonVar->setCallback([buttonVar, path, viewer]()
                {
                    MaterialPtr material = viewer->getSelectedMaterial();
                    mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
                    if (uniform)
                    {
                        if (uniform->getType() == mx::Type::FILENAME)
                        {
                            mx::ImageHandlerPtr handler = viewer->getImageHandler();
                            if (handler)
                            {
                                mx::StringSet extensions = handler->supportedExtensions();
                                std::vector<std::pair<std::string, std::string>> filetypes;
                                for (const auto& extension : extensions)
                                {
                                    filetypes.emplace_back(extension, extension);
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
                new ng::Label(twoColumns, label);
                ng::TextBox* stringVar =  new ng::TextBox(twoColumns, v);
                stringVar->setFixedSize({ 100, 20 });
                stringVar->setFontSize(15);
                stringVar->setCallback([path, viewer](const std::string &v)
                {
                    MaterialPtr material = viewer->getSelectedMaterial();
                    mx::ShaderPort* uniform = material ? material->findUniform(path) : nullptr;
                    if (uniform)
                    {
                        uniform->setValue(mx::Value::createValue<std::string>(v));
                    }
                    return true;
                });
            }
        }
    }
}

void PropertyEditor::updateContents(Viewer* viewer)
{
    create(*viewer);

    MaterialPtr material = viewer->getSelectedMaterial();
    if (!material)
    {
        return;
    }

    // Shading model display
    mx::TypedElementPtr elem = material->getElement();
    std::string shaderName = elem ? elem->getAttribute(mx::NodeDef::NODE_ATTRIBUTE) : mx::EMPTY_STRING;
    if (!shaderName.empty())
    {
        ng::Widget* twoColumns = new ng::Widget(_container);
        twoColumns->setLayout(_gridLayout2);
        ng::Label* modelLabel = new ng::Label(twoColumns, "Shading Model");
        modelLabel->setFontSize(20);
        modelLabel->setFont("sans-bold");
        ng::Label* nameLabel = new ng::Label(twoColumns, shaderName);
        nameLabel->setFontSize(20);
    }

    bool addedItems = false;
    const mx::VariableBlock* publicUniforms = material->getPublicUniforms();
    if (publicUniforms)
    {
        mx::UIPropertyGroup groups;
        mx::UIPropertyGroup unnamedGroups;
        const std::string pathSeparator(":");
        mx::createUIPropertyGroups(*publicUniforms, material->getDocument(), elem,
                                    pathSeparator, groups, unnamedGroups, false); 

        std::string previousFolder;
        // Make all inputs editable for now. Could make this read-only as well.
        const bool editable = true;
        for (auto it = groups.begin(); it != groups.end(); ++it)
        {
            const std::string& folder = it->first;
            const mx::UIPropertyItem& item = it->second;

            // Find out if the uniform is editable. Some
            // inputs may be optimized out during compilation.
            if (material->findUniform(item.variable->getPath()))
            {
                addItemToForm(item, (previousFolder == folder) ? mx::EMPTY_STRING : folder, _container, viewer, editable);
                previousFolder.assign(folder);
                addedItems = true;
            }
        }

        bool addedLabel = false;
        const std::string otherString("Other");
        for (auto it2 = unnamedGroups.begin(); it2 != unnamedGroups.end(); ++it2)
        {
            const mx::UIPropertyItem& item = it2->second;
            if (material->findUniform(item.variable->getPath()))
            {
                addItemToForm(item, addedLabel ? mx::EMPTY_STRING : otherString, _container, viewer, editable);
                addedLabel = true;
                addedItems = true;
            }
        }
    }
    if (!addedItems)
    {
        new ng::Label(_container, "No Input Parameters");
        new ng::Label(_container, "");
    }

    viewer->performLayout();
}

ng::FloatBox<float>* createFloatWidget(ng::Widget* parent, const std::string& label, float value,
                                       const mx::UIProperties* ui, std::function<void(float)> callback)
{
    new ng::Label(parent, label);

    ng::Slider *slider = new ng::Slider(parent);
    slider->setValue(value);

    ng::FloatBox<float>* box = new ng::FloatBox<float>(parent, value);
    box->setFixedWidth(60);
    box->setFontSize(15);
    box->setAlignment(ng::TextBox::Alignment::Right);

    if (ui)
    {
        std::pair<float, float> range(0.0f, 0.0f);
        if (ui->uiMin)
        {
            box->setMinValue(ui->uiMin->asA<float>());
            range.first = ui->uiMin->asA<float>();
        }
        if (ui->uiMax)
        {
            box->setMaxValue(ui->uiMax->asA<float>());
            range.second = ui->uiMax->asA<float>();
        }
        if (ui->uiSoftMin)
        {
            range.first = ui->uiSoftMin->asA<float>();
        }
        if (ui->uiSoftMax)
        {
            range.second = ui->uiSoftMax->asA<float>();
        }
        if (range.first != range.second)
        {
            slider->setRange(range);
        }
        if (ui->uiStep)
        {
            box->setValueIncrement(ui->uiStep->asA<float>());
            box->setSpinnable(true);
            box->setEditable(true);
        }
    }

    slider->setCallback([box, callback](float value) 
    {
        box->setValue(value);
        callback(value);
    });
    box->setCallback([slider, callback](float value)
    {
        slider->setValue(value);
        callback(value);
    });

    return box;
}

ng::IntBox<int>* createIntWidget(ng::Widget* parent, const std::string& label, unsigned int value,
    const mx::UIProperties* ui, std::function<void(int)> callback)
{
    new ng::Label(parent, label);

    ng::Slider *slider = new ng::Slider(parent);
    slider->setValue((float)value);

    ng::IntBox<int>* box = new ng::IntBox<int>(parent, value);
    box->setFixedWidth(60);
    box->setFontSize(15);
    box->setAlignment(ng::TextBox::Alignment::Right);
    if (ui)
    {
        std::pair<int, int> range(0, 1);
        if (ui->uiMin)
        {
            box->setMinValue(ui->uiMin->asA<int>());
            range.first = ui->uiMin->asA<int>();
        }
        if (ui->uiMax)
        {
            box->setMaxValue(ui->uiMax->asA<int>());
            range.second = ui->uiMax->asA<int>();
        }
        if (ui->uiSoftMin)
        {
            range.first = ui->uiSoftMin->asA<int>();
        }
        if (ui->uiSoftMax)
        {
            range.second = ui->uiSoftMax->asA<int>();
        }
        if (range.first != range.second)
        {
            std::pair<float, float> float_range((float)range.first, (float)range.second);
            slider->setRange(float_range);
        }
        if (ui->uiStep)
        {
            box->setValueIncrement(ui->uiStep->asA<int>());
            box->setSpinnable(true);
            box->setEditable(true);
        }
    }

    slider->setCallback([box, callback](float value)
    {
        box->setValue((int)value);
        callback((int)value);
    });
    box->setCallback([slider, callback](int value)
    {
        slider->setValue((float)value);
        callback(value);
    });

    return box;
}
