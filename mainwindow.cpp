#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>

MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent), ui(new Ui::MainWindow), m_process(new QProcess(this)), m_currentCommandEdited(nullptr)
{
	ui->setupUi(this);
	this->setAcceptDrops(true);

	connect(m_process, &QProcess::readyReadStandardError, this, &MainWindow::parseFFmpegProgress);
	connect(m_process, &QProcess::finished, this, &MainWindow::endFFmpegCommand);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
	const QMimeData* mimeData = event->mimeData();

	//drop only mkv or mp4 video files
	for (auto const& url : mimeData->urls())
	{
		QString ext = url.toLocalFile().split('.').last();

		if (ext == "mkv" || ext == "mp4")
			event->accept();
	}
}

void MainWindow::dropEvent(QDropEvent* event)
{
	m_mapCommandInfo.clear();
	this->ui->plainTextEdit_info->clear();
	this->ui->listWidget_commands->clear();

	for (auto const& url : event->mimeData()->urls())
		this->ui->listWidget_files->addItem(url.toLocalFile());

	this->updateEnabled();
}


void MainWindow::removeFile(QListWidgetItem* item)
{
	this->ui->listWidget_files->setCurrentItem(nullptr);
	delete item;
	m_mapCommandInfo.clear();
	this->ui->plainTextEdit_info->clear();
	this->ui->listWidget_commands->clear();
	m_currentCommandEdited = nullptr;
	this->updateEnabled();
}


void MainWindow::removeCommand(QListWidgetItem* item)
{
	delete item;
	this->updateEnabled();
}

void MainWindow::generateInfo()
{
	this->ui->plainTextEdit_info->clear();
	this->ui->progressBar->setValue(0);
	m_mapCommandInfo.clear();

	for (int i = 0; i < this->ui->listWidget_files->count(); i++)
	{
		QString filename = this->ui->listWidget_files->item(i)->text();

		QString cmd = "ffprobe -v error -show_streams -of json \"" + filename + "\"";
		QString output = execute(cmd);
		QJsonDocument doc = QJsonDocument::fromJson(output.toStdString().c_str());
		QJsonObject root = doc.object();

		if (root.contains("streams") && root["streams"].isArray())
		{
			QJsonArray streams = root["streams"].toArray();

			//to prepare commands
			QString ext = filename.split('.').last();
			QString filenameWithoutExt = filename;
			filenameWithoutExt.replace(ext, "");
			int indexAudioStream = 0;
			int indexSubtitlesStream = 0;
			int preferedIndexAudioStream = -1;
			int preferedIndexSubtitleStream = -1;
			QString videoCodec = "";
			QString audioCodec = "";

			for (const QJsonValue& val : std::as_const(streams))
			{
				if (val.isObject())
				{
					QJsonObject streamObj = val.toObject();
					QString codecType = streamObj["codec_type"].toString();
					QString codecName = streamObj["codec_name"].toString();
					QString language = streamObj["tags"].toObject()["language"].toString();
					QString title = streamObj["tags"].toObject()["title"].toString();
					this->ui->plainTextEdit_info->insertPlainText(codecType.toUpper() + "  (" + language + ", " + title + ")   |   " + codecName);

					if (codecType == "video")
					{
						QString pixFmt = streamObj["pix_fmt"].toString();
						QString w = QString::number(streamObj["width"].toInt(-1));
						QString h = QString::number(streamObj["height"].toInt(-1));
						this->ui->plainTextEdit_info->insertPlainText(" (" + pixFmt + ")   " + w + " x " + h);

						if (codecName == "h264" && pixFmt == "yuv420p")
							videoCodec = "copy";
						else
							videoCodec = "h264_nvenc -vf format=yuv420p -b:v 6M";
					}

					if (codecType == "audio")
					{
						QString channelLayout = streamObj["channel_layout"].toString();
						this->ui->plainTextEdit_info->insertPlainText("   " + channelLayout);

						if (language == "fre")
						{
							preferedIndexAudioStream = indexAudioStream;

							if (codecName == "aac" && channelLayout == "stereo")
								audioCodec = "copy";
							else
								audioCodec = "aac -ac 2";
						}
					}

					this->ui->plainTextEdit_info->insertPlainText("\n");
					bool createSubtitles = false;

					if (codecType == "subtitle" && codecName == "subrip")
					{
						bool forced = streamObj["disposition"].toObject()["forced"].toInt(-1) == 1;

						if (language == "fre" && forced)
							preferedIndexSubtitleStream = indexSubtitlesStream;
					}

					if (codecType == "subtitle")
						indexSubtitlesStream++;

					if (codecType == "audio")
						indexAudioStream++;
				}
			}

			CommandInfo commandInfo(filename, videoCodec, audioCodec, 0, preferedIndexAudioStream, preferedIndexSubtitleStream);
			m_mapCommandInfo.insert(this->ui->listWidget_files->item(i), commandInfo);
		}

		this->ui->plainTextEdit_info->insertPlainText("---------------------------------------------------------------------------------------------------\n");
	}

	this->updateCommandList();
    this->ui->listWidget_files->setCurrentRow(0);
}

