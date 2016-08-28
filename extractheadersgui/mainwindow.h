#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
	class ExtractHeadersWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void openSolutionDialog();
private:
	Ui::ExtractHeadersWindow *ui;
};

#endif // MAINWINDOW_H
