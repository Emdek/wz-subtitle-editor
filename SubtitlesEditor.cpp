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

#include "SubtitlesEditor.h"
#include "ui_SubtitlesEditor.h"

#include <QtCore/QSettings>

#include <QtGui/QTabBar>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QDesktopServices>
#include <QtGui/QGraphicsDropShadowEffect>

#include <Phonon/AudioOutput>
#include <Phonon/VideoWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
	m_ui(new Ui::MainWindow),
	m_videoWidget(new QGraphicsProxyWidget()),
	m_subtitlesTopWidget(new QGraphicsTextItem(m_videoWidget)),
	m_subtitlesBottomWidget(new QGraphicsTextItem(m_videoWidget)),
	m_currentSubtitle(0),
	m_currentTrack(0)
{
	m_subtitles.append(QList<Subtitle>());
	m_subtitles.append(QList<Subtitle>());

	m_ui->setupUi(this);

	Phonon::VideoWidget *videoWidget = new Phonon::VideoWidget();
	Phonon::AudioOutput *audioOutput = new Phonon::AudioOutput(this);
	audioOutput->setVolume(QSettings().value("volume", 0.8).toReal());

	m_mediaObject = new Phonon::MediaObject(this);
	m_mediaObject->setTickInterval(100);

	Phonon::createPath(m_mediaObject, videoWidget);
	Phonon::createPath(m_mediaObject, audioOutput);

	m_videoWidget->setWidget(videoWidget);

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

	m_fileNameLabel = new QLabel(tr("No file loaded"), this);
	m_fileNameLabel->setMaximumWidth(300);
	m_timeLabel = new QLabel("00:00.0 / 00:00.0", this);

	m_ui->actionOpen->setIcon(QIcon::fromTheme("document-open", style()->standardIcon(QStyle::SP_DirOpenIcon)));
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
	m_ui->seekSlider->setMediaObject(m_mediaObject);
	m_ui->volumeSlider->setAudioOutput(audioOutput);
	m_ui->statusBar->addPermanentWidget(m_fileNameLabel);
	m_ui->statusBar->addPermanentWidget(m_timeLabel);

	resize(QSettings().value("Window/size", size()).toSize());
	move(QSettings().value("Window/position", pos()).toPoint());
	restoreState(QSettings().value("Window/state", QByteArray()).toByteArray());

	setWindowTitle(tr("%1 - Unnamed").arg("Subtitles Editor"));

	updateVideo();

	connect(m_ui->actionOpen, SIGNAL(triggered()), this, SLOT(actionOpen()));
	connect(m_ui->actionSave, SIGNAL(triggered()), this, SLOT(actionSave()));
	connect(m_ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(actionSaveAs()));
	connect(m_ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(m_ui->actionAdd, SIGNAL(triggered()), this, SLOT(addSubtitle()));
	connect(m_ui->actionRemove, SIGNAL(triggered()), this, SLOT(removeSubtitle()));
	connect(m_ui->actionPrevious, SIGNAL(triggered()), this, SLOT(previousSubtitle()));
	connect(m_ui->actionNext, SIGNAL(triggered()), this, SLOT(nextSubtitle()));
	connect(m_ui->actionRescale, SIGNAL(triggered()), this, SLOT(rescaleSubtitles()));
	connect(m_ui->actionPlayPause, SIGNAL(triggered()), this, SLOT(playPause()));
	connect(m_ui->actionStop, SIGNAL(triggered()), m_mediaObject, SLOT(stop()));
	connect(m_ui->actionAboutQt, SIGNAL(triggered()), QApplication::instance(), SLOT(aboutQt()));
	connect(m_ui->actionAboutApplication, SIGNAL(triggered()), this, SLOT(actionAboutApplication()));
	connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(selectTrack(int)));
	connect(m_mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(stateChanged(Phonon::State)));
	connect(m_mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick()));
	connect(m_mediaObject, SIGNAL(finished()), this, SLOT(finished()));
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
	QSettings settings;
	settings.setValue("Window/size", size());
	settings.setValue("Window/position", pos());
	settings.setValue("Window/state", saveState());
	settings.setValue("volume", m_ui->volumeSlider->audioOutput()->volume());

	event->accept();
}

