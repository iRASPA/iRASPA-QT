#ifndef TOOLBARMENU_H
#define TOOLBARMENU_H

#include <QMenu>
#include <QCloseEvent>

class ToolbarMenu : public QMenu
{
public:
  ToolbarMenu(QWidget* parent = nullptr);
protected:
  void keyPressEvent(QKeyEvent *event) override final;
  void keyReleaseEvent(QKeyEvent *event) override final;
};

#endif // TOOLBARMENU_H
