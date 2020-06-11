#include "menu.h"
#include <QObject>
#include <mainwindow.h>

using namespace lc::ui;
using namespace lc::ui::api;

Menu::Menu(const char* menuName, QWidget* parent)
    :
    QMenu(QString(menuName), parent),
    _connected(false),
    _position(-1),
    insideMenu(false)
{
    this->setObjectName(menuName);
}

Menu::Menu(QMenuBar* menuBar)
    :
    QMenu(menuBar),
    _connected(false),
    _position(-1),
    insideMenu(false)
{
}

Menu::Menu(QMenu* menu)
    :
    QMenu(menu),
    _connected(false),
    _position(-1),
    insideMenu(false)
{
}

void Menu::addItem(MenuItem* item) {
    if (checkForItemOfSameLabel(item->label().c_str(), false)) {
        return;
    }

    this->addAction(item);

    lc::ui::api::Menu* menuWidget = this;
    while (menuWidget->insideMenu)
    {
        QList<QWidget*> widgets = menuWidget->menuAction()->associatedWidgets();
        menuWidget = qobject_cast<lc::ui::api::Menu*>(widgets[0]);
    }

    QMenuBar* menuBar = qobject_cast<QMenuBar*>(menuWidget->menuAction()->associatedWidgets()[0]);
    lc::ui::MainWindow* mainWindow = qobject_cast<lc::ui::MainWindow*>(menuBar->parentWidget());

    QObject::connect(item, SIGNAL(connectToCallback(lc::ui::api::MenuItem*, const std::string&, kaguya::LuaRef&)), mainWindow, SLOT(connectToCallbackMenu(lc::ui::api::MenuItem*, const std::string&, kaguya::LuaRef&)));
    QObject::connect(item, SIGNAL(disconnectCallback(lc::ui::api::MenuItem*, const std::string&, kaguya::LuaRef&)), mainWindow, SLOT(disconnectCallbackMenu(lc::ui::api::MenuItem*, const std::string&, kaguya::LuaRef&)));

    if (_connected) {
        item->enableConnections();
    }
}

MenuItem* Menu::addItem(const char* menuItemLabel) {
    MenuItem* newItem = new MenuItem(menuItemLabel);
    addItem(newItem);

    return newItem;
}

MenuItem* Menu::addItem(const char* menuItemLabel, kaguya::LuaRef callback) {
    MenuItem* newItem = new MenuItem(menuItemLabel, callback);
    addItem(newItem);

    return newItem;
}

void Menu::addMenu(Menu* newMenu) {
    if (checkForItemOfSameLabel(newMenu->label().c_str(), true)) {
        return;
    }

    this->addAction(newMenu->menuAction());
    newMenu->insideMenu = true;

    if (_connected) {
        newMenu->enableConnections();
    }
}

Menu* Menu::addMenu(const char* menuLabel) {
    Menu* newMenu = new Menu(menuLabel);
    addMenu(newMenu);
    return newMenu;
}

std::string Menu::label() const {
    return this->title().toStdString();
}

void Menu::setLabel(const char* newMenuLabel) {
    this->setTitle(newMenuLabel);
}

MenuItem* Menu::itemByName(const char* menuItemLabel) {
    QList<QAction*> menuActions = this->actions();

    for (QAction* action : menuActions)
    {
        if (action->menu()) {
            continue;
        }
        MenuItem* item = qobject_cast<MenuItem*>(action);

        if (item->label() == std::string(menuItemLabel)) {
            return item;
        }
    }

    return nullptr;
}

MenuItem* Menu::itemByPosition(int pos) {
    QList<QAction*> menuActions = this->actions();

    if (pos < 0 || pos >= menuActions.size()) {
        return nullptr;
    }

    if (menuActions[pos]->menu()) {
        return nullptr;
    }

    return qobject_cast<MenuItem*>(menuActions[pos]);
}

Menu* Menu::menuByName(const char* menuLabel) {
    QList<QAction*> menuActions = this->actions();

    for (QAction* action : menuActions)
    {
        if (action->menu()) {
            Menu* item = qobject_cast<Menu*>(action->menu());

            if (item->label() == std::string(menuLabel)) {
                return item;
            }
        }
    }

    return nullptr;
}

