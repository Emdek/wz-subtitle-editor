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

#ifndef SUBTITLESEDITOR_H
#define SUBTITLESEDITOR_H

#include <QtCore/QTime>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimediaWidgets/QGraphicsVideoItem>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QGraphicsTextItem>

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
	MainWindow(QWidget *parent = NULL);
	~MainWindow();

protected:
	void changeEvent(QEvent *event);
	void closeEvent(QCloseEvent *event);
	QString timeToString(qint64 time, bool readable = false);
	bool openFile(const QString &fileName);
	bool openMovie(const QString &filename);
	bool openSubtitles(const QString &fileName, int index);
	bool saveSubtitles(const QString &fileName);
	bool eventFilter(QObject *object, QEvent *event);

protected slots:
    void actionOpen(QString fileName = QString());
	void actionOpenRecent(QAction *action);
	void actionClearRecentFiles();
	void actionSave();
	void actionSaveAs();
	void actionAboutApplication();
	void errorOccured(QMediaPlayer::Error error);
	void stateChanged(QMediaPlayer::State state);
	void durationChanged(qint64 duration);
	void positionChanged(qint64 position);
	void playPause();
	void seek(int position);
	void selectTrack(int track);
	void addSubtitle();
	void removeSubtitle();
	void previousSubtitle();
	void nextSubtitle();
	void selectSubtitle();
	void updateSubtitle();
	void rescaleSubtitles();
	void updateAudio();
	void updateVideo();
	void updateActions();
	void updateRecentFilesMenu();

private:
	Ui::MainWindow *m_ui;
	QMediaPlayer *m_mediaPlayer;
	QGraphicsVideoItem *m_videoWidget;
	QGraphicsTextItem *m_subtitlesTopWidget;
	QGraphicsTextItem *m_subtitlesBottomWidget;
	QString m_currentPath;
	QList<QList<Subtitle> > m_subtitles;
	int m_currentSubtitle;
	int m_currentTrack;

signals:
	void timeChanged(QString time);
	void fileChanged(QString file);

};

#endif
