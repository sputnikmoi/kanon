#ifndef CHELP_H
#define CHELP_H

class QProcess;

/* CLASS DECLARATION **********************************************************/
/**
  Controls QAssistant (context sensitive help).
*******************************************************************************/
class CHelp
{
	QProcess* m_Process;
public:
	CHelp();
	~CHelp();
	void show(const QString& url);
};

// Singleton
CHelp& help();

#endif

