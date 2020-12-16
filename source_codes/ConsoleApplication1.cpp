// ConsoleApplication1.cpp : Defines the entry point for the console application.
// Created by Jiahao Yan on 12/12/20.
// Copyright © 2020 Jiahao Yan. All rights reserved.


#include "stdafx.h"
#include<iostream>
#include<algorithm>
#include<vector>
#include<queue>
#include<stack>
#include<time.h>
using namespace std;

# define numT 10 // the number of tasks
# define numC 3 //the number of cores
# define Ts 3 //The time of sending task onto the cloud
# define Tc 1 //The time of computing task on the cloud
# define Tr 1 //The time of receiving task from the cloud
# define Ps 0.5 //The power of sending task onto the cloud
# define P1 1 //The power of consumption of core 1
# define P2 2 //The power of consumption of core 2
# define P3 4 //The power of consumption of core 3
vector<float> priority(numT);
queue<int> execution_order;
float initial_energy(0);
float initial_time(0);

template<class T>
void printv(vector<T> a)
{
	cout << "vector: ";
	for (int i = 0; i < a.size(); i++)
		cout << " " << a[i];
	cout << endl;
}
template<class T>
T min_value(vector<T> a)
{
	T min=a[0];
	for (int i = 1; i < a.size(); i++)
		if (a[i] < min)
			min = a[i];
	return min;
}//vector a must have at least one value;
template<class T>
T max_value(vector<T> a)
{
	T max = a[0];
	for (int i = 1; i < a.size(); i++)
		if (a[i] > max)
			max = a[i];
	return max;
}//vector a must have at least one value;
template<class T>
int min_index(vector<T> a)
{
	int min = 0;
	T temp = a[0];
	for (int i = 1; i < a.size(); i++)
		if (a[i] < temp)
		{
			temp = a[i];
			min = i;
		}
	return min;
}//vector a must have at least one value;
template<class T>
int max_index(vector<T> a)
{
	int max = 0;
	T temp=a[0];
	for (int i = 1; i < a.size(); i++)
		if (a[i] > temp)
		{
			temp = a[i];
			max = i;
		}
	return max;
}//vector a must have at least one value;
template <class T>
float avg(vector<T> a)
{
	T sum=0;
	for (int i = 0; i < a.size(); i++)
		sum += a[i];
	return (float)sum/(float)a.size();
}//vector a must have at least one value;

