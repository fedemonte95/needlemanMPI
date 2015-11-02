// Algoritmo NeedlemanWunsch.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>
using namespace std;

int A = 0, G = 1, C = 2, T = 3;
int esp = -5;


int similitud(int first, int second)
{
	return first == second? 1 : -1;
}

vector<int> convertirStringToVector(string str)
{
	vector<int> l;
	for (int i = 0; i < str.size(); ++i)
	{
		int n = -1;
		if (str[i] == 'A')
			n = 0;
		if (str[i] == 'G')
			n = 1;
		if (str[i] == 'C')
			n = 2;
		if (str[i] == 'T')
			n = 3;
		l.push_back(n);
	}
	return l;
}

void imprimir_matriz(int** M, int n, int m) {
	for (int i = 0;i < n;++i) {
		for (int j = 0;j < m;++j) {
			cout << M[i][j] << '\t';
		}
		cout << endl << endl;
	}

}

int main()
{
	ofstream file("resultado.txt");

	/// abrimos las secuencias
	ifstream file1("secuencia1.txt");
	ifstream file2("secuencia2.txt");
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

	vector<int> sec_1 = convertirStringToVector(h1);
	vector<int> sec_2 = convertirStringToVector(h2);
	/// generamos la matriz
	for (int i = 1; i <= n; i++) {
		for (int j = 1; j <= m; j++) {
			int match = M[i-1][j-1] + similitud(sec_1[i], sec_2[j]);
			int insert = M[i-1][j] - 1;
			int deletenw = M[i][j-1] - 1;
			int max;
			/// escogemos al mayor y lo guardamos en max
			if (match >= insert)
				max = match;
			else
				max = insert;
			
			if (max >= deletenw)
				max = max;
			else
				max = deletenw;

			M[i][j] = max;
			
		}
	}

	imprimir_matriz(M, n + 1, m + 1);

	/// obtenemos las alineaciones

	string alin_1 = ""; 
	string alin_2 = "";
	int i = n; int j = m;
	while (i > 0 && j > 0) {
		int score = M[i][j];
		int diag = M[i-1][j-1];
		int up = M[i][j-1];
		int izq = M[i-1][j];
		if (score == diag + similitud(sec_1[i - 1], sec_2[j - 1])) {
			alin_1 = h1[i - 1] + alin_1;
			alin_2 = h1[i - 1] + alin_2;
			--i;--j;
		}
		else if (score == izq - 1) {
			alin_1 = h1[i - 1] + alin_1;
			alin_2 = "-" + alin_2;
			--i;
		}
		else if (score == up - 1) {
			alin_1 = "-" + alin_1;
			alin_2 = h2[j - 1] + alin_2;
			--j;
		}
	}
	while (i > 0) {
		alin_1 = h1[i - 1] + alin_1;
		alin_2 = "-" + alin_2;
		--i;
	}

	while (j > 0) {
		alin_1 = "-" + alin_1;
		alin_2 = h2[j - 1] + alin_2;
		--j;
	}

	cout << "Alineacion 1: " << alin_1 << endl;
	cout << "Alineacion 2: " << alin_2 << endl;

	file << "Alineacion 1: " << alin_1 << endl;
	file << "Alineacion 2: " << alin_2 << endl;

	for (int i = 0; i <= n; ++i) {
		delete M[i];
	}
	delete M;

    return 0;
}