void MainWindow::openMovie(const QString &fileName)
{
	QString title = QFileInfo(fileName).fileName();
	title = title.left(title.indexOf('.'));

	setWindowTitle(tr("%1 - %2").arg("Subtitles Editor").arg(title));

	m_fileNameLabel->setText(title);
	m_timeLabel->setText(QString("00:00.0 / %1").arg(timeToString(m_mediaObject->totalTime())));

	m_mediaObject->setCurrentSource(Phonon::MediaSource(fileName));

	m_videoWidget->widget()->show();
	m_videoWidget->widget()->update();

	m_ui->actionPlayPause->setEnabled(true);
}

void MainWindow::actionOpen()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Video or Subtitle file"),
			QSettings().value("lastUsedDir", QDesktopServices::storageLocation(QDesktopServices::HomeLocation)).toString(),
			tr("Video and subtitle files (*.txt *.og?)"));

	if (!fileName.isEmpty())
	{
		const QString basename = fileName.left(fileName.length() - 3);
		const QString oggfile = basename + "ogg";
		const QString ogmfile = basename + "ogm";
		const QString ogvfile = basename + "ogv";
		const QString txtfile = basename + "txt";
		const QString txafile = basename + "txa";

		m_subtitles[0].clear();
		m_subtitles[1].clear();

		if (QFile::exists(oggfile))
		{
			openMovie(oggfile);
		}

		if (QFile::exists(ogmfile))
		{
			openMovie(ogmfile);
		}

		if (QFile::exists(ogvfile))
		{
			openMovie(ogvfile);
		}

		if (QFile::exists(txtfile))
		{
			openSubtitles(txtfile, 1);

			m_currentPath = fileName;
		}

		if (QFile::exists(txafile))
		{
			openSubtitles(txafile, 0);

			m_currentPath = fileName;
		}

		selectTrack(1);

		QSettings().setValue("lastUsedDir", QFileInfo(fileName).dir().path());
	}
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
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Subtitle file"), (m_currentPath.isEmpty()?QDesktopServices::storageLocation(QDesktopServices::HomeLocation):QFileInfo(m_currentPath).dir().path()));

	if (!fileName.isEmpty())
	{
		saveSubtitles(fileName);
	}
}

void MainWindow::actionAboutApplication()
{
	QMessageBox::about(this, tr("About Subtitles Editor"), QString(tr("<b>Subtitles Editor %1</b><br />Subtitles previewer and editor for Warzone 2100.").arg(QApplication::instance()->applicationVersion())));
}

void MainWindow::stateChanged(Phonon::State state)
{
	switch (state)
	{
		case Phonon::ErrorState:
			if (m_mediaObject->errorType() == Phonon::FatalError)
			{
				QMessageBox::warning(this, tr("Fatal Error"), m_mediaObject->errorString());
			}
			else
			{
				QMessageBox::warning(this, tr("Error"), m_mediaObject->errorString());
			}

			m_ui->actionPlayPause->setEnabled(false);
		case Phonon::StoppedState:
			m_ui->actionPlayPause->setText(tr("Play"));
			m_ui->actionPlayPause->setIcon(QIcon::fromTheme("media-playback-play", style()->standardIcon(QStyle::SP_MediaPlay)));
			m_ui->actionStop->setEnabled(false);
			m_timeLabel->setText(QString("00:00.0 / %1").arg(timeToString(m_mediaObject->totalTime())));
			break;
		case Phonon::PlayingState:
			m_ui->actionPlayPause->setText(tr("Pause"));
			m_ui->actionPlayPause->setEnabled(true);
			m_ui->actionPlayPause->setIcon(QIcon::fromTheme("media-playback-pause", style()->standardIcon(QStyle::SP_MediaPause)));
			m_ui->actionStop->setEnabled(true);
			break;
		case Phonon::PausedState:
			m_ui->actionPlayPause->setText(tr("Play"));
			m_ui->actionPlayPause->setEnabled(true);
			m_ui->actionPlayPause->setIcon(QIcon::fromTheme("media-playback-play", style()->standardIcon(QStyle::SP_MediaPlay)));
			m_ui->actionStop->setEnabled(true);
			break;
		default:
			break;
	}
}

