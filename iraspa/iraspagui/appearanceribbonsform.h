/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#pragma once

#include <QWidget>
#include "ui_appearanceribbonsform.h"

class AppearanceRibbonsForm : public QWidget, public Ui::AppearanceRibbonsForm
{
  Q_OBJECT

public:
  explicit AppearanceRibbonsForm(QWidget *parent = nullptr);
  ~AppearanceRibbonsForm();
};
