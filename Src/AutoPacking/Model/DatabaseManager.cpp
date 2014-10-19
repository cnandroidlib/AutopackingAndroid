#include <QFile>
#include <QStandardItemModel>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlField>
#include "DatabaseManager.h"
#include "PathManager.h"
#include "Dialogs/BjMessageBox.h"

DatabaseManager* DatabaseManager::mpinstance = NULL;

DatabaseManager::DatabaseManager() :
QObject(),
mptableModel(NULL),
mptableModelStr(NULL),
mptableModelRes(NULL),
mmodelIndexStr(),
mmodelIndexRes()
{
	InitDataModel();
}

DatabaseManager::~DatabaseManager()
{
	Commit();
	mdatabase.close();
	if (mptableModel != NULL){
		mptableModel->deleteLater();
		mptableModel = NULL;
	}
	if (mptableModelStr != NULL){
		mptableModelStr->deleteLater();
		mptableModelStr = NULL;
	}
	if (mptableModelRes != NULL){
		mptableModelRes->deleteLater();
		mptableModelRes = NULL;
	}
}

DatabaseManager *DatabaseManager::GetInstance()
{
	if (mpinstance == NULL){
		mpinstance = new DatabaseManager;
	}
	return mpinstance;
}

void DatabaseManager::Destroyed()
{
	if (mpinstance != NULL){
		delete mpinstance;
		mpinstance = NULL;
	}
}

void DatabaseManager::InitDataModel()
{
	mdatabase = QSqlDatabase::addDatabase("QSQLITE");
	mdatabase.setDatabaseName(PathManager::GetDataBasePath());
	if (!QFile::exists(PathManager::GetDataBasePath())){
		if (!mdatabase.open()){
			BjMessageBox msg;
			msg.setText(QStringLiteral("���ݿⴴ��ʧ�ܣ�"));
			msg.exec();
			exit(1);
		};
		QSqlQuery query;
		bool isSuccess = query.exec("CREATE TABLE Channeltb ("
			"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
			"ChannelID INTEGER, "
			"ChannelName VARCHAR(256), "
			"ReplaceString VARCHAR(256), "
			"ReplaceRes VARCHAR(256), "
			"PackStatus VARCHAR(80))");

		if (!isSuccess){
			BjMessageBox msg;
			msg.setText(QStringLiteral("����Channeltbʧ�ܣ�"));
			msg.exec();
			mdatabase.close();
			QFile::remove(PathManager::GetDataBasePath());
			exit(1);
		}

		isSuccess = query.exec("CREATE TABLE Stringtb ("
			"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
			"ChanneltbID INTEGER, "
			"Path VARCHAR(256), "
			"SourceString VARCHAR(256), "
			"TargetString VARCHAR(256)) ");
		if (!isSuccess){
			BjMessageBox msg;
			msg.setText(QStringLiteral("����Stringtbʧ�ܣ�"));
			msg.exec();
			mdatabase.close();
			QFile::remove(PathManager::GetDataBasePath());
			exit(1);
			return;
		}

		isSuccess = query.exec("CREATE TABLE Resourcetb ("
			"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
			"ChanneltbID INTEGER, "
			"SourceResDir VARCHAR(256), "
			"TargetResDir VARCHAR(256)) ");
		if (!isSuccess){
			BjMessageBox msg;
			msg.setText(QStringLiteral("����Resourcetbʧ�ܣ�"));
			msg.exec();
			mdatabase.close();
			QFile::remove(PathManager::GetDataBasePath());
			exit(1);
			return;
		}
	}
	else{
		if (!mdatabase.open()){
			BjMessageBox msg;
			msg.setText(QStringLiteral("���ݿ��ʧ�ܣ�"));
			msg.exec();
			QFile::remove(PathManager::GetDataBasePath());
			exit(1);
		}
	}

	mptableModel = new BjTableModel(NULL, mdatabase);
	mptableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
	mptableModel->setTable("Channeltb");
	if (!mptableModel->select()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModel->lastError().text()), QMessageBox::Ok, QMessageBox::NoButton);
		mptableModel->revertAll();//�����ɾ��������
		return;
	}
	mptableModel->setHeaderData(1, Qt::Horizontal, QStringLiteral("����ID"));
	mptableModel->setHeaderData(2, Qt::Horizontal, QStringLiteral("��������"));
	mptableModel->setHeaderData(3, Qt::Horizontal, QStringLiteral("�༭�滻�ַ���"));
	mptableModel->setHeaderData(4, Qt::Horizontal, QStringLiteral("�༭�滻��Դ"));
	mptableModel->setHeaderData(5, Qt::Horizontal, QStringLiteral("״̬"));


	mptableModelStr = new BjTableModel(NULL, mdatabase);
	mptableModelStr->setEditStrategy(QSqlTableModel::OnManualSubmit);
	mptableModelStr->setTable("Stringtb");
	if (!mptableModelStr->select()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModelStr->lastError().text()), QMessageBox::Ok, QMessageBox::NoButton);
		mptableModelStr->revertAll();//�����ɾ��������
		return;
	}
	mptableModelStr->setHeaderData(2, Qt::Horizontal, QStringLiteral("�ļ����·��"));
	mptableModelStr->setHeaderData(3, Qt::Horizontal, QStringLiteral("ԭ�ڵ�"));
	mptableModelStr->setHeaderData(4, Qt::Horizontal, QStringLiteral("�滻��ڵ�"));

	mptableModelRes = new BjTableModel(NULL, mdatabase);
	mptableModelRes->setEditStrategy(QSqlTableModel::OnManualSubmit);
	mptableModelRes->setTable("Resourcetb");
	if (!mptableModelRes->select()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModelRes->lastError().text()), QMessageBox::Ok, QMessageBox::NoButton);
		mptableModelRes->revertAll();//�����ɾ��������
		return;
	}
	mptableModelRes->setHeaderData(2, Qt::Horizontal, QStringLiteral("ԭ��Դ����Ŀ¼"));
	mptableModelRes->setHeaderData(3, Qt::Horizontal, QStringLiteral("�滻��Դ����Ŀ¼"));
}