void MainWindow::finished()
{
	m_mediaObject->stop();

	m_videoWidget->widget()->show();
	m_videoWidget->widget()->update();
	m_subtitlesTopWidget->setHtml(QString());
	m_subtitlesBottomWidget->setHtml(QString());
}

void MainWindow::tick()
{
	m_timeLabel->setText(QString("%1 / %2").arg(timeToString(m_mediaObject->currentTime())).arg(timeToString(m_mediaObject->totalTime())));

	QString currentBottomSubtitles;
	QString currentTopSubtitles;
	const QTime currentTime = QTime().addMSecs(m_mediaObject->currentTime());

	for (int i = 0; i < m_subtitles[0].count(); ++i)
	{
		if (m_subtitles[0].at(i).begin < currentTime && m_subtitles[0].at(i).end > currentTime)
		{
			currentTopSubtitles.append(m_subtitles[0].at(i).text);
			currentTopSubtitles.append("<br />");
		}
	}

	for (int i = 0; i < m_subtitles[1].count(); ++i)
	{
		if (m_subtitles[1].at(i).begin < currentTime && m_subtitles[1].at(i).end > currentTime)
		{
			currentBottomSubtitles.append(m_subtitles[1].at(i).text);
			currentBottomSubtitles.append("<br />");

			m_currentSubtitle = i;
		}
	}

	selectSubtitle();

	m_subtitlesTopWidget->setHtml(currentTopSubtitles.left(currentTopSubtitles.length() - 6));
	m_subtitlesBottomWidget->setHtml(currentBottomSubtitles.left(currentBottomSubtitles.length() - 6));

	updateVideo();
}

void MainWindow::selectTrack(int track)
{
	m_currentSubtitle = 0;
	m_currentTrack = track;

	selectSubtitle();
}

void MainWindow::addSubtitle()
{
	Subtitle subtitle;
	subtitle.position = QPoint(20, 432);

	m_subtitles[m_currentTrack].insert(m_currentSubtitle, subtitle);

	nextSubtitle();
}

void MainWindow::removeSubtitle()
{
	if (QMessageBox::question(this, tr("Remove Subtitle"), tr("Are you sure that you want to remove this subtitle?")))
	{
		m_subtitles[m_currentTrack].removeAt(m_currentSubtitle);

		selectSubtitle();
	}
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
		m_ui->lengthTimeEdit->setTime(QTime().addMSecs(m_subtitles[m_currentTrack].at(m_currentSubtitle).begin.msecsTo(m_subtitles[m_currentTrack].at(m_currentSubtitle).end)));
		m_ui->xPositionSpinBox->setValue(m_subtitles[m_currentTrack].at(m_currentSubtitle).position.x());
		m_ui->yPositionSpinBox->setValue(m_subtitles[m_currentTrack].at(m_currentSubtitle).position.y());
	}
	else
	{
		m_ui->subtitleTextEdit->clear();
		m_ui->beginTimeEdit->setTime(QTime());
		m_ui->lengthTimeEdit->setTime(QTime());
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
	m_subtitles[m_currentTrack][m_currentSubtitle].end = m_ui->beginTimeEdit->time().addMSecs(m_ui->lengthTimeEdit->time().msecsTo(QTime()));
}

void MainWindow::rescaleSubtitles()
{
	bool ok = false;
	double scale = QInputDialog::getDouble(this, tr("Rescale Subtitles"), tr("Insert time multiplier:"), 1, 0, 100, 5, &ok);

	for (int i = 0; i < m_subtitles[0].count(); ++i)
	{
		m_subtitles[0][i].begin =  QTime().addMSecs(QTime().msecsTo(m_subtitles[0][i].begin) * scale);
		m_subtitles[0][i].end =  QTime().addMSecs(QTime().msecsTo(m_subtitles[0][i].end) * scale);
	}

	for (int i = 0; i < m_subtitles[1].count(); ++i)
	{
		m_subtitles[1][i].begin =  QTime().addMSecs(QTime().msecsTo(m_subtitles[1][i].begin) * scale);
		m_subtitles[1][i].end =  QTime().addMSecs(QTime().msecsTo(m_subtitles[1][i].end) * scale);
	}

	selectSubtitle();
}

