#include <QtCore/QProcess>
#include "CHelp.h"

/* FUNCTION *******************************************************************/
/**
@return Singleton
*******************************************************************************/
CHelp& help()
{
	static CHelp help;
	return help;
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
CHelp::CHelp()
	: m_Process(new QProcess())
{
}

/* METHOD *********************************************************************/
/**
*******************************************************************************/
CHelp::~CHelp()
{
	if (m_Process && m_Process->state() == QProcess::Running)
	{
		m_Process->terminate();
		m_Process->waitForFinished(3000);
	}
	delete m_Process;
}

/* METHOD *********************************************************************/
/**
@param :
*******************************************************************************/
void CHelp::show(const QString& url)
{
	if (!m_Process)
	{
		return;
	}
	if (m_Process->state() != QProcess::Running)
	{
		QStringList args;
		args
			<< QLatin1String("-enableRemoteControl")
			<< QLatin1String("-collectionFile")
			<< QLatin1String("Kanon.qhc");
		m_Process->start(QLatin1String("assistant"), args);
		if (!m_Process->waitForStarted())
		{
			delete m_Process;
			m_Process = nullptr;
		}
	}
	if (m_Process && m_Process->state() == QProcess::Running)
	{
		QByteArray ba;
		ba.append("setSource " + url + "\n");
		m_Process->write(ba);
	}
}

