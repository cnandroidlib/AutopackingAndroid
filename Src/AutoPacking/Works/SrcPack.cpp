#include <QDir>
#include "SrcPack.h"
#include "Model/PathManager.h"

SrcPack::SrcPack(QObject *parent):
Pack(parent)
{
}

SrcPack::~SrcPack()
{
}

void SrcPack::Init(QString &inPath, QString &outPath, QString &channelId, QString &channelName, QString &channeltbID,
	QList<ReplaceStrTable> &strTableList, QList<ReplaceResTable> &resTableList, QList<ReplacePakTable> &pakTableList, QList<ReplaceAppPakTable> &appPakTableList, int taskId)
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
	mappPakTableList = appPakTableList;
	moutputPath = outPath;
	mchanneltbId = channeltbID;
}

void SrcPack::run()
{
	mpprocess = new QProcess(NULL);
	if (!CreatPath(moutputPath, mchannelId, mchannelName, mchanneltbId)){
		mpprocess->close();
		delete mpprocess;
		mpprocess = NULL;
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:����ϴλ����������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		emit FinishSignal(1, mtaskId);
		return;
	}

	if (!CopySrc(PathManager::GetSrcPath(), mtmpSrcPath)){
		emit GenerateError(QStringLiteral("error:����Դ���ļ�ʧ�ܣ�����ID: %1, ������ : %2\n").arg(mchannelId).arg(mchannelName));
		mpprocess->close();
		delete mpprocess;
		mpprocess = NULL;
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		emit FinishSignal(1, mtaskId);
		return;
	}

	QFile buildXml(mtmpSrcPath + QStringLiteral("/build.xml"));
	if (buildXml.exists() && !buildXml.remove()){
		emit GenerateError(QStringLiteral("error:ɾ��ԭbuld.xml�ļ�ʧ�ܣ�����ID: %1, ������ : %2\n").arg(mchannelId).arg(mchannelName));
		mpprocess->close();
		delete mpprocess;
		mpprocess = NULL;
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		emit FinishSignal(1, mtaskId);
		return;
	}

	if (!PrePack(*mpprocess)){
		mpprocess->close();
		delete mpprocess;
		mpprocess = NULL;
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		emit FinishSignal(1, mtaskId);
		return;
	}

	if (!CheckError(*mpprocess)){
		mpprocess->close();
		delete mpprocess;
		mpprocess = NULL;
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		emit FinishSignal(1, mtaskId);
		return;
	}

	if (!PackFromSrc(*mpprocess)){
		mpprocess->close();
		delete mpprocess;
		mpprocess = NULL;
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		emit FinishSignal(1, mtaskId);
		return;
	}

	if (!CheckError(*mpprocess)){
		mpprocess->close();
		delete mpprocess;
		mpprocess = NULL;
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		emit FinishSignal(1, mtaskId);
		return;
	}

	QString apk = PathManager::GetReleaseApk(PathManager::GetBin(mtmpSrcPath));
	if (apk.isEmpty()){
		mpprocess->close();
		delete mpprocess;
		mpprocess = NULL;
		emit GenerateError(QStringLiteral("error:������apk�ļ�δ�ҵ�������ID: %1, ������: %2\n").arg(mchannelId).arg(mchannelName));
		emit FinishSignal(1, mtaskId);
		return;
	}
	if (!PathManager::CopyFile(PathManager::GetBin(mtmpSrcPath) + QString("/") + apk, moutFile, true)){
		mpprocess->close();
		delete mpprocess;
		mpprocess = NULL;
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
	mpprocess->close();
	delete mpprocess;
	mpprocess = NULL;
	emit FinishSignal(0, mtaskId);
}

bool SrcPack::CreatPath(QString &outPath, QString &channelId, QString &channelName, QString &channeltbId)
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

	QDir tmpDir(srcPath);
	if (tmpDir.exists(srcPath)){
		if (!PathManager::RemoveDir(srcPath)){
			return false;
		}
	}

	if (tmpDir.exists(signPath)){
		if (!PathManager::RemoveDir(signPath)){
			return false;
		}
	}

	PathManager::CreatDir(srcPath);
	PathManager::CreatDir(signPath);
	mtmpSrcPath = srcPath;
	mtmpSignFile = signPath + "/" + channelName + "_" + PathManager::GetVersion() + channelId + "_" + ".apk";
	return true;
}

