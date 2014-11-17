#include <QFileDialog>
#include "ImportZySingle.h"
#include "Model/DatabaseManager.h"
#include "Model/PathManager.h"
#include "Dialogs/BjMessageBox.h"


ImportZySingle::ImportZySingle(QWidget *parent /* = NULL */) :QDialog(parent),
ui(new Ui::ImportZySingle),
mpaser(NULL)
{
	ui->setupUi(this);
	InitSlot();
}

ImportZySingle::~ImportZySingle()
{
	delete ui;
	if (mpaser != NULL){
		delete mpaser;
	}
}

void ImportZySingle::InitView()
{

}

void ImportZySingle::InitSlot()
{
	connect(ui->ButtonScan, SIGNAL(clicked()), this, SLOT(ButtonScanSlot()));
	connect(ui->ButtonOk, SIGNAL(clicked()), this, SLOT(ButtonOkSlot()));
}

void ImportZySingle::ButtonScanSlot()
{
	QString defaultPath = PathManager::ReadLastPath(QStringLiteral("importZySingle"));
	if (defaultPath.isEmpty()){
		defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	}
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("���ļ�"), defaultPath, QStringLiteral("Excel�ļ� (*.xlsx)"));
	if (!fileName.isEmpty()){
		ui->LineEditXlsx->setText(fileName);
		mxlsxName = fileName;
		PathManager::WriteLastPath(QStringLiteral("importZySingle"), fileName.left(fileName.replace("\\", "/").lastIndexOf("/")));
		mpaser = new XlsxParser(mxlsxName);
		QStringList sheets = mpaser->GetAllSheet();
		if (sheets.isEmpty()){
			BjMessageBox::warning(NULL, QStringLiteral("����!"), QStringLiteral("Excel��ʧ��"), QMessageBox::Ok, QMessageBox::NoButton);
			return;
		}
		for (QStringList::iterator ite = sheets.begin(); ite != sheets.end(); ite++)
		{
			ui->ComboBoxSheet->addItem(*ite);
		}
		ui->ComboBoxSheet->setCurrentIndex(0);
	}
}

void ImportZySingle::ButtonOkSlot()
{
	if (mpaser == NULL){
		BjMessageBox::information(NULL, QStringLiteral("������ʾ!"), QStringLiteral("����ѡ����ȷ��Excel�ļ�"), QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}
	QString channelId = PathManager::GetChannelId();
	QString baseNm = PathManager::GetBaseNm();
	QString originalNm = PathManager::GetOriginalNm();
	QString appNmFile = PathManager::GetAppNmFile();
	QString oriAppNm = PathManager::GetOriAppNm();
	QString newAppNm = PathManager::GetNewAppNm();
	if (channelId.isEmpty()){
		BjMessageBox::information(NULL, QStringLiteral("������ʾ!"), QStringLiteral("����������idδ���ã�������������id"), QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}
	else if (baseNm.isEmpty()){
		BjMessageBox::information(NULL, QStringLiteral("������ʾ!"), QStringLiteral("���������ǰ׺δ���ã��������ð���ǰ׺"), QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}
	else if (originalNm.isEmpty()){
		BjMessageBox::information(NULL, QStringLiteral("������ʾ!"), QStringLiteral("������ԭʼ����δ���ã���������ԭʼ����"), QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}
	else if (appNmFile.isEmpty()){
		BjMessageBox::information(NULL, QStringLiteral("������ʾ!"), QStringLiteral("������Ӧ�����ļ�δ���ã���������Ӧ�����ļ�"), QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}
	else if (oriAppNm.isEmpty()){
		BjMessageBox::information(NULL, QStringLiteral("������ʾ!"), QStringLiteral("������Ӧ����ԭ�ַ���δ���ã���������Ӧ����ԭ�ַ���"), QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}
	else if (newAppNm.isEmpty()){
		BjMessageBox::information(NULL, QStringLiteral("������ʾ!"), QStringLiteral("������Ӧ�������ַ���δ���ã���������Ӧ�������ַ���"), QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}
	DatabaseManager::GetInstance()->ImportFromXlsx(*mpaser, ui->ComboBoxSheet->currentText(), 
	channelId,
	baseNm,
	originalNm,
	appNmFile,
	oriAppNm,
	newAppNm
		);
}