Menu* Menu::menuByPosition(int pos) {
    QList<QAction*> menuActions = this->actions();

    if (pos < 0 || pos >= menuActions.size()) {
        return nullptr;
    }

    if (!menuActions[pos]->menu()) {
        return nullptr;
    }

    return qobject_cast<Menu*>(menuActions[pos]->menu());
}

void Menu::hide() {
    menuAction()->setVisible(false);
}

void Menu::show() {
    menuAction()->setVisible(true);
}

void Menu::removeItem(const char* menuItemLabel) {
    itemByName(menuItemLabel)->remove();
}

void Menu::removeItem(MenuItem* item) {
    item->remove();
}

void Menu::removeMenu(const char* menuLabel) {
    menuByName(menuLabel)->remove();
}

void Menu::removeMenu(Menu* menu) {
    menu->remove();
}

void Menu::enableConnections(bool setCallbacks) {
    if (_connected) {
        return;
    }

    _connected = true;

    // set connections of all child menu items
    QList<QAction*> menuActions = this->actions();

    for (QAction* action : menuActions)
    {
        if (action->menu()) {
            Menu* item = qobject_cast<Menu*>(action->menu());
            if (item != nullptr) {
                item->enableConnections(setCallbacks);
            }
            continue;
        }

        MenuItem* item = qobject_cast<MenuItem*>(action);
        if (item != nullptr) {
            item->enableConnections(setCallbacks);
        }
    }

    /*
     * callbacks is connected when the menu gets added, so determine
     * the position of the menu
    */
    QList<QWidget*> widgets = this->menuAction()->associatedWidgets();

    if (insideMenu) {
        QMenu* menu = qobject_cast<QMenu*>(widgets[0]);
        if (menu != nullptr) {
            QList<QAction*> menuList = menu->actions();
            _position = menuList.size() - 1;
        }
    }
    else {
        QMenuBar* menuBar = qobject_cast<QMenuBar*>(widgets[0]);
        if (menuBar != nullptr) {
            QList<QAction*> menuList = menuBar->actions();
            _position = menuList.size() - 1;
        }
    }
}

int Menu::position() const {
    return _position;
}

void Menu::setPosition(int newPosition) {
    if (newPosition == _position || newPosition < 0) {
        return;
    }

    QList<QWidget*> widgets = this->menuAction()->associatedWidgets();

    if (!insideMenu) {
        QMenuBar* menuBar = qobject_cast<QMenuBar*>(widgets[0]);

        if (menuBar == nullptr) {
            return;
        }

        QList<QAction*> menuList = menuBar->actions();

        if (newPosition >= menuList.size()) {
            newPosition = menuList.size() - 1;
        }

        int setToNewPosition = newPosition;

        if (newPosition > _position) {
            newPosition++;
        }

        menuBar->insertMenu(menuList[newPosition], this);
        _position = setToNewPosition;

        // update position of menus after new position
        menuList = menuBar->actions();
        int n = menuList.size();

        for (int i = _position + 1; i < n; i++)
        {
            Menu* item = qobject_cast<Menu*>(menuList[i]->menu());
            if (item != nullptr) {
                item->updatePositionVariable(i);
            }
        }
    }
    else {
        QMenu* menu = qobject_cast<QMenu*>(widgets[0]);

        if (menu == nullptr) {
            return;
        }

        QList<QAction*> items = menu->actions();

        if (newPosition >= items.size()) {
            newPosition = items.size() - 1;
        }

        int size = items.size();

        for (int i = 0; i < size; i++)
        {
            QAction* item = items[i];
            if (i > _position) {
                if (item->menu()) {
                    Menu* menu_i = qobject_cast<Menu*>(item->menu());
                    if (menu_i != nullptr) {
                        menu_i->updatePositionVariable(menu_i->position() - 1);
                    }
                }
                else {
                    MenuItem* menuItem = qobject_cast<MenuItem*>(item);
                    if (menuItem != nullptr) {
                        menuItem->updatePositionVariable(menuItem->position() - 1);
                    }
                }
            }
        }

        QAction* currentItem = items[_position];
        menu->removeAction(currentItem);

        std::stack<QAction*> actionsStack;
        items = menu->actions();
        while (items.size() != newPosition)
        {
            QAction* item = items[items.size() - 1];
            if (item->menu()) {
                Menu* menu_i = qobject_cast<Menu*>(item->menu());
                if (menu_i != nullptr) {
                    menu_i->updatePositionVariable(items.size());
                }
            }
            else {
                MenuItem* menuItem = qobject_cast<MenuItem*>(item);
                if (menuItem != nullptr) {
                    menuItem->updatePositionVariable(items.size());
                }
            }
            actionsStack.push(item);
            menu->removeAction(item);
            items = menu->actions();
        }

        menu->addAction(currentItem);

        while (actionsStack.size() > 0) {
            menu->addAction(actionsStack.top());
            actionsStack.pop();
        }

        _position = newPosition;
    }
}

