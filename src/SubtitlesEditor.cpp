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

#include "ui_SubtitlesEditor.h"

#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTabBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QGraphicsDropShadowEffect>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
	m_ui(new Ui::MainWindow),
	m_mediaPlayer(new QMediaPlayer(this)),
	m_videoWidget(new QGraphicsVideoItem()),
	m_subtitlesTopWidget(new QGraphicsTextItem(m_videoWidget)),
	m_subtitlesBottomWidget(new QGraphicsTextItem(m_videoWidget)),
	m_currentSubtitle(0),
	m_currentTrack(0)
{
	m_ui->setupUi(this);

	m_subtitles.append(QList<Subtitle>());
	m_subtitles.append(QList<Subtitle>());

	m_mediaPlayer->setVolume(QSettings().value("Player/volume", 80).toInt());
	m_mediaPlayer->setVideoOutput(m_videoWidget);
	m_mediaPlayer->setNotifyInterval(100);

	QGraphicsDropShadowEffect *topShadowEffect = new QGraphicsDropShadowEffect(m_subtitlesTopWidget);
	topShadowEffect->setOffset(0, 0);
	topShadowEffect->setBlurRadius(3);
	topShadowEffect->setColor(QColor(Qt::black));

	m_subtitlesTopWidget->setGraphicsEffect(topShadowEffect);
	m_subtitlesTopWidget->setDefaultTextColor(QColor(230, 230, 230));

	QGraphicsDropShadowEffect *bottomShadowEffect = new QGraphicsDropShadowEffect(m_subtitlesBottomWidget);
	bottomShadowEffect->setOffset(0, 0);
	bottomShadowEffect->setBlurRadius(3);
	bottomShadowEffect->setColor(QColor(Qt::black));

	m_subtitlesBottomWidget->setGraphicsEffect(bottomShadowEffect);
	m_subtitlesBottomWidget->setDefaultTextColor(QColor(230, 230, 230));

	m_ui->graphicsView->setScene(new QGraphicsScene(this));
	m_ui->graphicsView->scene()->addItem(m_videoWidget);
	m_ui->graphicsView->installEventFilter(this);

	QTabBar *tabBar = new QTabBar(m_ui->centralWidget);
	tabBar->setDocumentMode(true);
	tabBar->setShape(QTabBar::RoundedWest);
	tabBar->addTab(tr("Top"));
	tabBar->addTab(tr("Bottom"));
	tabBar->setCurrentIndex(1);

	m_ui->tabBarLayout->insertWidget(0, tabBar);

	m_ui->actionPlayPause->setIcon(QIcon::fromTheme("media-playback-start", style()->standardIcon(QStyle::SP_MediaPlay)));
	m_ui->actionPlayPause->setShortcut(tr("Space"));
	m_ui->actionPlayPause->setDisabled(true);

	m_ui->actionStop->setIcon(QIcon::fromTheme("media-playback-stop", style()->standardIcon(QStyle::SP_MediaStop)));
	m_ui->actionStop->setDisabled(true);

	QLabel *timeLabel = new QLabel("00:00.0 / 00:00.0", this);
	QLabel *fileNameLabel = new QLabel(tr("No file loaded"), this);
	fileNameLabel->setMaximumWidth(300);

	m_ui->actionOpen->setIcon(QIcon::fromTheme("document-open", style()->standardIcon(QStyle::SP_DirOpenIcon)));
	m_ui->menuOpenRecent->setIcon(QIcon::fromTheme("document-open-recent"));
	m_ui->actionClearRecentFiles->setIcon(QIcon::fromTheme("edit-clear-list"));
	m_ui->actionSave->setIcon(QIcon::fromTheme("document-save", style()->standardIcon(QStyle::SP_DialogSaveButton)));
	m_ui->actionSaveAs->setIcon(QIcon::fromTheme("document-save-as"));
	m_ui->actionExit->setIcon(QIcon::fromTheme("application-exit", style()->standardIcon(QStyle::SP_DialogCloseButton)));
	m_ui->actionAdd->setIcon(QIcon::fromTheme("list-add"));
	m_ui->actionRemove->setIcon(QIcon::fromTheme("list-remove"));
	m_ui->actionPrevious->setIcon(QIcon::fromTheme("go-previous"));
	m_ui->actionNext->setIcon(QIcon::fromTheme("go-next"));
	m_ui->actionRescale->setIcon(QIcon::fromTheme("chronometer"));
	m_ui->actionAboutApplication->setIcon(QIcon::fromTheme("help-about"));
	m_ui->playPauseButton->setDefaultAction(m_ui->actionPlayPause);
	m_ui->stopButton->setDefaultAction(m_ui->actionStop);
	m_ui->addButton->setDefaultAction(m_ui->actionAdd);
	m_ui->removeButton->setDefaultAction(m_ui->actionRemove);
	m_ui->previousButton->setDefaultAction(m_ui->actionPrevious);
	m_ui->nextButton->setDefaultAction(m_ui->actionNext);
	m_ui->statusBar->addPermanentWidget(fileNameLabel);
	m_ui->statusBar->addPermanentWidget(timeLabel);
	m_ui->volumeSlider->setValue(m_mediaPlayer->volume());

	resize(QSettings().value("Window/size", size()).toSize());
	move(QSettings().value("Window/position", pos()).toPoint());
	restoreState(QSettings().value("Window/state", QByteArray()).toByteArray());
	setWindowTitle(tr("%1 - Unnamed[*]").arg("Subtitles Editor"));
	updateAudio();
	updateVideo();

	connect(this, SIGNAL(fileChanged(QString)), fileNameLabel, SLOT(setText(QString)));
	connect(this, SIGNAL(timeChanged(QString)), timeLabel, SLOT(setText(QString)));
	connect(m_ui->menuFile, SIGNAL(aboutToShow()), this, SLOT(updateRecentFilesMenu()));
	connect(m_ui->actionOpen, SIGNAL(triggered()), this, SLOT(actionOpen()));
	connect(m_ui->menuOpenRecent, SIGNAL(triggered(QAction*)), this, SLOT(actionOpenRecent(QAction*)));
	connect(m_ui->actionClearRecentFiles, SIGNAL(triggered()), this, SLOT(actionClearRecentFiles()));
	connect(m_ui->actionSave, SIGNAL(triggered()), this, SLOT(actionSave()));
	connect(m_ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(actionSaveAs()));
	connect(m_ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(m_ui->actionAdd, SIGNAL(triggered()), this, SLOT(addSubtitle()));
	connect(m_ui->actionRemove, SIGNAL(triggered()), this, SLOT(removeSubtitle()));
	connect(m_ui->actionPrevious, SIGNAL(triggered()), this, SLOT(previousSubtitle()));
	connect(m_ui->actionNext, SIGNAL(triggered()), this, SLOT(nextSubtitle()));
	connect(m_ui->actionRescale, SIGNAL(triggered()), this, SLOT(rescaleSubtitles()));
	connect(m_ui->actionPlayPause, SIGNAL(triggered()), this, SLOT(playPause()));
	connect(m_ui->actionStop, SIGNAL(triggered()), m_mediaPlayer, SLOT(stop()));
	connect(m_ui->actionAboutQt, SIGNAL(triggered()), QApplication::instance(), SLOT(aboutQt()));
	connect(m_ui->actionAboutApplication, SIGNAL(triggered()), this, SLOT(actionAboutApplication()));
	connect(m_ui->seekSlider, SIGNAL(sliderMoved(int)), this, SLOT(seek(int)));
	connect(m_ui->volumeSlider, SIGNAL(sliderMoved(int)), m_mediaPlayer, SLOT(setVolume(int)));
	connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(selectTrack(int)));
	connect(m_mediaPlayer, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(errorOccured(QMediaPlayer::Error)));
	connect(m_mediaPlayer, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChanged(QMediaPlayer::State)));
	connect(m_mediaPlayer, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));
	connect(m_mediaPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
	connect(m_mediaPlayer, SIGNAL(volumeChanged(int)), this, SLOT(updateAudio()));
}

