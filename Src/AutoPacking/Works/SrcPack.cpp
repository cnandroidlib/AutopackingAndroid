#include <QDir>
#include "SrcPack.h"
#include "Model/PathManager.h"

SrcPack::SrcPack(QObject *parent):
Pack(parent)
{
	mpprocess = new QProcess(this);
}

SrcPack::~SrcPack()
{
	mpprocess->deleteLater();
}

void SrcPack::Start(QString &inPath, QString &outPath, QString &channelId, QString &channelName, QString &channeltbID,
	QList<ReplaceStrTable> &strTableList, QList<ReplaceResTable> &resTableList, QList<ReplacePakTable> &pakTableList, int taskId)
{
	if (inPath.isEmpty() || outPath.isEmpty()){
		emit GenerateError(QStringLiteral("error:�������·��Ϊ�գ�����ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		emit FinishSignal(1, mtaskId);
		return;
	}

	if (channelId.isEmpty() && channelName.isEmpty()){
		emit GenerateError(QStringLiteral("error:����id����������Ϊ�գ�����ID: %1, ������ : %2\n").arg(mchannelId).arg(mchannelName));
		emit FinishSignal(1, mtaskId);
		return;
	}

	mchannelId = channelId;
	mchannelName = channelName;
	mtaskId = taskId;
	mstrTableList = strTableList;
	mresTableList = resTableList;
	mpakTableList = pakTableList;
	CreatPath(outPath, channelId, channelName, channeltbID);
	if (!CopySrc(PathManager::GetSrcPath(), mtmpSrcPath)){
		emit GenerateError(QStringLiteral("error:����Դ���ļ�ʧ�ܣ�����ID: %1, ������ : %2\n").arg(mchannelId).arg(mchannelName));
		emit FinishSignal(1, mtaskId);
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		return;
	}

	QFile buildXml(mtmpSrcPath + QStringLiteral("/build.xml"));
	if (buildXml.exists() && !buildXml.remove()){
		emit GenerateError(QStringLiteral("error:ɾ��ԭbuld.xml�ļ�ʧ�ܣ�����ID: %1, ������ : %2\n").arg(mchannelId).arg(mchannelName));
		emit FinishSignal(1, mtaskId);
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		return;
	}
	PrePack();
}

void SrcPack::Stop()
{
	if (mpprocess->state() == QProcess::Running){
		mpprocess->close();
	}
	if (!PathManager::RemoveDir(mtmpPath)){
		emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
	}
	emit FinishSignal(2, mtaskId);
}

void SrcPack::CreatPath(QString &outPath, QString &channelId, QString &channelName, QString &channeltbId)
{
	if (outPath.endsWith("/")){
		moutFile = outPath + channelName + "_" + channelId + "_" + PathManager::GetVersion() + ".apk";
	}
	else{
		moutFile = outPath + "/" + channelName + "_" + channelId + "_" + PathManager::GetVersion() + ".apk";
	}

	mtmpPath = PathManager::GetTmpPath() + QStringLiteral("/") + channeltbId;
	QString srcPath = mtmpPath + QStringLiteral("/src");
	QString signPath = mtmpPath + QStringLiteral("/sign");
	PathManager::CreatDir(srcPath);
	PathManager::CreatDir(signPath);
	mtmpSrcPath = srcPath;
	mtmpSignFile = signPath + "/" + channelName + "_" + channelId + "_" + PathManager::GetVersion() + ".apk";
}

bool SrcPack::CopySrc(QString &srcPath, QString &destPath)
{
	QDir dir(destPath);
	if (dir.exists()){
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:����ϴλ����������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
			return false;
		}
	}
	return PathManager::CopyDir(srcPath, destPath,true);
}

bool SrcPack::ReplacePakByTable()
{
	for (QList<ReplacePakTable>::iterator ite = mpakTableList.begin(); ite != mpakTableList.end(); ite++)
	{
		if (!PathManager::ReplacePak(mtmpSrcPath, ite->GetSrcPakName(), ite->GetDestPakName())){
			emit GenerateError(QStringLiteral("error:�滻������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
			return false;
		}
	}
	return true;
}

void SrcPack::PrePack()
{
	connect(mpprocess, SIGNAL(finished(int)), this, SLOT(PrePackFinishedSlot(int)));
	QString target = PathManager::GetTarget(mtmpSrcPath);
	if (target.isEmpty()){
		emit GenerateError(QStringLiteral("error:δ�ҵ�project.properties�ļ�����δָ��target"));
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		return;
	}
	QString prepackBat = PathManager::GetPrePack();
	QStringList param;
	target = target.remove("").remove("target=android-");
	param << PathManager::GetAndroid() << PathManager::GetSdkToolsPath() << QString::number(target.toInt() - 13) << mtmpSrcPath;
	QString content = QStringLiteral("\nkey.store=%1\nkey.alias=%2\nkey.store.password=%3\nkey.alias.password=%4\n")
		.arg(PathManager::GetKeyPath())
		.arg(PathManager::GetKeyAliases())
		.arg(PathManager::GetPasswd())
		.arg(PathManager::GetAliasesPasswd());
	PathManager::AppendContentToProperties(content, mtmpSrcPath);

	if (!Pack::ReplaceStrByTable(mtmpPath)){
		emit FinishSignal(1, mtaskId);
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		return;
	}

	if (!Pack::ReplaceResByTable(mtmpPath)){
		emit FinishSignal(1, mtaskId);
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		return;
	}

	if (!ReplacePakByTable()){
		emit FinishSignal(1, mtaskId);
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		return;
	}
	ExecuteCmd(prepackBat, param, PathManager::GetSdkPath());
}

void SrcPack::PackFromSrc()
{
	connect(mpprocess, SIGNAL(finished(int)), this, SLOT(PackFromSrcFinishedSlot(int)));
	QString srcpackBat = PathManager::GetSrcPack();
	QStringList param;
	param << PathManager::GetAnt() << mtmpSrcPath;
	ExecuteCmd(srcpackBat, param, PathManager::GetAntPath());
}

void SrcPack::PrePackFinishedSlot(int stat)
{
	disconnect(mpprocess, SIGNAL(finished(int)), this, SLOT(PrePackFinishedSlot(int)));
	if (!CheckError()){
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		emit FinishSignal(1, mtaskId);
		return;
	}
	PackFromSrc();
}

void SrcPack::PackFromSrcFinishedSlot(int stat)
{
	disconnect(mpprocess, SIGNAL(finished(int)), this, SLOT(PackFromSrcFinishedSlot(int)));
	if (!CheckError()){
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		emit FinishSignal(1, mtaskId);
		return;
	}
	QString apk = PathManager::GetReleaseApk(PathManager::GetBin(mtmpSrcPath));
	if (apk.isEmpty()){
		emit GenerateError(QStringLiteral("error:������apk�ļ�δ�ҵ�������ID: %1, ������ : %2\n").arg(mchannelId).arg(mchannelName));
		emit FinishSignal(1, mtaskId);
		return;
	}
	if (!QFile::copy(PathManager::GetBin(mtmpSrcPath) + QString("/") + apk, moutFile)){
		emit GenerateError(QStringLiteral("error:����ǩ���ļ���������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		emit FinishSignal(1, mtaskId);
		return;
	}

	if (!PathManager::RemoveDir(mtmpPath)){
		emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
	}

	emit FinishSignal(0, mtaskId);
}