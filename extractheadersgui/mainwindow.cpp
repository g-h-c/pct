#include "mainwindow.h"
#include "ui_extractheaders.h"
#include <qfiledialog.h>
#include <qaction.h>

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::ExtractHeadersWindow)
{
	ui->setupUi(this);

	connect(ui->actionOpen_solution, &QAction::triggered, this, &MainWindow::openSolutionDialog);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::openSolutionDialog()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open solution"), NULL, tr("Solution Files (*.sln)"));
}

