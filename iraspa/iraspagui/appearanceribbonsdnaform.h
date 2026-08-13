/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#pragma once

#include <QWidget>
#include "ui_appearanceribbonsdnaform.h"

class AppearanceRibbonsDNAForm : public QWidget, public Ui::AppearanceRibbonsDNAForm
{
  Q_OBJECT

public:
  explicit AppearanceRibbonsDNAForm(QWidget *parent = nullptr);
  ~AppearanceRibbonsDNAForm();
};