bool SrcPack::CopySrc(QString &srcPath, QString &destPath)
{
	if (!PathManager::CopyDir(srcPath, destPath, true)){
		return false;
	}
	if (!PathManager::RemoveDir(destPath + "/bin")){
		return false;
	}
	return true;
}

bool SrcPack::ReplacePakByTable()
{
	for (QList<ReplacePakTable>::iterator ite = mpakTableList.begin(); ite != mpakTableList.end(); ite++)
	{
		switch (PathManager::ReplacePakInSrc(mtmpSrcPath, ite->GetSrcPakName(), ite->GetDestPakName()))
		{
		case 0:
			break;
		case 1:
			emit GenerateError(QStringLiteral("error:�滻��������,ԭ���������ڣ�����ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
			return false;
		case 2:
			emit GenerateError(QStringLiteral("error:�滻��������,��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
			return false;
		case 3:
			emit GenerateError(QStringLiteral("error:�滻��������,Ŀ�İ����Ѿ����ڣ�����ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
			return false;
		case 4:
			emit GenerateError(QStringLiteral("error:�滻��������,�滻�������̳�������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
			return false;
		}
	}
	return true;
}

bool SrcPack::ReplaceAppPakByTable()
{
	for (QList<ReplaceAppPakTable>::iterator ite = mappPakTableList.begin(); ite != mappPakTableList.end(); ite++)
	{
		switch (PathManager::ReplaceAppPakInSrc(mtmpSrcPath, ite->GetSrcPakName(), ite->GetDestPakName()))
		{
		case 0:
			break;
		case 1:
			emit GenerateError(QStringLiteral("error:�滻��������,ԭ���������ڣ�����ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
			return false;
		case 2:
			emit GenerateError(QStringLiteral("error:�滻��������,��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
			return false;
		case 3:
			emit GenerateError(QStringLiteral("error:�滻��������,Ŀ�İ����Ѿ����ڣ�����ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
			return false;
		case 4:
			emit GenerateError(QStringLiteral("error:�滻��������,�滻�������̳�������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
			return false;
		case 5:
			emit GenerateError(QStringLiteral("error:�滻App��������,�滻�������̳�������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
			return false;
		}
	}
	return true;
}

bool SrcPack::PrePack(QProcess &pprocess)
{
	QString target = PathManager::GetTarget(mtmpSrcPath);
	if (target.isEmpty()){
		emit GenerateError(QStringLiteral("error:δ�ҵ�project.properties�ļ�����δָ��target"));
		return false;
	}
	QString prepackBat = QStringLiteral("prepack.bat");
	QStringList param;
	target = target.remove(" ").remove("target=android-");
	param << "\"" + PathManager::GetAndroid() + "\"" << "\"" + PathManager::GetSdkToolsPath() + "\"" << QString::number(target.toInt() - 13) << "\"" + mtmpSrcPath + "\"";
	QString content = QStringLiteral("\nkey.store=%1\nkey.alias=%2\nkey.store.password=%3\nkey.alias.password=%4\n")
		.arg(PathManager::GetKeyPath())
		.arg(PathManager::GetKeyAliases())
		.arg(PathManager::GetPasswd())
		.arg(PathManager::GetAliasesPasswd());
	PathManager::AppendContentToProperties(content, mtmpSrcPath);

	if (!Pack::ReplaceStrByTable(mtmpSrcPath)){
		return false;
	}

	if (!Pack::ReplaceResByTable(mtmpSrcPath)){
		return false;
	}

	if (!ReplacePakByTable()){
		return false;
	}

	if (!ReplaceAppPakByTable()){
		return false;
	}

	if (!ExecuteCmd(prepackBat, param, pprocess,PathManager::GetToolPath())){
		emit GenerateError(QStringLiteral("error:����ִ�д�������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		return false;
	}
	return true;
}

bool SrcPack::PackFromSrc(QProcess &pprocess)
{
	QString srcpackBat = QStringLiteral("srcpack.bat");
	QStringList param;
	param << "\"" + PathManager::GetAnt() + "\"" << mtmpSrcPath;
	if (!ExecuteCmd(srcpackBat, param, pprocess,PathManager::GetToolPath())){
		emit GenerateError(QStringLiteral("error:����ִ�д�������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		return false;
	}
	return true;
}