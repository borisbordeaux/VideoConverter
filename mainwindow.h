#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QProcess>
#include <QMap>

#include "commandinfo.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

protected:
	// QWidget interface
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;

private slots:
	void removeFile(QListWidgetItem* item);
	void removeCommand(QListWidgetItem* item);
	void generateInfo();
	void launchCommands();

	void parseFFmpegProgress();
	void endFFmpegCommand(int code);

	void updateWidgetCommands();
	void saveChanges();

private:
	QString execute(QString const& cmd);
	void updateCommandList();
    void updateEnabled();

private:
	Ui::MainWindow* ui;

	QProcess* m_process;
	int m_totalDuration = 0;
	QMap<QListWidgetItem*, CommandInfo> m_mapCommandInfo;
	QListWidgetItem* m_currentCommandEdited;
};
#endif // MAINWINDOW_H