void MainWindow::updateVideo()
{
	m_videoWidget->resize(m_ui->graphicsView->size());

	m_ui->graphicsView->centerOn(m_videoWidget);
	m_ui->graphicsView->scene()->setSceneRect(m_ui->graphicsView->rect());

	m_subtitlesTopWidget->setPos(5, 5);
	m_subtitlesTopWidget->setTextWidth(m_ui->graphicsView->scene()->width() - 10);

	m_subtitlesBottomWidget->setPos(5, (m_ui->graphicsView->scene()->height()- 5 - m_subtitlesBottomWidget->shape().controlPointRect().height()));
	m_subtitlesBottomWidget->setTextWidth(m_ui->graphicsView->scene()->width() - 10);
}

void MainWindow::playPause()
{
	if (m_mediaObject->state() == Phonon::PlayingState)
	{
		m_mediaObject->pause();
	}
	else
	{
		m_mediaObject->play();
	}
}

QString MainWindow::timeToString(qint64 time)
{
	QString string;
	int fractions = (time / 100);
	int seconds = (fractions / 10);
	int minutes = (seconds / 60);

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

	string.append(QString::number(seconds));
	string.append('.');

	fractions = (fractions - (seconds * 10) - (minutes * 600));

	string.append(QString::number(fractions));

	return string;
}

void MainWindow::openSubtitles(const QString &fileName, int index)
{
	QFile file(fileName);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, tr("Error"), tr("Can not read subtitle file:\n%1").arg(fileName));

		return;
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
			subtitle.begin = QTime().addMSecs(capturedTexts.value(3).toFloat() * 1000);
			subtitle.end = QTime().addMSecs(capturedTexts.value(4).toFloat() * 1000);
			subtitle.position = QPoint(capturedTexts.value(1).toInt(), capturedTexts.value(2).toInt());

			m_subtitles[index].append(subtitle);
		}
	}

	file.close();

	QString title = QFileInfo(fileName).fileName();
	title = title.left(title.indexOf('.'));

	setWindowTitle(tr("%1 - %2").arg("Subtitles Editor").arg(title));

	m_fileNameLabel->setText(title);
}

bool MainWindow::saveSubtitles(QString fileName)
{
	for (int i = 1; i >= 0; --i)
	{
		if (m_subtitles[i].isEmpty())
		{
			continue;
		}

		if (i == 0)
		{
			fileName = fileName.left(fileName.length() - 3) + "txa";
		}

		QFile file(fileName);

		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QMessageBox::warning(this, tr("Error"), tr("Can not save subtitle file:\n%1").arg(fileName));

			return false;
		}

		QTextStream textStream(&file);

		for (int j = 0; j < m_subtitles[i].count(); ++j)
		{
			textStream << QString("%1\t%2\t\t%3\t%4\t_(\"%5\")\n").arg(m_subtitles[i][j].position.x()).arg(m_subtitles[i][j].position.y()).arg(timeToString(m_subtitles[i][j].begin.msecsTo(QTime()))).arg(timeToString(m_subtitles[i][j].end.msecsTo(QTime()))).arg(m_subtitles[i][j].text);

			if ((j + 1) < m_subtitles[i].count() && m_subtitles[i][j].begin != m_subtitles[i][j + 1].begin)
			{
				textStream << "\n";
			}
		}

		file.close();
	}

	QString title = QFileInfo(fileName).fileName();
	title = title.left(title.indexOf('.'));

	setWindowTitle(tr("%1 - %2").arg("Subtitles Editor").arg(title));

	m_fileNameLabel->setText(title);

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