void Menu::remove() {
    QList<QWidget*> widgets = this->menuAction()->associatedWidgets();

    updateOtherPositionsAfterRemove();

    if (insideMenu) {
        for (QWidget* widget : widgets) {
            QMenu* parentMenu = qobject_cast<QMenu*>(widget);
            if (parentMenu != nullptr) {
                parentMenu->removeAction(this->menuAction());
            }
        }
    }
    else {
        QMenuBar* menuBar = qobject_cast<QMenuBar*>(widgets[0]);
        if (menuBar != nullptr) {
            lc::ui::MainWindow* mainWindow = qobject_cast<lc::ui::MainWindow*>(menuBar->parentWidget());
            if (mainWindow != nullptr) {
                mainWindow->removeFromMenuMap(this->label());
                menuBar->removeAction(this->menuAction());
            }
        }
    }
}

void Menu::updatePositionVariable(int newPosition) {
    _position = newPosition;
}

bool Menu::checkForItemOfSameLabel(const char* label, bool isMenu) {
    QList<QAction*> menuActions = this->actions();

    for (QAction* action : menuActions)
    {
        if (!isMenu) {
            if (action->menu()) {
                continue;
            }
            MenuItem* item = qobject_cast<MenuItem*>(action);

            if (item != nullptr) {
                if (item->label() == std::string(label)) {
                    return true;
                }
            }
        }
        else {
            if (action->menu()) {
                Menu* item = qobject_cast<Menu*>(action->menu());

                if (item != nullptr) {
                    if (item->label() == std::string(label)) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

void Menu::updateOtherPositionsAfterRemove() {
    QList<QWidget*> widgets = this->menuAction()->associatedWidgets();

    if (insideMenu) {
        for (QWidget* widget : widgets) {
            QMenu* menu = qobject_cast<QMenu*>(widget);

            if (menu == nullptr) {
                return;
            }

            QList<QAction*> actions = menu->actions();

            for (int i = _position + 1; i < actions.size(); i++) {
                QAction* item = actions[i];

                if (item->menu()) {
                    Menu* menu_i = qobject_cast<Menu*>(item->menu());
                    if (menu_i != nullptr) {
                        menu_i->updatePositionVariable(menu_i->position() - 1);
                    }
                }
                else {
                    MenuItem* menuItem = qobject_cast<MenuItem*>(item);
                    if (menuItem != nullptr) {
                        menuItem->updatePositionVariable(menuItem->position() - 1);
                    }
                }
            }
        }
    }
    else {
        QMenuBar* menuBar = qobject_cast<QMenuBar*>(widgets[0]);

        if (menuBar == nullptr) {
            return;
        }

        QList<QAction*> menuList = menuBar->actions();

        for (int i = _position + 1; i < menuList.size(); i++) {
            QAction* item = menuList[i];

            Menu* menu = qobject_cast<Menu*>(item->menu());

            if (menu != nullptr) {
                menu->updatePositionVariable(menu->position() - 1);
            }
        }
    }
}
