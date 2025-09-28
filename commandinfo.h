#ifndef COMMANDINFO_H
#define COMMANDINFO_H

#include <QString>

class CommandInfo
{
public:
	CommandInfo(const QString& filename, const QString& videoCodec, const QString& audioCodec, int indexVideo, int indexAudio, int indexSubtitle);

	CommandInfo() = default;

	QString filename() const;
	QString videoCodec() const;
	QString audioCodec() const;
	int indexVideo() const;
	int indexAudio() const;
	int indexSubtitle() const;
	bool needsSubtitles() const;

	void setFilename(const QString& newFilename);
	void setVideoCodec(const QString& newVideoCodec);
	void setAudioCodec(const QString& newAudioCodec);
	void setIndexVideo(int newIndexVideo);
	void setIndexAudio(int newIndexAudio);
	void setIndexSubtitle(int newIndexSubtitle);

	QString toSubtitleCommand() const;
	QString toVideoCommand() const;

private:
	QString m_filename;
	QString m_videoCodec;
	QString m_audioCodec;
	int m_indexVideo;
	int m_indexAudio;
	int m_indexSubtitle;
};

#endif // COMMANDINFO_H
