#pragma once

#include <QObject>
#include <QRadioButton>

#include <kaguya/kaguya.hpp>

namespace lc {
    namespace ui {
        namespace api {
            /**
            * \brief CheckBox GUI Widget
            */
            class RadioButtonGUI : public QRadioButton
            {
                Q_OBJECT

            public:
                /**
                * \brief RadioButton GUI Constructor
                * \param string button label
                * \param bool checked
                * \param parent qwidget parent
                */
                RadioButtonGUI(std::string label, QWidget* parent = nullptr);

                /**
                * \brief Return label of the radiobutton
                * \return string label
                */
                std::string label() const;

                /**
                * \brief Set label of radiobutton
                * \param string new label
                */
                void setLabel(const std::string& newLabel);

                /**
                * \brief Add callback on state changed
                * \param LuaRef lua callback
                */
                void addCallback(kaguya::LuaRef cb);

            public slots:
                /**
                * \brief Run all callbacks
                */
                void callbackCalled(bool toggled);

            private:
                std::string _label;
                std::vector<kaguya::LuaRef> _callbacks;
            };
        }
    }
}
