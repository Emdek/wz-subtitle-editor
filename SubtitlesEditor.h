/***********************************************************************************
* Warzone 2100 Subtitles Editor.
* Copyright (C) 2010 - 2011 Michal Dutkiewicz aka Emdek <emdeck@gmail.com>
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

#ifndef SUBTITLESEDITOR_H
#define SUBTITLESEDITOR_H

#include <QtCore/QTime>

#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QGraphicsProxyWidget>

#include <Phonon/MediaObject>

namespace Ui
{
	class MainWindow;
}

struct Subtitle
{
	QString text;
	QTime begin;
	QTime end;
	QPoint position;
};

class SubtitlesWidget;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void actionOpen();
	void actionOpenRecent(QAction *action);
	void actionClearRecentFiles();
	void actionSave();
	void actionSaveAs();
	void actionAboutApplication();
	void stateChanged(Phonon::State state);
	void finished();
	void tick();
	void selectTrack(int track);
	void addSubtitle();
	void removeSubtitle();
	void previousSubtitle();
	void nextSubtitle();
	void selectSubtitle();
	void updateSubtitle();
	void rescaleSubtitles();
	void playPause();
	void updateVideo();
	void updateActions();
	void updateRecentFilesMenu();

protected:
	void changeEvent(QEvent *event);
	void closeEvent(QCloseEvent *event);
	void openMovie(const QString &filename);
	QString timeToString(qint64 time);
	bool openFile(const QString &fileName);
	bool openSubtitles(const QString &fileName, int index);
	bool saveSubtitles(QString fileName);
	bool eventFilter(QObject *object, QEvent *event);

private:
	Ui::MainWindow *m_ui;
	Phonon::MediaObject *m_mediaObject;
	QGraphicsProxyWidget *m_videoWidget;
	QGraphicsTextItem *m_subtitlesTopWidget;
	QGraphicsTextItem *m_subtitlesBottomWidget;
	QString m_currentPath;
	QLabel *m_fileNameLabel;
	QLabel *m_timeLabel;
	QList<QList<Subtitle> > m_subtitles;
	int m_currentSubtitle;
	int m_currentTrack;

};

#endif // SUBTITLESEDITOR_H
