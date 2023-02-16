/***

  Olive - Non-Linear Video Editor
  Copyright (C) 2022 Olive Team

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "footageviewer.h"

namespace olive {

#define super ViewerPanelBase

FootageViewerPanel::FootageViewerPanel() :
  super(QStringLiteral("FootageViewerPanel"))
{
  // Set ViewerWidget as the central widget
  FootageViewerWidget* fvw = new FootageViewerWidget(this);
  SetViewerWidget(fvw);

  // Set strings
  Retranslate();

  // Show and raise on connect
  SetShowAndRaiseOnConnect();
}

void FootageViewerPanel::OverrideWorkArea(const TimeRange &r)
{
  GetFootageViewerWidget()->OverrideWorkArea(r);
}

QVector<ViewerOutput *> FootageViewerPanel::GetSelectedFootage() const
{
  QVector<ViewerOutput *> list;

  if (GetConnectedViewer()) {
    list.append(GetConnectedViewer());
  }

  return list;
}

void FootageViewerPanel::Retranslate()
{
  super::Retranslate();

  SetTitle(tr("Footage Viewer"));
}

}
