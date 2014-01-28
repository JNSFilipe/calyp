/*    This file is a part of plaYUVer project
 *    Copyright (C) 2014  by plaYUVer developers
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * \file     ConfigureResolutionDialog.h
 * \brief    Dialog box to set the sequence resolution
 */

#ifndef __CONFIGURERESOLUTIONDIALOG_H__
#define __CONFIGURERESOLUTIONDIALOG_H__

#include <QtCore>
#include <QWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include "TypeDef.h"

class QPixmap;
class QColor;

namespace plaYUVer
{

#define MAX_STANDARD_RESOLUTIONSIZES 10
/**
 *
 */
class ConfigureResolutionDialog: public QDialog
{
Q_OBJECT

public:

  ConfigureResolutionDialog( QWidget *parent = 0 );

  QSize getResolution()
  {
    return QSize( widthSpinBox->value(), heightSpinBox->value() );
  }

private slots:
  void StandardResolutionSelection();

private:
  QStringList standardResolutionNames;
  QList<QSize> standardResolutionSizesList;
  QSize  standardResolutionSizes[MAX_STANDARD_RESOLUTIONSIZES];
  QVBoxLayout *MainLayout;
  QHBoxLayout *headLayout;
  QLabel *dialogTitleLabel;
  QSpacerItem *headerSpacer;
  QLabel *dialogIconLabel;
  QHBoxLayout *standardResolutionLayout;
  QLabel *standardResolutionLabel;
  QSpacerItem *horizontalSpacer;
  QComboBox *standardResolutionBox;
  QGridLayout *resolutionGrid;
  QLabel *widthLabel;
  QLabel *pixelsLabel;
  QSpinBox *widthSpinBox;
  QLabel *resolutionLabel;
  QSpinBox *heightSpinBox;
  QLabel *heightLabel;
  QSpacerItem *verticalSpacer;
  QDialogButtonBox *dialogButtonOkCancel;

  Void setStandardResolutionSizes()
  {

    standardResolutionNames.push_back( QString( "VGA (640x480)" ) );
    standardResolutionSizesList.push_back( QSize( 640, 480 ) );

    standardResolutionNames.push_back( QString( "HD (1024x768)" ) );
    standardResolutionSizesList.push_back( QSize( 1024, 768 ) );

    standardResolutionNames.push_back( QString( "Full HD (1920x1080)" ) );
    standardResolutionSizesList.push_back( QSize( 1920, 1080 ) );

  }

};

} // NAMESPACE

#endif // __CONFIGURERESOLUTIONDIALOG_H__
