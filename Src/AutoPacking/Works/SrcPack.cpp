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
	moutputPath = outPath;
	mchanneltbId = channeltbID;
}

void SrcPack::run()
{
	QProcess *pprocess= new QProcess(NULL);
	CreatPath(moutputPath, mchannelId, mchannelName, mchanneltbId);
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

	if (!PrePack(*pprocess)){
		return;
	}

	if (!CheckError(*pprocess)){
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		emit FinishSignal(1, mtaskId);
		return;
	}

	if (!PackFromSrc(*pprocess)){
		return;
	}

	if (!CheckError(*pprocess)){
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		emit FinishSignal(1, mtaskId);
		return;
	}

	QString apk = PathManager::GetReleaseApk(PathManager::GetBin(mtmpSrcPath));
	if (apk.isEmpty()){
		emit GenerateError(QStringLiteral("error:������apk�ļ�δ�ҵ�������ID: %1, ������: %2\n").arg(mchannelId).arg(mchannelName));
		emit FinishSignal(1, mtaskId);
		return;
	}
	if (!PathManager::CopyFile(PathManager::GetBin(mtmpSrcPath) + QString("/") + apk, moutFile,true)){
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

void SrcPack::Stop()
{
	this->terminate();
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

bool SrcPack::PrePack(QProcess &pprocess)
{
	QString target = PathManager::GetTarget(mtmpSrcPath);
	if (target.isEmpty()){
		emit GenerateError(QStringLiteral("error:δ�ҵ�project.properties�ļ�����δָ��target"));
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		return false;
	}
	QString prepackBat = QStringLiteral("prepack.bat");
	QStringList param;
	target = target.remove("").remove("target=android-");
	param << "\"" + PathManager::GetAndroid() + "\"" << "\"" + PathManager::GetSdkToolsPath() + "\"" << QString::number(target.toInt() - 13) << "\"" + mtmpSrcPath + "\"";
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
		return false;
	}

	if (!Pack::ReplaceResByTable(mtmpPath)){
		emit FinishSignal(1, mtaskId);
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		return false;
	}

	if (!ReplacePakByTable()){
		emit FinishSignal(1, mtaskId);
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		return false;
	}
	if (!ExecuteCmd(prepackBat, param, pprocess,PathManager::GetToolPath())){
		emit GenerateError(QStringLiteral("error:����ִ�д�������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		emit FinishSignal(1, mtaskId);
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
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
		emit FinishSignal(1, mtaskId);
		if (!PathManager::RemoveDir(mtmpPath)){
			emit GenerateError(QStringLiteral("error:��������������ID:%1,������:%2\n").arg(mchannelId).arg(mchannelName));
		}
		return false;
	}
	return true;
}