void MainWindow::launchCommands()
{
	if (this->ui->listWidget_commands->count() > 0)
	{
		this->ui->progressBar->setValue(0);
		QString command = this->ui->listWidget_commands->item(0)->text();
		QString input = command.split(" ")[5];

		QString cmd = "ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 " + input;
		QString output = this->execute(cmd).trimmed();
		m_totalDuration = (int)output.toDouble();

		m_process->startCommand(command);
	}
	else
		this->updateEnabled();
}

void MainWindow::parseFFmpegProgress()
{
	QByteArray data = m_process->readAllStandardError();
	QString text = QString::fromUtf8(data);

	// Chercher time=hh:mm:ss.xx
	static QRegularExpression regex("time=(\\d+):(\\d+):(\\d+\\.?\\d*)");
	QRegularExpressionMatchIterator it = regex.globalMatch(text);

	while (it.hasNext())
	{
		QRegularExpressionMatch match = it.next();
		int hours = match.captured(1).toInt();
		int minutes = match.captured(2).toInt();
		double seconds = match.captured(3).toDouble();
		int currentSec = hours * 3600 + minutes * 60 + (int)seconds;

		if (m_totalDuration > 0)
		{
			int progress = (currentSec * 100) / m_totalDuration;
			this->ui->progressBar->setValue(progress);
		}
	}
}

void MainWindow::endFFmpegCommand(int /*code*/)
{
	this->ui->progressBar->setValue(100);
	this->ui->listWidget_commands->takeItem(0);
	this->launchCommands();
}

void MainWindow::updateWidgetCommands()
{
    QListWidgetItem* item = this->ui->listWidget_files->currentItem();
	if (m_mapCommandInfo.contains(item))
	{
		m_currentCommandEdited = item;
		this->ui->comboBox_video->setCurrentIndex(m_mapCommandInfo[item].videoCodec() == "copy" ? 0 : 1);
		this->ui->comboBox_audio->setCurrentIndex(m_mapCommandInfo[item].audioCodec() == "copy" ? 0 : 1);
		this->ui->spinBox_video->setValue(m_mapCommandInfo[item].indexVideo());
		this->ui->spinBox_audio->setValue(m_mapCommandInfo[item].indexAudio());
		this->ui->spinBox_subs->setValue(m_mapCommandInfo[item].indexSubtitle());
		this->updateEnabled();
	}else{
        this->ui->listWidget_files->setCurrentItem(nullptr);
    }
}

void MainWindow::saveChanges()
{
	if (m_currentCommandEdited != nullptr)
	{
		m_mapCommandInfo[m_currentCommandEdited].setVideoCodec(this->ui->comboBox_video->currentText());
		m_mapCommandInfo[m_currentCommandEdited].setAudioCodec(this->ui->comboBox_audio->currentText());
		m_mapCommandInfo[m_currentCommandEdited].setIndexVideo(this->ui->spinBox_video->value());
		m_mapCommandInfo[m_currentCommandEdited].setIndexAudio(this->ui->spinBox_audio->value());
		m_mapCommandInfo[m_currentCommandEdited].setIndexSubtitle(this->ui->spinBox_subs->value());

		this->updateCommandList();
	}
}

QString MainWindow::execute(const QString& cmd)
{
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.toStdString().c_str(), "r"), pclose);

	if (!pipe) throw std::runtime_error("popen() failed!");

	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
		result += buffer.data();

	return result.c_str();
}

void MainWindow::updateCommandList()
{
	this->ui->listWidget_commands->clear();

	for (auto const& command : m_mapCommandInfo)
	{
		if (command.needsSubtitles())
			this->ui->listWidget_commands->addItem(command.toSubtitleCommand());

		this->ui->listWidget_commands->addItem(command.toVideoCommand());
	}

	this->updateEnabled();
}

void MainWindow::updateEnabled()
{
	this->ui->pushButton_info->setDisabled(this->ui->listWidget_files->count() == 0);
	this->ui->pushButton_saveChange->setDisabled(this->ui->listWidget_files->currentItem() == nullptr);
	this->ui->comboBox_video->setDisabled(this->ui->listWidget_files->currentItem() == nullptr);
	this->ui->comboBox_audio->setDisabled(this->ui->listWidget_files->currentItem() == nullptr);
	this->ui->spinBox_video->setDisabled(this->ui->listWidget_files->currentItem() == nullptr);
	this->ui->spinBox_audio->setDisabled(this->ui->listWidget_files->currentItem() == nullptr);
	this->ui->spinBox_subs->setDisabled(this->ui->listWidget_files->currentItem() == nullptr);
	this->ui->pushButton_convert->setDisabled(this->ui->listWidget_commands->count() == 0);
}

