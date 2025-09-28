#include "commandinfo.h"

#include <QList>

CommandInfo::CommandInfo(const QString& filename, const QString& videoCodec, const QString& audioCodec, int indexVideo, int indexAudio, int indexSubtitle) :
	m_filename(filename),
	m_videoCodec(videoCodec),
	m_audioCodec(audioCodec),
	m_indexVideo(indexVideo),
	m_indexAudio(indexAudio),
	m_indexSubtitle(indexSubtitle)
{}

QString CommandInfo::filename() const
{
	return m_filename;
}

QString CommandInfo::videoCodec() const
{
	return m_videoCodec;
}

QString CommandInfo::audioCodec() const
{
	return m_audioCodec;
}

int CommandInfo::indexVideo() const
{
	return m_indexVideo;
}

int CommandInfo::indexAudio() const
{
	return m_indexAudio;
}

int CommandInfo::indexSubtitle() const
{
	return m_indexSubtitle;
}

bool CommandInfo::needsSubtitles() const
{
	return m_indexSubtitle != -1;
}

void CommandInfo::setFilename(const QString& newFilename)
{
	m_filename = newFilename;
}

void CommandInfo::setVideoCodec(const QString& newVideoCodec)
{
	m_videoCodec = newVideoCodec;
}

void CommandInfo::setAudioCodec(const QString& newAudioCodec)
{
	m_audioCodec = newAudioCodec;
}

void CommandInfo::setIndexVideo(int newIndexVideo)
{
	m_indexVideo = newIndexVideo;
}

void CommandInfo::setIndexAudio(int newIndexAudio)
{
	m_indexAudio = newIndexAudio;
}

void CommandInfo::setIndexSubtitle(int newIndexSubtitle)
{
	m_indexSubtitle = newIndexSubtitle;
}

QString CommandInfo::toSubtitleCommand() const
{
	QString ext = m_filename.split('.').last();
	QString filenameWithoutExt = m_filename;
	filenameWithoutExt.replace(ext, "");
	return "ffmpeg -hwaccel auto -y -i \"" + m_filename + "\" -map 0:s:" + QString::number(m_indexSubtitle) + " \"" + filenameWithoutExt + "vtt\"";
}

QString CommandInfo::toVideoCommand() const
{
	QString ext = m_filename.split('.').last();
	QString filenameWithoutExt = m_filename;

	if (ext == "mp4")
		filenameWithoutExt.replace(".mp4", "_1.");
	else
		filenameWithoutExt.replace(ext, "");

    QString command;
    if(m_indexAudio == -1){
        command = "ffmpeg -hwaccel auto -y -i \"" + m_filename + "\" -c:v " + m_videoCodec;
        command += " -map 0:v:" + QString::number(m_indexVideo);
        command += " -sn \"" + filenameWithoutExt + "mp4\"";
    }else{
        command = "ffmpeg -hwaccel auto -y -i \"" + m_filename + "\" -c:v " + m_videoCodec + " -c:a " + m_audioCodec;
        command += " -map 0:v:" + QString::number(m_indexVideo) + " -map 0:a:" + QString::number(m_indexAudio);
        command += " -sn \"" + filenameWithoutExt + "mp4\"";
    }

	return command;
}