MainWindow::~MainWindow()
{
	delete m_ui;
}

void MainWindow::changeEvent(QEvent *event)
{
	QMainWindow::changeEvent(event);

	switch (event->type())
	{
		case QEvent::LanguageChange:
			m_ui->retranslateUi(this);

			break;
		default:
			break;
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (isWindowModified() && QMessageBox::warning(this, tr("Question"), tr("Do you really want to close current subtitles without saving?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
	{
		event->ignore();

		return;
	}

	QSettings settings;
	settings.setValue("Window/size", size());
	settings.setValue("Window/position", pos());
	settings.setValue("Window/state", saveState());
	settings.setValue("Player/volume", m_mediaPlayer->volume());

	event->accept();
}

void MainWindow::actionOpen(QString fileName)
{
	if (isWindowModified() && QMessageBox::warning(this, tr("Question"), tr("Do you really want to close current subtitles without saving?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
	{
		return;
	}

	if (fileName.isEmpty())
	{
		fileName = QFileDialog::getOpenFileName(this, tr("Open Video or Subtitle file"), QSettings().value("lastUsedDir", QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first()).toString(), tr("Video and subtitle files (*.txt *.txa *.og?)"));
	}

	if (fileName.isEmpty())
	{
		return;
	}

	if (!openFile(fileName))
	{
		QMessageBox::warning(this, tr("Error"), tr("Can not open sequence files."));
	}

	selectSubtitle();
	updateActions();

	m_ui->seekSlider->setValue(0);
}

void MainWindow::actionOpenRecent(QAction *action)
{
	if (!action->data().toString().isEmpty())
	{
		actionOpen(action->data().toString());
	}
}

void MainWindow::actionClearRecentFiles()
{
	QSettings().remove("recentFiles");
}

void MainWindow::actionSave()
{
	if (m_currentPath.isEmpty())
	{
		actionSaveAs();
	}
	else
	{
		saveSubtitles(m_currentPath);
	}
}

void MainWindow::actionSaveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Subtitle file"), (m_currentPath.isEmpty() ? QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() : QFileInfo(m_currentPath).dir().path()));

	if (!fileName.isEmpty() && saveSubtitles(fileName))
	{
		QFileInfo fileInfo(fileName);
		QStringList recentFiles = QSettings().value("recentFiles").toStringList();
		recentFiles.removeAll(fileInfo.absoluteFilePath());
		recentFiles.prepend(fileInfo.absoluteFilePath());
		recentFiles = recentFiles.mid(0, 10);

		QSettings().setValue("recentFiles", recentFiles);
	}
}

void MainWindow::actionAboutApplication()
{
	QMessageBox::about(this, tr("About Subtitles Editor"), QString(tr("<b>Subtitles Editor %1</b><br>Subtitles previewer and editor for Warzone 2100.").arg(QApplication::instance()->applicationVersion())));
}

void MainWindow::errorOccured(QMediaPlayer::Error error)
{
	Q_UNUSED(error)

	QMessageBox::warning(this, tr("Error"), m_mediaPlayer->errorString());
}

void MainWindow::stateChanged(QMediaPlayer::State state)
{
	switch (state)
	{
		case QMediaPlayer::StoppedState:
			m_ui->actionPlayPause->setText(tr("Play"));
			m_ui->actionPlayPause->setIcon(QIcon::fromTheme("media-playback-play", style()->standardIcon(QStyle::SP_MediaPlay)));
			m_ui->actionStop->setEnabled(false);
			m_ui->seekSlider->setValue(0);
			m_ui->seekSlider->setRange(0, 0);
			m_subtitlesTopWidget->setHtml(QString());
			m_subtitlesBottomWidget->setHtml(QString());
			m_videoWidget->hide();

			emit timeChanged(QString("00:00.0 / %1").arg(timeToString(m_mediaPlayer->duration(), true)));

			break;
		case QMediaPlayer::PlayingState:
			m_ui->actionPlayPause->setText(tr("Pause"));
			m_ui->actionPlayPause->setEnabled(true);
			m_ui->actionPlayPause->setIcon(QIcon::fromTheme("media-playback-pause", style()->standardIcon(QStyle::SP_MediaPause)));
			m_ui->actionStop->setEnabled(true);
			m_ui->seekSlider->setValue(0);
			m_ui->seekSlider->setRange(0, m_mediaPlayer->duration());
			m_videoWidget->show();

			break;
		case QMediaPlayer::PausedState:
			m_ui->actionPlayPause->setText(tr("Play"));
			m_ui->actionPlayPause->setEnabled(true);
			m_ui->actionPlayPause->setIcon(QIcon::fromTheme("media-playback-play", style()->standardIcon(QStyle::SP_MediaPlay)));
			m_ui->actionStop->setEnabled(true);

			break;
		default:
			break;
	}
}

void MainWindow::durationChanged(qint64 duration)
{
	m_ui->seekSlider->setRange(0, duration);
}

void MainWindow::positionChanged(qint64 position)
{
	m_ui->seekSlider->setValue(position);

	QString currentBottomSubtitles;
	QString currentTopSubtitles;
	const QTime currentTime = QTime(0, 0, 0).addMSecs(m_mediaPlayer->position());

	emit timeChanged(QString("%1 / %2").arg(timeToString(m_mediaPlayer->position(), true)).arg(timeToString(m_mediaPlayer->duration(), true)));

	for (int i = 0; i < m_subtitles[0].count(); ++i)
	{
		if (m_subtitles[0].at(i).begin < currentTime && m_subtitles[0].at(i).end > currentTime)
		{
			currentTopSubtitles.append(m_subtitles[0].at(i).text);
			currentTopSubtitles.append("<br>");
		}
	}

	for (int i = 0; i < m_subtitles[1].count(); ++i)
	{
		if (m_subtitles[1].at(i).begin < currentTime && m_subtitles[1].at(i).end > currentTime)
		{
			currentBottomSubtitles.append(m_subtitles[1].at(i).text);
			currentBottomSubtitles.append("<br>");

			m_currentSubtitle = i;
		}
	}

	selectSubtitle();

	m_subtitlesTopWidget->setHtml(currentTopSubtitles.left(currentTopSubtitles.length() - 6));
	m_subtitlesBottomWidget->setHtml(currentBottomSubtitles.left(currentBottomSubtitles.length() - 6));
}

void MainWindow::playPause()
{
	if (m_mediaPlayer->state() == QMediaPlayer::PlayingState)
	{
		m_mediaPlayer->pause();
	}
	else
	{
		m_mediaPlayer->play();
	}
}

void MainWindow::seek(int position)
{
	m_mediaPlayer->setPosition(position);
}

void MainWindow::selectTrack(int track)
{
	m_currentSubtitle = 0;
	m_currentTrack = track;

	selectSubtitle();
	updateActions();
}

void MainWindow::addSubtitle()
{
	Subtitle subtitle;
	subtitle.position = QPoint(20, 432);

	m_subtitles[m_currentTrack].insert(m_currentSubtitle, subtitle);

	nextSubtitle();
	updateActions();

	setWindowModified(true);
}

void MainWindow::removeSubtitle()
{
	if (QMessageBox::question(this, tr("Remove Subtitle"), tr("Are you sure that you want to remove this subtitle?")))
	{
		m_subtitles[m_currentTrack].removeAt(m_currentSubtitle);

		selectSubtitle();
		updateActions();
	}

	setWindowModified(true);
}

void MainWindow::previousSubtitle()
{
	--m_currentSubtitle;

	selectSubtitle();
}

void MainWindow::nextSubtitle()
{
	++m_currentSubtitle;

	selectSubtitle();
}

void MainWindow::selectSubtitle()
{
	disconnect(m_ui->subtitleTextEdit, SIGNAL(textChanged()), this, SLOT(updateSubtitle()));
	disconnect(m_ui->xPositionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSubtitle()));
	disconnect(m_ui->yPositionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSubtitle()));
	disconnect(m_ui->beginTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(updateSubtitle()));
	disconnect(m_ui->lengthTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(updateSubtitle()));

	if (m_currentTrack < 0 || m_currentTrack > 1)
	{
		m_currentTrack = 0;
	}

	if (m_currentSubtitle < 0)
	{
		if (m_subtitles[m_currentTrack].count() > 0)
		{
			m_currentSubtitle = (m_subtitles[m_currentTrack].count() - 1);
		}
		else
		{
			m_currentSubtitle = 0;
		}
	}
	else if (m_currentSubtitle >= m_subtitles[m_currentTrack].count())
	{
		m_currentSubtitle = 0;
	}

	if (m_currentSubtitle < m_subtitles[m_currentTrack].count())
	{
		m_ui->subtitleTextEdit->setPlainText(m_subtitles[m_currentTrack].at(m_currentSubtitle).text);
		m_ui->beginTimeEdit->setTime(m_subtitles[m_currentTrack].at(m_currentSubtitle).begin);
		m_ui->lengthTimeEdit->setTime(QTime(0, 0, 0).addMSecs(m_subtitles[m_currentTrack].at(m_currentSubtitle).begin.msecsTo(m_subtitles[m_currentTrack].at(m_currentSubtitle).end)));
		m_ui->xPositionSpinBox->setValue(m_subtitles[m_currentTrack].at(m_currentSubtitle).position.x());
		m_ui->yPositionSpinBox->setValue(m_subtitles[m_currentTrack].at(m_currentSubtitle).position.y());
	}
	else
	{
		m_ui->subtitleTextEdit->clear();
		m_ui->beginTimeEdit->setTime(QTime(0, 0, 0));
		m_ui->lengthTimeEdit->setTime(QTime(0, 0, 0));
		m_ui->xPositionSpinBox->setValue(0);
		m_ui->yPositionSpinBox->setValue(0);
	}

	connect(m_ui->subtitleTextEdit, SIGNAL(textChanged()), this, SLOT(updateSubtitle()));
	connect(m_ui->xPositionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSubtitle()));
	connect(m_ui->yPositionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSubtitle()));
	connect(m_ui->beginTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(updateSubtitle()));
	connect(m_ui->lengthTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(updateSubtitle()));
}

void MainWindow::updateSubtitle()
{
	if (m_currentSubtitle == 0 && m_subtitles[m_currentTrack].count() == 0)
	{
		Subtitle subtitle;

		m_subtitles[m_currentTrack].append(subtitle);
	}

	m_subtitles[m_currentTrack][m_currentSubtitle].text = m_ui->subtitleTextEdit->toPlainText();
	m_subtitles[m_currentTrack][m_currentSubtitle].position = QPoint(m_ui->xPositionSpinBox->value(), m_ui->yPositionSpinBox->value());
	m_subtitles[m_currentTrack][m_currentSubtitle].begin = m_ui->beginTimeEdit->time();
	m_subtitles[m_currentTrack][m_currentSubtitle].end = m_ui->beginTimeEdit->time().addMSecs(m_ui->lengthTimeEdit->time().msecsTo(QTime(0, 0, 0)));

	setWindowModified(true);
	updateActions();
}

void MainWindow::rescaleSubtitles()
{
	bool ok = false;
	double scale = QInputDialog::getDouble(this, tr("Rescale Subtitles"), tr("Insert time multiplier:"), 1, 0, 100, 5, &ok);

	for (int i = 0; i < m_subtitles[0].count(); ++i)
	{
		m_subtitles[0][i].begin = QTime(0, 0, 0).addMSecs(QTime(0, 0, 0).msecsTo(m_subtitles[0][i].begin) * scale);
		m_subtitles[0][i].end = QTime(0, 0, 0).addMSecs(QTime(0, 0, 0).msecsTo(m_subtitles[0][i].end) * scale);
	}

	for (int i = 0; i < m_subtitles[1].count(); ++i)
	{
		m_subtitles[1][i].begin = QTime(0, 0, 0).addMSecs(QTime(0, 0, 0).msecsTo(m_subtitles[1][i].begin) * scale);
		m_subtitles[1][i].end = QTime(0, 0, 0).addMSecs(QTime(0, 0, 0).msecsTo(m_subtitles[1][i].end) * scale);
	}

	selectSubtitle();
}

void MainWindow::updateAudio()
{
	m_ui->volumeSlider->setToolTip(tr("Volume: %1%").arg(m_mediaPlayer->volume()));
}

void MainWindow::updateVideo()
{
	m_videoWidget->setSize(m_ui->graphicsView->size());

	m_ui->graphicsView->centerOn(m_videoWidget);
	m_ui->graphicsView->scene()->setSceneRect(m_ui->graphicsView->rect());

	m_subtitlesTopWidget->setPos(5, 5);
	m_subtitlesTopWidget->setTextWidth(m_ui->graphicsView->scene()->width() - 10);

	m_subtitlesBottomWidget->setPos(5, (m_ui->graphicsView->scene()->height()- 5 - m_subtitlesBottomWidget->shape().controlPointRect().height()));
	m_subtitlesBottomWidget->setTextWidth(m_ui->graphicsView->scene()->width() - 10);
}

void MainWindow::updateActions()
{
	const bool available = (!m_subtitles[0].isEmpty() || !m_subtitles[1].isEmpty());

	m_ui->actionSave->setEnabled(available || isWindowModified());
	m_ui->actionSaveAs->setEnabled(available || isWindowModified());
	m_ui->actionPrevious->setEnabled(available && m_subtitles[m_currentTrack].count() > 1);
	m_ui->actionNext->setEnabled(available && m_subtitles[m_currentTrack].count() > 1);
	m_ui->actionRemove->setEnabled(available);
	m_ui->actionRescale->setEnabled(available);
}

void MainWindow::updateRecentFilesMenu()
{
	const QStringList recentFiles = QSettings().value("recentFiles").toStringList();

	for (int i = 0; i < 10; ++i)
	{
		if (i < recentFiles.count())
		{
			QFileInfo fileInfo(recentFiles.at(i));

			m_ui->menuOpenRecent->actions().at(i)->setText(QString("%1. %2 (%3)").arg(i + 1).arg(fileInfo.fileName()).arg(recentFiles.at(i)));
			m_ui->menuOpenRecent->actions().at(i)->setData(recentFiles.at(i));
			m_ui->menuOpenRecent->actions().at(i)->setVisible(true);
		}
		else
		{
			m_ui->menuOpenRecent->actions().at(i)->setVisible(false);
		}
	}

	m_ui->menuOpenRecent->setEnabled(recentFiles.count());
}

QString MainWindow::timeToString(qint64 time, bool readable)
{
	QString string;
	int fractions = (time / 100);
	int seconds = (fractions / 10);
	int minutes = 0;

	if (readable)
	{
		minutes = (seconds / 60);

		if (minutes < 10)
		{
			string.append('0');
		}

		string.append(QString::number(minutes));
		string.append(':');

		seconds = (seconds - (minutes * 60));

		if (seconds < 10)
		{
			string.append('0');
		}
	}

	string.append(QString::number(seconds));
	string.append('.');

	fractions = (fractions - (seconds * 10) - (minutes * 600));

	string.append(QString::number(fractions));

	return string;
}

bool MainWindow::openFile(const QString &fileName)
{
	if (!QFile::exists(fileName))
	{
		return false;
	}

	m_currentPath = fileName.left(fileName.lastIndexOf('.'));

	const QString oggFile = m_currentPath + ".ogg";
	const QString ogmFile = m_currentPath + ".ogm";
	const QString ogvFile = m_currentPath + ".ogv";
	const QString txtFile = m_currentPath + ".txt";
	const QString txaFile = m_currentPath + ".txa";

	m_subtitles[0].clear();
	m_subtitles[1].clear();

	if (QFile::exists(oggFile) && !openMovie(oggFile))
	{
		return false;
	}

	if (QFile::exists(ogmFile) && !openMovie(ogmFile))
	{
		return false;
	}

	if (QFile::exists(ogvFile) && !openMovie(ogvFile))
	{
		return false;
	}

	if (QFile::exists(txaFile) && !openSubtitles(txaFile, 0))
	{
		return false;
	}

	if (QFile::exists(txtFile) && !openSubtitles(txtFile, 1))
	{
		return false;
	}

	selectTrack(1);

	QString title = QFileInfo(fileName).fileName();
	title = title.left(title.indexOf('.'));

	emit fileChanged(title);

	setWindowModified(false);
	setWindowTitle(tr("%1 - %2[*]").arg("Subtitles Editor").arg(title));

	QFileInfo fileInfo(fileName);
	QStringList recentFiles = QSettings().value("recentFiles").toStringList();
	recentFiles.removeAll(fileInfo.absoluteFilePath());
	recentFiles.prepend(fileInfo.absoluteFilePath());
	recentFiles = recentFiles.mid(0, 10);

	QSettings().setValue("recentFiles", recentFiles);
	QSettings().setValue("lastUsedDir", fileInfo.dir().path());

	return true;
}

bool MainWindow::openMovie(const QString &fileName)
{
	QString title = QFileInfo(fileName).fileName();
	title = title.left(title.indexOf('.'));

	setWindowTitle(tr("%1 - %2[*]").arg("Subtitles Editor").arg(title));

	emit fileChanged(title);
	emit timeChanged(QString("00:00.0 / %1").arg(timeToString(m_mediaPlayer->duration(), true)));

	m_mediaPlayer->setMedia(QUrl::fromLocalFile(fileName));

	m_ui->actionPlayPause->setEnabled(true);

	return true;
}

bool MainWindow::openSubtitles(const QString &fileName, int index)
{
	QFile file(fileName);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, tr("Error"), tr("Can not read subtitle file:\n%1").arg(fileName));

		return false;
	}

	m_subtitles[index].clear();

	QTextStream textStream(&file);

	while (!textStream.atEnd())
	{
		QString line = textStream.readLine().trimmed();

		if (line.isEmpty() || line.startsWith("//"))
		{
			continue;
		}

		QRegExp expression("(\\d+)\\s+(\\d+)\\s+([\\d\\.]+)\\s+([\\d\\.]+)\\s+_?\\(?\"(.+)\"\\)?");

		if (expression.exactMatch(line))
		{
			QStringList capturedTexts = expression.capturedTexts();
			Subtitle subtitle;

			subtitle.text = capturedTexts.value(5);
			subtitle.begin = QTime(0, 0, 0).addMSecs(capturedTexts.value(3).toFloat() * 1000);
			subtitle.end = QTime(0, 0, 0).addMSecs(capturedTexts.value(4).toFloat() * 1000);
			subtitle.position = QPoint(capturedTexts.value(1).toInt(), capturedTexts.value(2).toInt());

			m_subtitles[index].append(subtitle);
		}
	}

	file.close();

	return true;
}