void DatabaseManager::AddRow()
{
	int rowNum = mptableModel->rowCount();//��ñ������
	QSqlRecord record = mptableModel->record();
	QSqlField fieldStr("ReplaceString", QVariant::Char);
	QSqlField fieldRes("ReplaceRes", QVariant::Char);
	fieldStr.setValue(QStringLiteral("˫���༭"));
	fieldRes.setValue(QStringLiteral("˫���༭"));
	record.append(fieldStr);
	record.append(fieldRes);

	if (!mptableModel->insertRecord(rowNum, record)){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModel->lastError().text()), QMessageBox::Ok, QMessageBox::NoButton);
		mptableModel->revertAll();//�����ɾ��������
		return;
	}

	if (!mptableModel->submitAll()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModel->lastError().text()));
		mptableModel->revertAll();//�����ɾ��������
	}
}

void DatabaseManager::DelRow(QModelIndexList &selecteds)
{
	foreach(QModelIndex index, selecteds)
	{
		int curRow = index.row(); //ɾ�����б�ѡ�е���		
		if (!mptableModel->removeRow(curRow)){
			BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModel->lastError().text()));
			mptableModel->revertAll();//�����ɾ��������
			return;
		}
		QString id = mptableModel->record(curRow).value("ID").toString();
		mptableModelStr->setFilter(QString("ChanneltbID=\'%1\'").arg(id));
		if (!mptableModelStr->select()){
			BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModelStr->lastError().text()));
			mptableModelStr->revertAll();//�����ɾ��������
		}
		DeleteAll(*mptableModelStr);

		mptableModelRes->setFilter(QString("ChanneltbID=\'%1\'").arg(id));
		if (!mptableModelRes->select()){
			BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModelRes->lastError().text()));
			mptableModelRes->revertAll();//�����ɾ��������
		}
		DeleteAll(*mptableModelRes);
	}

	if (!mptableModel->submitAll()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModelStr->lastError().text()));
		mptableModel->revertAll();//�����ɾ��������
	}
}

void DatabaseManager::AddRowStr()
{
	int rowNum = mptableModelStr->rowCount();//��ñ������
	QString id = mptableModel->record(mmodelIndexStr.row()).value("ID").toString();
	QSqlRecord record = mptableModelStr->record();
	QSqlField fieldChannltbId("ChanneltbID", QVariant::Int);
	fieldChannltbId.setValue(id);
	record.append(fieldChannltbId);

	if (!mptableModelStr->insertRecord(rowNum, record)){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModelStr->lastError().text()), QMessageBox::Ok, QMessageBox::NoButton);
		mptableModelStr->revertAll();//�����ɾ��������
		return;
	}

	if (!mptableModelStr->submitAll()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModelStr->lastError().text()));
		mptableModelStr->revertAll();//�����ɾ��������
	}

}

