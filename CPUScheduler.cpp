/******************************************************************
* 	Author: Jason Hachach
*	Class: COSC 3336
*	Purpose: Create a program to simulate the execution of a 
*			 process by a tablet with large memory, one display, 
*			 a multi-core processing unit, and onesolid-state drive
******************************************************************/
#include <iostream>
#include <string>
#include <stdio.h>
#include <vector>
#include <iomanip>
#include <algorithm>

using namespace std;

struct command
{
	int time;
	int startTime;
	int processNum;
	string command;	
	bool termStatement;
};

void printNewProcess(vector<string>, int, int);
void printTermProcess(vector<string>, int, int, int);
void printSummary(int, int, int, double, double);
int total (vector <int>);
vector<command> sortVector(vector<command>);

int main()
{
	int nCores;
	int numCoresAvail;
	int clock;
	int crt = 0;
	int srt = 0;
	int irt = 0;
	int SSDAccess = 0;
	int i = 0;
	double coreT = 0.0;
	double ssdT = 0.0;
	bool Sbusy = false;
	bool Ibusy = false;
	command front;
	vector <string> status;
	vector <command> input;
	vector <int> dCore;
	vector <command> core;
	vector <int> dSsd;
	vector <command> ssd;
	vector <int> dIo;
	vector <command> io;
	
//	Input variables
	string statement;
	int inTime;
	int numProcess = -1;

//	Read from file	
	while(cin >> statement && cin >> inTime)
	{
		
		input.push_back(command());
		input[i].command = statement;
		input[i].time = inTime;
		input[i].termStatement = false;
		i++;
		if(statement == "CORE" || statement == "SSD" || statement == "INPUT")
		{
			input.push_back(command());
			input[i].command = "R"+ statement;
			input[i].termStatement = false;
			input[i].time = 0;
			i++;
			
		}
		
	}
// assign the rest of the elements
	for(i = 1; i < input.size();i++)
	{

//	If the command is new increment numProcess and assign to processNum	
		if(input[i].command == "NEW")
		{		
			numProcess++;
			
// If numProcess is greater than zero make previous command terminating statement		
			if(numProcess > 0)
			{
				input[i-1].termStatement = true;
				input[i].startTime = input[i].time;
				input[i].time = 0;
			}
			else
			{
				input[i].startTime = 0;
				
			}	
			
		}
		
//	create starting time for each commmand
		else
			input[i].startTime = input[i - 1].startTime + input[i - 1].time;
		input[i].processNum = numProcess;
	}
	
// Makes last input the terminating statement
	input[input.size()-1].termStatement = true;

// Sort input by starting time
	input = sortVector(input);
	
	

// Simulate process in order of the queue starting with nCores
	
	
	front = input[0];
	nCores = front.time;
	int busy[nCores] = {0};
	numCoresAvail = nCores;
	input.erase(input.begin());
	while(!input.empty())
	{
		front = input.front();
		clock = front.startTime;
	
//	If command is new then print the time and status of the rest of the processes
		if(front.command == "NEW")
		{
			status.push_back("READY");
			printNewProcess(status, clock, status.size());
		}
		
		else if(front.command == "CORE")
		{
			
//	if there are cores available then mark it busy till current time + time of command 
			if(numCoresAvail > 0)
			{
				coreT += front.time;
				crt = front.startTime + front.time;
				
// Garuntees that the time the next available core will be the first element
				busy[0] = crt;
				sort(busy,busy + nCores);
				numCoresAvail--;
				status[front.processNum] = "RUNNING";


			}
			else 
			{
// insert the delay before the process can go in
				int delay = busy[0] - clock;
				dCore.insert(dCore.end(), delay);

// Get total delay of everything in the vector
				delay = total(dCore);
				
// Iterate the starting time by delay to all elements of the same process number
				for(int j = 0;j < input.size(); j++)
				{
					if(input[j].processNum == front.processNum)
						input[j].startTime += delay;
				}
				core.push_back(front);
			}
			
		}
		else if(front.command == "RCORE")
//	add one numCoresAvail and make one of the cores free 
		{
			numCoresAvail++;
			busy[0] = 0;
			
//	if there is an element in core vector then the next command to execute is the front element
			if(core.size() > 0)
			{
				dCore.erase(dCore.begin());
				input.insert(input.begin()+1, core.front());
				core.erase(core.begin());
			}
		}
		else if(front.command == "SSD")
		{

//	check if the ssd is free
			if(srt <= clock )
			{
//	make ssd free until clock + command time
				ssdT += front.time;
				SSDAccess++;
				srt = clock + front.time;
				status[front.processNum] = "BLOCKED";
			}
			else
			{
//	get delay and add it to the starting time to every command of the same processNum
				int delay = srt - clock;
				dSsd.insert(dSsd.end(), delay);
				delay = total(dSsd);
					
				for(int j = 0;j < input.size(); j++)
				{
					if(input[j].processNum == front.processNum)
						input[j].startTime += delay;
				}
				front.startTime += delay;
				ssd.push_back(front);
			}
		}
		else if(front.command == "RSSD")
		{
//	make the first element in ssd queue the next front 
			if(ssd.size() > 0)
			{
				dSsd.erase(dSsd.begin());
				input.insert(input.begin()+1, ssd.front());
				ssd.erase(ssd.begin());
			}
			
		}

		else if(front.command == "INPUT")
		{
			
//	check if input is free mark busy till current time + command time
			if(irt <= clock)
			{
				srt = clock + front.time;
				status[front.processNum] = "BLOCKED";
			}
			else
			{
// add delay to the rest of the command with the same processNum
				int delay = irt - clock;
				dIo.insert(dIo.end(), delay);
				delay = total(dIo);
					
				for(int j = 0;j < input.size(); j++)
				{
					if(input[j].processNum == front.processNum)
						input[j].startTime += delay;
				}
				
				io.push_back(front);
			}
			
		}
		else
		{
			if(io.size() > 0)
			{
				dIo.erase(dIo.begin());			
				input.insert(input.begin()+1, io.front());
				io.erase(io.begin());
			}
		}
// if the current command is the last one for the process print time and current status of other processes
		if(front.termStatement)
		{
			clock += front.time;
			status[front.processNum] = "TERMINATED";
			printTermProcess(status, clock, status.size(), front.processNum);
		}	
// remove command fromt input and resort array by starting time
		input.erase(input.begin());
		if(!input.empty())
			input = sortVector(input);
	}
	
// print the summary of all processes, cpu utilization and ssd utilization	
	printSummary(clock, numProcess + 1, SSDAccess, ssdT, coreT);
	
	return 0;

}
// 	Calls whenever a new process is introduced into the program
//	Prints the current status of all processes and the time the new one comes in.
void printNewProcess(vector<string> status, int time, int size)
{
	cout << "Process: " << size -1 << " starts at time: " << time << "ms" << endl;
	
	for(int i = 0; i < size - 1; i++)
		cout << "Process: " << i << " is in the " << status[i] << " state." << endl;
	
	cout<< endl;
}
//	Calls whenever a process terminates
//	Prints the current status of all processes and the time the process terminates
void printTermProcess(vector<string> term, int time, int size, int processNum)
{
	cout << "Process: " << processNum << " Terminates at time: " << time << "ms" << endl;
	for(int i = 0; i < size; i++)
		cout << "Process: " << i << " is in the " << term[i] << " state." << endl;
	
	cout<< endl;
}
//	Prints the total number of processes, the amount of times the ssd was accessed
//	total time of the program, core untilization, and ssd Utilization
void printSummary(int time, int nProc, int ssd, double sAvg, double cUtil)
{
	cout << "Number of processes that completed: " << nProc << endl;
	cout << "Total number of SSD accesses: " << ssd << endl;
	cout << "Average SSD access time: " << sAvg / ssd << " ms" << endl;
	cout << "Total elasped time: " << time << " ms" << endl;
	cout << setprecision(4) << "Core utilization: " << (cUtil / time) * 100 << " percent" << endl;
	cout << setprecision (2) << "SSD utilization: " << (sAvg / time ) * 100 << " percent " << endl;
}

//Sorts the main vector by starting time 
vector<command> sortVector(vector<command> v1)
{
	vector<command> tmp;
	int i =1;
	tmp.push_back(command());
	tmp[0] = v1[0];
	v1.erase(v1.begin());
	
	while(!v1.empty())
	{
		int lowIndex = 0;
		for(int j = 1; j < v1.size(); j++)
		{
			if(v1[lowIndex].startTime > v1[j].startTime)
				lowIndex = j;
		}
		tmp.push_back(command());
		tmp[i] = v1[lowIndex];
		v1.erase(v1.begin() + lowIndex);
		i++;
	}
	return tmp;	
}

//	Used to find the total value of the integers in the vector
int total (vector <int> v1)
{
	int total = 0;
	
	for(int i = 0; i < v1.size(); i++)
		total += v1[i];
	
	return total;	
}