bool MainWindow::saveSubtitles(const QString &fileName)
{
	for (int i = 1; i >= 0; --i)
	{
		if (m_subtitles[i].isEmpty())
		{
			continue;
		}

		QString path = (fileName.contains(QRegExp("\\.(txt|txa|ogg|ogm|ogv)$", Qt::CaseInsensitive)) ? fileName.left(fileName.lastIndexOf('.')) : fileName) + (i ? ".txt" : ".txa");
		QFile file(path);

		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QMessageBox::warning(this, tr("Error"), tr("Can not save subtitle file:\n%1").arg(path));

			return false;
		}

		QTextStream textStream(&file);

		for (int j = 0; j < m_subtitles[i].count(); ++j)
		{
			textStream << QString("%1\t%2\t\t%3\t%4\t_(\"%5\")\n").arg(m_subtitles[i][j].position.x()).arg(m_subtitles[i][j].position.y()).arg(timeToString(QTime(0, 0, 0).msecsTo(m_subtitles[i][j].begin))).arg(timeToString(QTime(0, 0, 0).msecsTo(m_subtitles[i][j].end))).arg(m_subtitles[i][j].text);

			if ((j + 1) < m_subtitles[i].count() && m_subtitles[i][j].begin != m_subtitles[i][j + 1].begin)
			{
				textStream << "\n";
			}
		}

		file.close();
	}

	QString title = QFileInfo(fileName).fileName();
	title = title.left(title.indexOf('.'));

	emit fileChanged(title);

	setWindowTitle(tr("%1 - %2[*]").arg("Subtitles Editor").arg(title));
	setWindowModified(false);

	return true;
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::Resize)
	{
		updateVideo();
	}

	return QObject::eventFilter(object, event);
}