void DatabaseManager::DelRowStr(QModelIndexList &selecteds)
{
	foreach(QModelIndex index, selecteds)
	{
		int curRow = index.row(); //ɾ�����б�ѡ�е���		
		if (!mptableModelStr->removeRow(curRow)){
			BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModelStr->lastError().text()));
			mptableModelStr->revertAll();//�����ɾ��������
			return;
		}
	}

	if (!mptableModelStr->submitAll()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModelStr->lastError().text()));
		mptableModelStr->revertAll();//�����ɾ��������
	}
}

void DatabaseManager::AddRowRes()
{
	int rowNum = mptableModelRes->rowCount();//��ñ������
	QString id = mptableModel->record(mmodelIndexRes.row()).value("ID").toString();
	QSqlRecord record = mptableModelRes->record();
	QSqlField fieldChannltbId("ChanneltbID", QVariant::Int);
	fieldChannltbId.setValue(id);
	record.append(fieldChannltbId);


	if (!mptableModelRes->insertRecord(rowNum, record)){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModelRes->lastError().text()), QMessageBox::Ok, QMessageBox::NoButton);
		mptableModelRes->revertAll();//�����ɾ��������
		return;
	}

	if (!mptableModelRes->submitAll()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModelRes->lastError().text()));
		mptableModelRes->revertAll();//�����ɾ��������
	}
}

void DatabaseManager::DelRowRes(QModelIndexList &selecteds)
{
	foreach(QModelIndex index, selecteds)
	{
		int curRow = index.row(); //ɾ�����б�ѡ�е���		
		if (!mptableModelRes->removeRow(curRow)){
			BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModelRes->lastError().text()));
			mptableModelRes->revertAll();//�����ɾ��������
			return;
		}
	}

	if (!mptableModelRes->submitAll()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModelRes->lastError().text()));
		mptableModelRes->revertAll();//�����ɾ��������
	}
}

void DatabaseManager::DeleteAll(QSqlTableModel &model)
{
	int rowNum = model.rowCount();
	if (rowNum == 0){
		return;
	}

	if (!model.removeRows(0, rowNum)){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(model.lastError().text()));
		model.revertAll();//�����ɾ��������
	 }

	if (!model.submitAll()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(model.lastError().text()));
		model.revertAll();//�����ɾ��������
	}
}

bool DatabaseManager::isDatabaseEmpty()
{
	if (mptableModel->rowCount() == 0){
		return true;
	}
	return false;
}

BjTableModel *DatabaseManager::GetTableModel()
{
	return mptableModel;
}

QSqlDatabase *DatabaseManager::GetDatabase()
{
	return &mdatabase;
}

BjTableModel *DatabaseManager::GetTableModelStr(const QModelIndex &modelIndexStr)
{
	mmodelIndexStr = modelIndexStr;
	QString id = mptableModel->record(modelIndexStr.row()).value("ID").toString();
	mptableModelStr->setFilter(QString("ChanneltbID=\'%1\'").arg(id));
	if (!mptableModelStr->select()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModelRes->lastError().text()), QMessageBox::Ok, QMessageBox::NoButton);
		mptableModel->revertAll();//�����ɾ��������
	}
	return mptableModelStr;
}

BjTableModel *DatabaseManager::GetTableModelRes(const QModelIndex &modelIndexRes)
{
	mmodelIndexRes = modelIndexRes;
	QString id = mptableModel->record(mmodelIndexRes.row()).value("ID").toString();
	mptableModelRes->setFilter(QString("ChanneltbID=\'%1\'").arg(id));
	if (!mptableModelRes->select()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModelRes->lastError().text()), QMessageBox::Ok, QMessageBox::NoButton);
		mptableModel->revertAll();//�����ɾ��������
	}
	return mptableModelRes;
}

BjTableModel *DatabaseManager::GetTableModelStr()
{
	return mptableModelStr;
}

BjTableModel *DatabaseManager::GetTableModelRes()
{
	return mptableModelRes;
}

