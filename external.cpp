#include "QStream.h"
#include <QList>
#include <QVariant>

QList<QString> parseArguments(int argc, char *argv[])
{
	QList<QString> arguments;
	for(int i = 0; i < argc; i++)
	{
		arguments.append(argv[i]);
	}
	arguments.takeFirst();
	return arguments;
}

bool getArgumentValues(QList<QString> arguments,int &pid, float &initialTemperature)
{
	bool isSet = true;
	bool isValidFloat = false;
	if(arguments[0] == "0" || (arguments[0].toInt() >=1 && arguments[0].toInt() <= 6))
		pid = arguments[0].toInt();
	else 
		{
			qout << "You did not enter a valid PID." << endl;
			isSet = false;
		}

	arguments[1].toFloat(&isValidFloat);
	if(isValidFloat) initialTemperature = arguments[1].toFloat();
	else 
		{
			qout << "You did not enter a valid temperature." << endl;
			isSet = false;
		}
	return isSet;
}

int main(int argc, char *argv[])
{
	int pid = -1;
	float initialTemperature;
	if(!getArgumentValues(parseArguments(argc, argv), pid, initialTemperature)) exit(0);
	return 0;
}