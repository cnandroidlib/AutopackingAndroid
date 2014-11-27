#include <QFileDialog>
#include <QStandardPaths>
#include "BjToolBar.h"
#include "Model/PathManager.h"

BjToolBar::BjToolBar(QWidget *parent)
:QToolBar(parent),
mcombox(parent),
mlablePath(parent),
mlableWay(parent),
mlableCustom(parent),
mlineedit(parent),
mbuttonScan(parent),
mcomboxCustom(parent)
{
	InitView();
	InitSlot();
}

void BjToolBar::InitView()
{
	mpactionStar = new QAction(this);
	mpactionStop = new QAction(this);
	mpactionStar->setIconText(QStringLiteral("��ʼ"));
	mpactionStop->setIconText(QStringLiteral("ֹͣ"));
	mpactionStar->setEnabled(true);
	mpactionStop->setDisabled(true);
	/*����Ϊ0 ��ʾԴ����*/
	mlableWay.setText(QStringLiteral("�����ʽ:"));
	mcombox.addItem(QStringLiteral("Դ����"));
	mcombox.addItem(QStringLiteral("��������"));
	mcombox.setCurrentIndex(PathManager::GetPackWay());

	
	mlableCustom.setText(QStringLiteral("���ư汾:"));
	mcomboxCustom.addItem(QStringLiteral("�������"));
	mcomboxCustom.addItem(QStringLiteral("��������"));
	mcomboxCustom.setCurrentIndex(PathManager::GetCustomWay());

	this->setFixedHeight(30);
	this->addWidget(&mlableWay);
	this->addWidget(&mcombox);
	this->addSeparator();
	this->addWidget(&mlableCustom);
	this->addWidget(&mcomboxCustom);
	this->addSeparator();

	mlablePath.setText(QStringLiteral("���·��"));
	this->addWidget(&mlablePath);
	mlineedit.setMaximumHeight(23);
	mlineedit.setMaximumWidth(150);
	mlineedit.setText(PathManager::GetOutPath());
	this->addWidget(&mlineedit);
	mbuttonScan.setText(QStringLiteral("���"));
	mbuttonLog.setText(QStringLiteral("��־"));
	mbuttonThreadConfig.setText(QStringLiteral("�߳�����"));
	this->addWidget(&mbuttonScan);
	this->addSeparator();
	this->addAction(mpactionStar);
	this->addSeparator();
	this->addAction(mpactionStop);
	this->addSeparator();
	this->addWidget(&mbuttonLog);
	this->addSeparator();
	this->addWidget(&mbuttonThreadConfig);
}

void BjToolBar::InitSlot()
{
	connect(&mbuttonScan, SIGNAL(clicked()), this, SLOT(ButtonScanClickSlot()));
	connect(&mcombox, SIGNAL(currentIndexChanged(int)), this, SLOT(ComIndexChangSlot(int)));
	connect(&mcomboxCustom, SIGNAL(currentIndexChanged(int)), this, SLOT(CustomComIndexChangSlot(int)));
}

void BjToolBar::ButtonScanClickSlot()
{
	QString defaultPath = PathManager::ReadLastPath(QStringLiteral("toolbar"));
	if (defaultPath.isEmpty()){
		defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	}
	QString	dir = QFileDialog::getExistingDirectory(this, QStringLiteral("ѡ��·��"), defaultPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty()){ 
		PathManager::SetOutPath(dir);
		PathManager::WriteLastPath(QStringLiteral("toolbar"), dir);
		mlineedit.setText(dir);
	}
}

QAction *BjToolBar::GetActionStar()
{
	return mpactionStar;
}

QAction *BjToolBar::GetActionStop()
{
	return mpactionStop;
}

QComboBox *BjToolBar::GetCombox()
{
	return &mcombox;
}

QComboBox *BjToolBar::GetCustomBox()
{
	return &mcomboxCustom;
}

QPushButton *BjToolBar::GetButtonLog()
{
	return &mbuttonLog;
}

QPushButton *BjToolBar::GetButtonThreadConfig()
{
	return &mbuttonThreadConfig;
}

QPushButton *BjToolBar::GetButtonScan()
{
	return &mbuttonScan;
}

void BjToolBar::ComIndexChangSlot(int index)
{
	PathManager::SetPackWay(index);
}

void BjToolBar::CustomComIndexChangSlot(int index)
{
	PathManager::SetCustomWay(index);
}