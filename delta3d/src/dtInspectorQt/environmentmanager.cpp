#include <dtInspectorQt/environmentmanager.h>
#include "ui_dtinspectorqt.h"
#include <QtGui/QColorDialog>

//////////////////////////////////////////////////////////////////////////
dtInspectorQt::EnvironmentManager::EnvironmentManager(Ui::InspectorWidget& ui)
:mUI(&ui)
{
   connect(mUI->envFogToggle, SIGNAL(stateChanged(int)), this, SLOT(OnFogToggled(int)));
   connect(mUI->envFogVisibilityEdit, SIGNAL(valueChanged(double)), this, SLOT(OnVisibilityChanged(double)));
   connect(mUI->envFogRedEdit, SIGNAL(valueChanged(double)), this, SLOT(OnFogColorChanged(double)));
   connect(mUI->envFogGreenEdit, SIGNAL(valueChanged(double)), this, SLOT(OnFogColorChanged(double)));
   connect(mUI->envFogBlueEdit, SIGNAL(valueChanged(double)), this, SLOT(OnFogColorChanged(double)));
   connect(mUI->envFogColorButton, SIGNAL(clicked()), this, SLOT(OnFogColorPickerClicked()));
   connect(mUI->envSkyRedEdit, SIGNAL(valueChanged(double)), this, SLOT(OnSkyColorChanged(double)));
   connect(mUI->envSkyGreenEdit, SIGNAL(valueChanged(double)), this, SLOT(OnSkyColorChanged(double)));
   connect(mUI->envSkyBlueEdit, SIGNAL(valueChanged(double)), this, SLOT(OnSkyColorChanged(double)));
   connect(mUI->envSkyColorButton, SIGNAL(clicked()), this, SLOT(OnSkyColorPickerClicked()));
   connect(mUI->envDateTimeEdit, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(OnDateTimeChanged(QDateTime)));
   connect(mUI->envRefLatEdit, SIGNAL(valueChanged(double)), this, SLOT(OnRefPositionChanged(double)));
   connect(mUI->envRefLongEdit, SIGNAL(valueChanged(double)), this, SLOT(OnRefPositionChanged(double)));
}

//////////////////////////////////////////////////////////////////////////
dtInspectorQt::EnvironmentManager::~EnvironmentManager()
{
}

//////////////////////////////////////////////////////////////////////////
void dtInspectorQt::EnvironmentManager::OperateOn(dtCore::Base* b)
{
   dtCore::Environment *environment = dynamic_cast<dtCore::Environment*>(b);

   mOperateOn = environment;
   Update();
}

//////////////////////////////////////////////////////////////////////////
void dtInspectorQt::EnvironmentManager::OnFogToggled(int checked)
{
   if (mOperateOn.valid())
   {
      mOperateOn->SetFogEnable(checked ? true : false);
   }
}

//////////////////////////////////////////////////////////////////////////
void dtInspectorQt::EnvironmentManager::OnVisibilityChanged(double newValue)
{
   if (mOperateOn.valid())
   {
      mOperateOn->SetVisibility(newValue);
   }
}

//////////////////////////////////////////////////////////////////////////
void dtInspectorQt::EnvironmentManager::OnFogColorChanged(double newValue)
{
   if (mOperateOn.valid())
   {
      osg::Vec3 color(mUI->envFogRedEdit->value(),
         mUI->envFogGreenEdit->value(), mUI->envFogBlueEdit->value());
      mOperateOn->SetFogColor(color);
   }
}

//////////////////////////////////////////////////////////////////////////
void dtInspectorQt::EnvironmentManager::OnFogColorPickerClicked()
{
   QColor initialColor(mUI->envFogRedEdit->value() * 255,
      mUI->envFogGreenEdit->value() * 255, mUI->envFogBlueEdit->value() * 255);
   QColor newColor = QColorDialog::getColor(initialColor, mUI->envFogColorButton);
   if (newColor.isValid())
   {
      mUI->envFogRedEdit->setValue(newColor.redF());
      mUI->envFogGreenEdit->setValue(newColor.greenF());
      mUI->envFogBlueEdit->setValue(newColor.blueF());
   }
}

//////////////////////////////////////////////////////////////////////////
void dtInspectorQt::EnvironmentManager::OnSkyColorChanged(double newValue)
{
   if (mOperateOn.valid())
   {
      osg::Vec3 color(mUI->envSkyRedEdit->value(),
         mUI->envSkyGreenEdit->value(), mUI->envSkyBlueEdit->value());
      mOperateOn->SetSkyColor(color);
   }
}

//////////////////////////////////////////////////////////////////////////
void dtInspectorQt::EnvironmentManager::OnSkyColorPickerClicked()
{
   QColor initialColor(mUI->envSkyRedEdit->value() * 255,
      mUI->envSkyGreenEdit->value() * 255, mUI->envSkyBlueEdit->value() * 255);
   QColor newColor = QColorDialog::getColor(initialColor, mUI->envSkyColorButton);
   if (newColor.isValid())
   {
      mUI->envSkyRedEdit->setValue(newColor.redF());
      mUI->envSkyGreenEdit->setValue(newColor.greenF());
      mUI->envSkyBlueEdit->setValue(newColor.blueF());
   }
}

//////////////////////////////////////////////////////////////////////////
void dtInspectorQt::EnvironmentManager::OnDateTimeChanged(QDateTime newDateTime)
{
   if (mOperateOn.valid())
   {
      QDate newDate = newDateTime.date();
      QTime newTime = newDateTime.time();
      mOperateOn->SetDateTime(newDate.year(), newDate.month(), newDate.day(),
         newTime.hour(), newTime.minute(), newTime.second());
   }
}

//////////////////////////////////////////////////////////////////////////
void dtInspectorQt::EnvironmentManager::OnRefPositionChanged(double newValue)
{
   if (mOperateOn.valid())
   {
      osg::Vec2 refPos;
      refPos[0] = mUI->envRefLatEdit->value();
      refPos[1] = mUI->envRefLongEdit->value();
      mOperateOn->SetRefLatLong(refPos);
   }
}

//////////////////////////////////////////////////////////////////////////
void dtInspectorQt::EnvironmentManager::Update()
{
   if (mOperateOn.valid())
   {
      mUI->envGroupBox->show();

      mUI->envFogToggle->setChecked(mOperateOn->GetFogEnable());
      mUI->envFogVisibilityEdit->setValue(mOperateOn->GetVisibility());

      osg::Vec3 fColor;
      mOperateOn->GetFogColor(fColor);
      mUI->envFogRedEdit->setValue(fColor[0]);
      mUI->envFogGreenEdit->setValue(fColor[1]);
      mUI->envFogBlueEdit->setValue(fColor[2]);

      osg::Vec3 sColor;
      mOperateOn->GetSkyColor(sColor);
      mUI->envSkyRedEdit->setValue(sColor[0]);
      mUI->envSkyGreenEdit->setValue(sColor[1]);
      mUI->envSkyBlueEdit->setValue(sColor[2]);

      unsigned yr, mo, da, hr, mi, sec;
      mOperateOn->GetDateTime(yr, mo, da, hr, mi, sec);
      mUI->envDateTimeEdit->setDateTime(QDateTime(QDate(yr, mo, da), QTime(hr, mi, sec)));

      osg::Vec2 refPos;
      mOperateOn->GetRefLatLong(refPos);
      mUI->envRefLatEdit->setValue(refPos[0]);
      mUI->envRefLongEdit->setValue(refPos[1]);
   }
   else
   {
      mUI->envGroupBox->hide();
   }
}

//////////////////////////////////////////////////////////////////////////

