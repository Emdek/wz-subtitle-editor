/***********************************************************************************
* Warzone 2100 Subtitles Editor
* Copyright (C) 2010 - 2013 Michal Dutkiewicz aka Emdek <emdeck@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
***********************************************************************************/

#include "SubtitlesEditor.h"

#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	application.setApplicationName("WZSubtitlesEditor");
	application.setApplicationVersion("1.1");
	application.setOrganizationName("Warzone2100");
	application.setOrganizationDomain("wz2100.net");

	MainWindow window;
	window.show();

	return application.exec();
}
