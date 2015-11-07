// Algoritmo NeedlemanWunsch.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
using namespace std;

int A = 0, G = 1, C = 2, T = 3;
int esp = -5;


int similitud(int first, int second)
{
	return first == second? 1 : -1;
}

vector<char> convertirStringToVector(string str)
{
	vector<char> l;
	for (int i = 0; i < str.size(); ++i)
	{
		l.push_back(str[i]);
	}
	return l;
}

void imprimir_matriz(int** M, vector<char> sec_1, vector<char> sec_2) {
	int n = sec_1.size();
	int m = sec_2.size();
	
	cout << '\t' << '\t';
	for (int j = 0; j < m; j++) {
		cout << sec_1[j] << '\t';
	}
	cout << endl << endl;	
	for (int i = 0; i <= n; i++ ) {
		if (i > 0){
		cout << sec_2[i-1] << '\t';
		} else {
			cout << '\t';
		}
		for (int j = 0; j <= m; j++) {
			cout << M[j][i] << '\t';
		}
		cout << endl << endl;
	}
}

int main()
{
	string variable1; 
	string variable2;
	
	cout << "Ingrese primer archivo: "; 
	cin >> variable1;
	cout << endl;
	cout << "Ingrese segundo archivo: ";
	cin >> variable2;

	/// abrimos las secuencias
	ifstream file1(variable1.c_str());
	ifstream file2(variable2.c_str());
	string buffer;
	
	string h1, h2;

	while (file1.good()) {
		getline(file1, buffer);
		h1.append(buffer);
	}

	while (file2.good()) {
		getline(file2, buffer);
		h2.append(buffer);
	}

	int n = h1.size();
	int m = h2.size();

	int ** M = new int *[n + 1];
	for (int i = 0; i <= n; i++)
		M[i] = new int[m + 1];

	M[0][0] = 0;

	for (int j = 1; j <= m; j++)
		M[0][j] = -j;
	for (int i = 1; i <= n; i++)
		M[i][0] = -i;

	vector<char> sec_1 = convertirStringToVector(h1);
	vector<char> sec_2 = convertirStringToVector(h2);
	/// generamos la matriz
	for (int i = 1; i <= n; i++) {
		for (int j = 1; j <= m; j++) {
			int match = M[i-1][j-1] + similitud(sec_1[i-1], sec_2[j-1]);
			int insert = M[i-1][j] - 1;
			int deletenw = M[i][j-1] - 1;
			int max;
			int values[] = {match, insert, deletenw};
			// cout << 
			// 	"[" << i << "]" <<
			// 	"[" << j <<"] " << 
			// 	" M" << match << 
			// 	" I" << insert << 
			// 	" D" << deletenw << 
			// 	" [i-1][j-1]" << M[i-1][j-1] << endl;
			/// escogemos al mayor y lo guardamos en max
			max = match;
			for (int temp = 1; temp < 3; temp++){
				if (values[temp] > max){
					max = values[temp];
				}
			}
			M[i][j] = max;
			cout << max << endl << endl;
		}
	}

	imprimir_matriz(M, sec_1, sec_2);
	cout << "El mejor Score es: " << M[n][m] << endl;


	for (int i = 0; i <= n; ++i) {
		delete M[i];
	}
	delete M;

    return 0;
}