void DatabaseManager::ChangStatInDatabase(int row, QString &status)
{
	QSqlRecord record = mptableModel->record(row);
	record.setValue("PackStatus", status);
	if (!mptableModel->setRecord(row, record)){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModel->lastError().text()));
		mptableModel->revertAll();//�����ɾ��������
	}
	if (!mptableModel->submitAll()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1").arg(mptableModel->lastError().text()));
		mptableModel->revertAll();//�����ɾ��������
	}
}

void DatabaseManager::ReadyData(QString &id, QList<ReplaceStrTable> &strTable, QList<ReplaceResTable>  &resTable)
{
	mptableModelStr->setFilter(QString("ChanneltbID=\'%1\'").arg(id));
	int rowNum = mptableModelStr->rowCount();
	for (int i = 0; i < rowNum; i++)
	{
		QString path = mptableModelStr->record(i).value("Path").toString().trimmed();
		QString srcStr = mptableModelStr->record(i).value("SourceString").toString();
		QString destStr = mptableModelStr->record(i).value("TargetString").toString();
		ReplaceStrTable reStrTable;
		reStrTable.SetFolder(path);
		reStrTable.SetSrcStr(srcStr);
		reStrTable.SetDestStr(destStr);
		strTable.push_back(reStrTable);
	}

	mptableModelRes->setFilter(QString("ChanneltbID=\'%1\'").arg(id));
	rowNum = mptableModelRes->rowCount();
	for (int i = 0; i < rowNum; i++)
	{
		QString srcDir = mptableModelRes->record(i).value("SourceResDir").toString().trimmed();
		QString destDir = mptableModelRes->record(i).value("TargetResDir").toString().trimmed();
		ReplaceResTable reResTable;
		reResTable.SetFolderSrc(srcDir);
		reResTable.SetFolderDest(destDir);
		resTable.push_back(reResTable);
	}
}

void DatabaseManager::ExportData(QString &fileName)
{
	Commit();
	mdatabase.close();
	if (!PathManager::CopyFile(PathManager::GetDataBasePath(), fileName, true)){
		BjMessageBox::warning(NULL, QStringLiteral("�������ݳ���"), QStringLiteral("�������ݴ���!"), QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}
	BjMessageBox::information(NULL, QStringLiteral("�����ɹ���"), QStringLiteral("�����ɹ���"), QMessageBox::Ok, QMessageBox::NoButton);
	ReloadData();
}

void DatabaseManager::ImportData(QString &fileName)
{
	Commit();
	mdatabase.close();
	if (!PathManager::CopyFile(fileName,PathManager::GetDataBasePath(), true)){
		BjMessageBox::warning(NULL, QStringLiteral("�������ݳ���"), QStringLiteral("�������ݴ���!"), QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}
	BjMessageBox::information(NULL, QStringLiteral("����ɹ���"), QStringLiteral("����ɹ���"), QMessageBox::Ok, QMessageBox::NoButton);
	ReloadData();
}

void DatabaseManager::ReloadData()
{
	mdatabase.close();
	if (mptableModel != NULL){
		mptableModel->deleteLater();
		mptableModel = NULL;
	}
	if (mptableModelStr != NULL){
		mptableModelStr->deleteLater();
		mptableModelStr = NULL;
	}
	if (mptableModelRes != NULL){
		mptableModelRes->deleteLater();
		mptableModelRes = NULL;
	}
	if (!mdatabase.open()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ�򿪳���"), QStringLiteral("���ݿ��ʧ��!"), QMessageBox::Ok, QMessageBox::NoButton);
	}
	InitDataModel();
	emit ReloadDataSignal();
}

void DatabaseManager::Commit()
{
	if (!mdatabase.transaction()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("��ʼ���������!"), QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}
	if (!mptableModel->submitAll() || !mptableModelStr->submitAll() || !mptableModelRes->submitAll()){
		BjMessageBox::warning(NULL, QStringLiteral("���ݿ����"), QStringLiteral("���ݿ����: %1 %2 %3")
			.arg(mptableModel->lastError().text())
			.arg(mptableModelStr->lastError().text())
			.arg(mptableModelRes->lastError().text()));
		mdatabase.rollback();//�ع�
		return;
	}
	mdatabase.commit();//�ύ
}