vector<int> Initial_Scheduling(vector<vector <int> >adj, vector<vector <int> >T_Local)
{
	vector<int> T_Local_min(numT),T_re(numT),L_or_R(numT),entry(numT);
	/********************************************************************
	if there are other entry tasks, setting
	the location before running, in my sample, the entry task is Task-1
	********************************************************************/
	entry[0] = 1;
	vector<float> W(numT);
	//primary assignment&& set Wi
	for (int i = 0; i < numT; i++)
	{
		T_Local_min[i] = min_value(T_Local[i]);
		T_re[i] = Ts + Tc + Tr;
		T_re[i] < T_Local_min[i] ? L_or_R[i] = 0 : L_or_R[i] = 1;
		if (entry[i] == 1)//the entry task must be local
			L_or_R[i] = 1;
		L_or_R[i] == 0 ? W[i] = T_re[i] : W[i] = avg(T_Local[i]);
	}
	//task prioritizing
	for (int i = numT - 1; i >= 0; i--)
	{
		vector<float>succ_priority(numT);
		for (int j = 0; j < numT; j++)
		{
			if (adj[i][j] == 1)
				succ_priority[j] = priority[j];
		}
		priority[i] = W[i] + max_value(succ_priority);
	}
	//Execution unit selection
	float energy(0);
	vector<float>priority_copy = priority;
	//get the order of the priority
	queue<int>execution;
	for (int i = 0; i < numT; i++)
	{
		int order = max_index(priority_copy);
		execution.push(order);
		priority_copy[order] = 0;
	}
	execution_order = execution;
	vector<int> S(numT);//the location of each task executes
	vector<int> Task_end(numT);//the finish time of each task
	//saving the available time for each channal
	vector<int> C_1 = {0}, C_2 = { 0 }, C_3 = { 0 }, 
		W_S = { 0 }, Cloud = { 0 }, W_R = { 0 };
	vector<int> channal(numC+1);
	int current = 0;
	for (int i = 0; i < numT; i++)
	{
		current = execution.front();
		channal[0] = T_re[current];
		channal[1] = T_Local[current][0];
		channal[2] = T_Local[current][1];
		channal[3] = T_Local[current][2];
		if (L_or_R[current] == 0)//V(current) is a cloud task
		{
			vector<int> pred_ws = { 0 }, pred_c = { 0 };
			for (int j = 0; j < numT; j++)
			{
				if (adj[j][current] == 1)
				{
					if (S[j] == 0)
					{
						pred_ws.push_back(W_S.back());
						pred_c.push_back(Cloud.back());
					}
					else pred_ws.push_back(Task_end[j]);
				}
			}
			int RT_ws = max_value(pred_ws);
			W_S.push_back(RT_ws + Ts);
			pred_c.push_back(RT_ws + Ts);
			int RT_c = max_value(pred_c);
			Cloud.push_back(RT_c + Tc);
			W_R.push_back(Cloud.back() + Tr);
			S[current] = 0;
			Task_end[current] = W_R.back();
			energy += (float)Ps*(float)Ts;
		}
		else//V(current) is not a cloud task
		{
			vector<int> pred = { 0 }, FT(numC + 1), pred_ws = { 0 };
			for (int j = 0; j < numT; j++)
				if (adj[j][current] == 1)
				{
					if (S[j] != 0)
					{
						pred.push_back(Task_end[j]);
						pred_ws.push_back(Task_end[j]);
					}
					else
					{
						pred_ws.push_back(W_S.back());
						pred.push_back(Task_end[j]);
					}
				}
			int RT_l = max_value(pred),RT_c=max_value(pred_ws);
			RT_c > W_S.back() ? FT[0] = RT_c + channal[0] : FT[0] = W_S.back() + channal[0];
			RT_l > C_1.back() ? FT[1] = RT_l + channal[1] : FT[1] = C_1.back() + channal[1];
			RT_l > C_2.back() ? FT[2] = RT_l + channal[2] : FT[2] = C_2.back() + channal[2];
			RT_l > C_3.back() ? FT[3] = RT_l + channal[3] : FT[3] = C_3.back() + channal[3];
			/*********************************************************************
			If the task is entry task, it'll must be local;
			So we should not allow it to run on the cloud.
			*********************************************************************/
			if (entry[current] == 1)
				FT[0] = FT[1] + FT[2] + FT[3];
			S[current] = min_index(FT);
			switch (S[current])
			{case 0:
			{
				W_R.push_back(FT[0]);
				Cloud.push_back(FT[0] - Tr);
				W_S.push_back(FT[0] - Tr - Tc);
				Task_end[current] = W_R.back();
				energy += (float)Ps*(float)Ts;
				break;
			}
			case 1:
			{	
				C_1.push_back(FT[1]);
				Task_end[current] = C_1.back();
				energy += (float)P1*(float)channal[1];
				break;
			}
			case 2:
			{
				C_2.push_back(FT[2]);
				Task_end[current] = C_2.back();
				energy += (float)P2*(float)channal[2];
				break;
			}
			case 3:
			{
				C_3.push_back(FT[3]);
				Task_end[current] = C_3.back();
				energy += (float)P3*(float)channal[3];
				break;
			}
			default:
				break;
			}
		}
		execution.pop();//clear the execution queue
	}
	initial_time = max_value(Task_end);//or use the last time of the exit task finished
	cout << "Total time is: " << initial_time << endl;
	cout << "Total energy is: " << energy << endl;
	initial_energy = energy;
		return S;
}
vector<int> Task_Migration(vector<vector <int> >adj, vector<vector <int> >T_Local, vector<int> S, int T_max)
{
	vector<int> result(numT);
	vector<int> S_new(S);//the location of each task executes
	int temp_s, temp_time(0);
	float best_energy((float)initial_energy), best_time((float)initial_time);//my sample
	//float best_energy(100.5), best_time(18);
	for (int m = 0; m < numT; m++)
	{
		if (S_new[m] != 0)//not cloud
		{
			vector<vector <int> >S_choice;
			S_choice.resize(numC+1, vector<int>(numT));
			vector<float>E(numC+1), T(numC+1);
			for (int n = 0; n < numC + 1; n++)
			{
				float energy(0);
				if (n != S_new[m])// S_new must be different from the original channal
				{
					queue<int> execution = execution_order;
					//assumpt that sequence has low total time or low total energy
					//if it violates T_max or is not the best ratio of energy reduction
					//we'll recover the sequence before next loop
					temp_s = S_new[m];
					S_new[m] = n;
					vector<int> Task_end(numT);//the finish time of each task
											   //saving the available time for each channal
					vector<int> C_1 = { 0 }, C_2 = { 0 }, C_3 = { 0 },
						W_S = { 0 }, Cloud = { 0 }, W_R = { 0 };
					vector<int> channal(numC + 1);
					int current = 0,T_total(0);
					for (int i = 0; i < numT; i++)
					{
						current = execution.front();
						channal[0] = Ts + Tc + Tr;
						channal[1] = T_Local[current][0];
						channal[2] = T_Local[current][1];
						channal[3] = T_Local[current][2];
						if (S_new[current] == 0)//V(current) is a cloud task
						{
							vector<int> pred_ws = { 0 }, pred_c = { 0 };
							for (int j = 0; j < numT; j++)
							{
								if (adj[j][current] == 1)
								{
									if (S_new[j] == 0)
									{
										pred_ws.push_back(W_S.back());
										pred_c.push_back(Cloud.back());
									}
									else pred_ws.push_back(Task_end[j]);
								}
							}
							int RT_ws = max_value(pred_ws);
							W_S.push_back(RT_ws + Ts);
							pred_c.push_back(RT_ws + Ts);
							int RT_c = max_value(pred_c);
							Cloud.push_back(RT_c + Tc);
							W_R.push_back(Cloud.back() + Tr);
							Task_end[current] = W_R.back();
							energy += (float)Ps*(float)Ts;
						}
						else if(S_new[current] == 1)// allocate the task into core1
						{
							vector<int> pred = { 0 };
							int FT(0);
							for (int j = 0; j < numT; j++)
								if (adj[j][current] == 1)
									pred.push_back(Task_end[j]);
							int RT_l = max_value(pred);
							RT_l > C_1.back() ? FT = RT_l + channal[1] : FT = C_1.back() + channal[1];
							C_1.push_back(FT);
							Task_end[current] = C_1.back();
							energy += (float)P1*(float)channal[1];
						}
						else if (S_new[current] == 2)// allocate the task into core2
						{
							vector<int> pred = { 0 };
							int FT(0);
							for (int j = 0; j < numT; j++)
								if (adj[j][current] == 1)
									pred.push_back(Task_end[j]);
							int RT_l = max_value(pred);
							RT_l > C_2.back() ? FT = RT_l + channal[2] : FT = C_2.back() + channal[2];
							C_2.push_back(FT);
							Task_end[current] = C_2.back();
							energy += (float)P2*(float)channal[2];
						}
						else if (S_new[current] == 3)// allocate the task into core3
						{
							vector<int> pred = { 0 };
							int FT(0);
							for (int j = 0; j < numT; j++)
								if (adj[j][current] == 1)
									pred.push_back(Task_end[j]);
							int RT_l = max_value(pred);
							RT_l > C_3.back() ? FT = RT_l + channal[3] : FT = C_3.back() + channal[3];
							C_3.push_back(FT);
							Task_end[current] = C_3.back();
							energy += (float)P3*(float)channal[3];
						}
						temp_time=max_value(Task_end);
						if (temp_time > T_max)
							break;
						execution.pop();//clear the execution queue
					}
					int test(0);
				}
				T[n] = temp_time;
				E[n] = energy;
				S_choice[n] = S_new;
			}
			//judge which choice
			int selected(0);
			for (int k = 0; k < numC+1; k++)
			{
				if (T[k] <= best_time&& E[k] < best_energy)
				{	
					S_new= S_choice[k];
					result = S_choice[k];
					best_time = T[k];
					best_energy = E[k];
					selected = 1;
					//cout << "ideal case time & energy: " << best_time << " " << best_energy << endl;
					//cout << " sequece: ";
					//printv(result);
				}
			}
			if (selected == 0)
			{
				for (int k = 0; k < numC+1; k++)
				{
					if (T[k] <= T_max&& E[k] < best_energy)
					{
						S_new = S_choice[k];
						result = S_choice[k];
						best_time = T[k];
						best_energy = E[k];
					}
				}
				//cout << "**second case time & energy: " << best_time << " " << best_energy << endl;
				//cout << "**sequece: ";
				//printv(result);
			}
		}
	}
	cout << "total time & energy: " << best_time << " " << best_energy << endl;
	return result;
}
int main()
{
	vector<vector <int> >adj = { 
	{ 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };//Example 1
	/*
	vector<vector <int> >adj = {
		{ 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 1, 1, 0 },
		{ 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 1, 1, 1, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
		{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };//Example 2
		*/
	vector<vector <int> >T_Local = {
		{ 9,7,5 },
		{ 8,6,5 },
		{ 6,5,4 },
		{ 7,5,3 },
		{ 5,4,2 },
		{ 7,6,4 },
		{ 8,5,3 },
		{ 6,4,2 },
		{ 5,3,2 },
		{ 7,4,2 },
	}; 
	clock_t initial_begin, initial_end, migration_begin, migration_end;
	initial_begin = clock();
	vector<int> a = Initial_Scheduling(adj, T_Local);
	initial_end = clock();
	cout << "after initial scheduling: ";
	printv(a);
	cout<<"running time of initial scheduling: "<< (double)(initial_end - initial_begin) / CLOCKS_PER_SEC << endl;
	//vector<int> S = { 3,0,3,1,3,2,3,2,3,3 }; the paper's task sequence
	migration_begin = clock();
	vector<int> b = Task_Migration(adj, T_Local, a, 27);
	migration_end = clock();
	cout << "after task migration: ";
	printv(b);
	cout << "running time of task migration: " << (double)(migration_end - migration_begin) / CLOCKS_PER_SEC << endl;
	system("pause");
}

