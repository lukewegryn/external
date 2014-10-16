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

QList<QString> parseArguments(int argc, char *argv[]);
bool getArgumentValues(QList<QString> arguments,int &pid, float &initialTemperature);
void sendFloat(mqd_t name, float f, float pid);
float receiveFloat(mqd_t mailbox, float &pid);
mqd_t openMailbox(const char *name, mq_attr &ma);
bool hasMessages(mqd_t mqueue);
int numMessages(mqd_t mqueue);
int main(int argc, char *argv[])
{
	int pid = -1; //PID 
	float temp = 0; //inital temperature from command line
	float downTemp = 0;
	float parentTemp = 0;
	float upTemp = 0;
	//parses command line arguments and updates pid and initial temperature/exits on fail
	if(!getArgumentValues(parseArguments(argc, argv), pid, temp)) exit(0);
	
	mqd_t mqueue; /*message queue*/
	mqd_t parent;
	mqd_t child_1;
	mqd_t child_2;
	struct mq_attr ma;
	ma.mq_flags = 0;
	ma.mq_maxmsg = 10;
	ma.mq_msgsize = 33;
	ma.mq_curmsgs = 0;

	float child1Value;
	float child2Value;
	//sendFloat(mqueue, temp);
			//qout << receiveFloat(mqueue);
			//mq_close(mqueue);
			//mq_unlink(P0);
	switch(pid){
		case 0:
		{
			mqueue = openMailbox(P0, ma);
			child_1 = openMailbox(P1, ma);
			child_2 = openMailbox(P2, ma);
			sendFloat(child_1, temp, 0.0); //false flags as sent down
			sendFloat(child_2, temp, 0.0);
			break;
		}
		case 1:
			parent = openMailbox(P0, ma);
			mqueue = openMailbox(P1, ma);
			child_1 = openMailbox(P3, ma);
			child_2 = openMailbox(P4, ma);
			break;
		case 2:
			parent = openMailbox(P0, ma);
			mqueue = openMailbox(P2, ma);
			child_1 = openMailbox(P5, ma);
			child_2 = openMailbox(P6, ma);
			break;
		case 3:
			parent = openMailbox(P1, ma);
			mqueue = openMailbox(P3, ma);
			break;
		case 4:
			parent = openMailbox(P1, ma);
			mqueue = openMailbox(P4, ma);
			break;
		case 5:
			parent = openMailbox(P2, ma);
			mqueue = openMailbox(P5, ma);
			break;
		case 6:
			parent = openMailbox(P2, ma);
			mqueue = openMailbox(P6, ma);
			break;
	}


	bool isStable = false;
	while(!isStable)
	{
		if(pid == 0)
		{
			if(numMessages(mqueue) > 1)
			{
				float cPid = 10;
				float c2Pid = 10;
				float child1Value = receiveFloat(mqueue, cPid);
				float child2Value = receiveFloat(mqueue, c2Pid);
				upTemp = (temp + child1Value + child2Value)/3.0;
				qout << "Process " << pid << " current temperature " << upTemp << endl;
				if(qAbs(upTemp - temp) <= .01)
				{
					temp = upTemp;
					isStable = true;
					sendFloat(child_1, temp, 20);
					sendFloat(child_2, temp, 20);
				}
				else
				{
					temp = upTemp;
					sendFloat(child_1, temp, pid);
					sendFloat(child_2, temp, pid);
				}
			}

		}
		if(pid == 1 || pid == 2)
		{	
			if(hasMessages(mqueue))
			{
				float rPid = 10;
				float value = receiveFloat(mqueue, rPid);
				if(rPid == 20)
				{
					isStable = true;
					sendFloat(child_1, temp, 20);
					sendFloat(child_1, temp, 20);
				}

				else if(rPid == 0)
				{
					float downTemp = (temp + value)/2.0;
					temp = downTemp;
					sendFloat(child_1, downTemp, pid);
					sendFloat(child_2, downTemp, pid);
				}

				else if(rPid == 3 || rPid == 4 || rPid == 5 || rPid == 6)
				{
					float child1_value = value;
					float sPid = 10;
					float child2_value = receiveFloat(mqueue, sPid);
					if(sPid == 3 || sPid == 4 || sPid == 5 || sPid == 6)
					{
						upTemp = (temp + child1_value + child2_value)/3.0;
						temp = upTemp;
						qout << "Process " << pid << " current temperature " << upTemp << endl;
						sendFloat(parent, upTemp, pid);
					}
				}

			}
		}

		if(pid == 3 || pid == 4 || pid == 5 || pid ==6)	
		{
			if(hasMessages(mqueue))
			{
				float rPid;
				parentTemp = receiveFloat(mqueue, rPid);
				if(rPid == 20)
				{
					isStable = true;
				}
				else
				{
					downTemp = (temp + parentTemp)/2.0;
					temp = downTemp;
					sendFloat(parent, downTemp, pid); //true flags it as sent up 
					qout << "Process " << pid << " current temperature " << downTemp << endl;
				}
			}
		}
	}

	qout << "Process " << pid << " final temperature " << temp << endl;
	
	mq_close(parent);
	mq_close(child_1);
	mq_close(child_2);
	mq_close(mqueue);

	mq_unlink(P0);
	mq_unlink(P1);
	mq_unlink(P2);
	mq_unlink(P3);
	mq_unlink(P4);
	mq_unlink(P5);
	mq_unlink(P6);

	return 0;

} /*end main()*/



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

void sendFloat(mqd_t name, float f, float pid)
{
	char send[40];
	float *f_send = (float*)send;
	*f_send = f;
	f_send[1] = pid;
	mq_send(name, send, sizeof(f_send), 1); //original sizeof(float)+1
}

float receiveFloat(mqd_t mailbox, float &pid)
{
	char receive[40];
	float *f_receive = (float*)receive;
	mq_receive(mailbox, receive, 50, NULL);
	pid = f_receive[1];
	return f_receive[0];
}

mqd_t openMailbox(const char *name, mq_attr &ma){
	return mq_open(name, O_CREAT|O_RDWR, 0644, &ma);
}

bool hasMessages(mqd_t mqueue)
{
	mq_attr mqstat;
	bool hasMessages = false;
	if(!mq_getattr(mqueue, &mqstat))
	{
		if(mqstat.mq_curmsgs > 0)
		hasMessages = true;
	}

	return hasMessages;
}

int numMessages(mqd_t mqueue)
{
	mq_attr mqstat;
	if(!mq_getattr(mqueue, &mqstat))
	{
		return mqstat.mq_curmsgs;
	}

	else
		return 0;
}