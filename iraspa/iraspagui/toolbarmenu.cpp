#include "toolbarmenu.h"
#include <QDebug>

ToolbarMenu::ToolbarMenu(QWidget *parent): QMenu(parent)
{

}

// avoid that pressing 'ALT' closes the toolbar menu
void ToolbarMenu::keyPressEvent(QKeyEvent *event)
{
  Q_UNUSED(event);
}

// avoid that releasing 'ALT' closes the toolbar menu
void ToolbarMenu::keyReleaseEvent(QKeyEvent *event)
{
  Q_UNUSED(event);
}
