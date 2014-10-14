#include "QStream.h"
#include <QList>
#include <QVariant>
#include <mqueue.h>
#include <stdio.h>

#define P0 "/70"
#define P1 "/71"
#define P2 "/72"
#define P3 "/73"
#define P4 "/74"
#define P5 "/75"
#define P6 "/76"

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
	int pid = -1; //PID 
	float temp; //inital temperature from command line
	//parses command line arguments and updates pid and initial temperature/exits on fail
	if(!getArgumentValues(parseArguments(argc, argv), pid, temp)) exit(0);
	
	mqd_t mailbox; /*message queue*/
	mqd_t parent;
	mqd_t child_1;
	mqd_t child_2;
	struct mq_attr ma;
	ma.mq_flags = 0;
	ma.mq_maxmsg = 10;
	ma.mq_msgsize = 33;
	ma.mq_curmsgs = 0;

	/*char send[sizeof(float)];
	char receive[40];
	float *f_receive = (float*)receive;
	f_receive = 5.0;*/
	char send[40];
	float *f_send = (float*)send;
	*f_send = temp;
	char receive[40];
	float *f_receive = (float*)receive;

	switch(pid){
		case 0:
			mailbox = mq_open(P0, O_CREAT|O_RDWR, 0644, &ma);
			mq_send(mailbox, send, sizeof(float)+1, 1);
			//qout << ma.mq_curmsgs;
			mq_receive(mailbox, receive, 50, NULL);
			mq_close(mailbox);
			mq_unlink(P0);
			qout << f_receive[0];
			break;
		
	}
	return 0;
}