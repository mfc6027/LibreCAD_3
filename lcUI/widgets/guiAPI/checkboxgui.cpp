#include "checkboxgui.h"

#include <QVBoxLayout>

using namespace lc::ui::api;

CheckBoxGUI::CheckBoxGUI(std::string label, bool checked, QWidget* parent)
    :
    InputGUI(label, parent)
{
    _checkBox = new QCheckBox(QString(label.c_str()));
    this->setLayout(new QVBoxLayout());
    this->layout()->addWidget(_checkBox);
    _checkBox->setChecked(checked);
    connect(_checkBox, &QCheckBox::stateChanged, this, &CheckBoxGUI::callbackCalled);
}

void CheckBoxGUI::setLabel(const std::string& newLabel) {
    InputGUI::setLabel(newLabel);
    _checkBox->setText(QString(newLabel.c_str()));
}

void CheckBoxGUI::addCallback(kaguya::LuaRef cb) {
    _callbacks.push_back(cb);
}

void CheckBoxGUI::callbackCalled(int state) {
    for (kaguya::LuaRef& cb : _callbacks) {
        cb(state == Qt::Checked);
    }
}

void CheckBoxGUI::getLuaValue(kaguya::LuaRef& table) {
    table[_key] = _checkBox->isChecked();
}

bool CheckBoxGUI::value() const {
    return _checkBox->isChecked();
}

void CheckBoxGUI::setValue(bool check) {
    _checkBox->setChecked(check);
}
