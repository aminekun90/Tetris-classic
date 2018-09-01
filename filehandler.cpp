#include "filehandler.h"

DWORD GetHeightScore()
{
	DWORD points = 0;
	ifstream f("score.data");
	if(f)
	{
		while(f >> points){}
		f.close();
	}else
	{
		return 0;
	}
	return points;
}

void RegisterHightScore(DWORD points){
	DWORD temp = GetHeightScore();
	ofstream f("score.data");
	if(f)
	{
		if(points>temp)
		{
		f<<points;
		f.close();
		}
	}

}
void RegisterScore(DWORD points){
	string temp[6] = { "NAN", "0", "NAN", "0", "NAN", "0" };
	GetScore(temp);
	string name;
	if (points >= atoi(temp[5].c_str()))
	{
		system("cls");
		cout << endl<<"\t\t\t\t-->Nouveau Score<--"<<endl;
	cout << endl<<"\t\t\tVotre Nom : ";
	cin >> name;
	ofstream f("splus.data",ios::ate);
	if (f)
	{
		if (points >= atoi(temp[1].c_str()))
		{
			f << name << endl
				<< points << endl
				<< temp[0] << endl
				<< temp[1] << endl
				<< temp[2] << endl
				<< temp[3] << endl;
			
		}else if(points>=atoi(temp[3].c_str())){
			f << temp[0] << endl
				<< temp[1] << endl
				<< name << endl
				<< points << endl
				<< temp[2] << endl
				<< temp[3] << endl;
		}else if(points>=atoi(temp[5].c_str()))
		{
			f << temp[0] << endl
				<< temp[1] << endl
				<< temp[2] << endl
				<< temp[3] << endl
				<< name << endl
				<< points << endl;
		}else
		{
			
				f << temp[0] << endl
				<< temp[1] << endl
				<< temp[2] << endl
				<< temp[3] << endl
				<< temp[4] << endl
				<< temp[5] << endl;
		}

	f.close();
	}
	}
	else return;
}
 void GetScore(string *points)
{
	
	short i=0;
	ifstream f("splus.data");
	if (f)
	{
		while (f >> points[i]){
			
			if (i == 5){ break; }
			i++;
		}
		f.close();
	}
	else
	{
		ofstream f1("splus.data", ios::ate);
			if(f1){
				f1 << "NAN" << endl << 0 << endl << "NAN" << endl << 0 << endl << "NAN" << endl << 0 << endl;
				for (int i = 0; i < 6;i++)
				{
					if (i % 2 == 0){ points[i] = "NAN"; }
					points[i] = "0";
				}
				f1.close();
			}
		
	}
	
}