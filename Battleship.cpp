// Battleship.cpp: определяет точку входа для консольного приложения.
//
#include "stdafx.h"
#include <iostream>
#include <conio.h>
#include <ctime>
#include <windows.h>
using namespace std;


const int turn_N = 4;
const int numship = 10;
const bool hor = 1;
const bool ver = 0;
const int X0 = 10;
const int Y0 = 10;
const int X0Y0 = 100;
const int firstphase = 4;
const int size_sr = 9;
const int maxlen = 4;
const char a_1 = 95;//горизонтальная линия верхней и нижний части поля
const char aI1 = 166;//вертикальная линия
const char aS1 = 72;//знак ячейки корабля
const char aS2 = 42;//попадание
const char aS3 = 35;//ячейка потопленного корабля
const char aN1 = 111;//промах
const char aI2 = 134;//крестовая перегородка
const char a_2 = 45;//горизонтальная перегородка
const char A = 65; //Латинская буква A
const int delay = 3000;

struct sr //сектор обстрела
{
	int turnphase;	//кол-во ходов, необходимых для обстрела этого участка
	int phase[turn_N];	//phase
	int next;	//next - следующая фаза
	int use;	// use - использованна ли эта фаза в стратегии
};

struct tn // структура, описывающия ходы
{
	int phaseturn;	//указывание на часть фазы стратегии ИИ
	int endstrat;	//окончание фаз стратегии
	int arrXY[maxlen];	//координаты поврежденного корабля
	int hitship;	//кол-во попаданий по кораблю
	bool supposed_vector; //предпологаемый вектор, определяемый двумя попаданиями
	sr sector[size_sr];	//информация для хода ИИ
	int numb;// номер текущего хода
	bool p_turn; // Последоавтельность хода (1 - нечетные ходы игрока, 0 - четные ходы игрока)
};

struct st // сегмент корабля
{
	int x, y; // координаты сегмента коробля
	bool whole; // состояние сегмента (1 - целый, 0 - поврежденн)
};

struct ship // корабль
{
	st segments[4];
	int length;//его длинна
	int destroy, vector;//destroy - потоплен корабль или нет(1 - потоплен, 0 - нет), vector - направление корабля
};

struct field //игровое поле
{
	ship fleet[numship];
	bool f[X0][Y0]; // поле кораблей
	bool strike[X0][Y0];// поле попаданий
	bool death_f[X0][Y0];// поле полностью потонувших короблей
	int n; //кол-во еще не потопленных короблей
};

//nullarr - функции, обнуляющие массивы
void nullarr(int *arr, int size)
{
	for (int i = 0; i < size; ++i)
		arr[i] = 0;
}

void nullarr(bool *arr, int size)
{
	for (int i = 0; i < size; ++i)
		arr[i] = 0;
}

void nullarr(bool arr[X0][Y0])
{
	for (int i = 0; i < X0; ++i)
		for (int j = 0; j < Y0; ++j)
			arr[i][j] = 0;
}

//обнуление корбля
ship nullship()
{
	ship copy;
	int len = 1;
	copy.destroy = 0;
	copy.length = len;
	copy.vector = 0;
	for (int i = 0; i < len; ++i)
	{
		copy.segments[i].x = 0;
		copy.segments[i].y = 0;
		copy.segments[i].whole = 1;
	}
	return copy;
}

//обнуление поля
field nullfield()
{
	field f;
	nullarr(f.f);
	nullarr(f.strike);
	nullarr(f.death_f);
	for (int i = 0; i < 10; ++i)
		f.fleet[i] = nullship();
	f.n = 0;
	return f;
}

//обнуление описания ходов
tn nullturn()
{
	tn t;
	t.endstrat = 0;
	t.hitship = 0;
	t.phaseturn = 0;
	t.supposed_vector = 0;
	t.numb = 1;
	return t;
}

// проверка на наличие кораблей, в определенной координате
int clearcube(field &ff, int x1, int y1)
{
	if (ff.f[x1][y1])
		return 0;
	if (y1 && x1 && (y1 != (Y0 - 1)) && (x1 != (X0 - 1)))
	{
		for (int i = x1 - 1; i <= x1 + 1; ++i)
			for (int j = y1 - 1; j <= y1 + 1; ++j)
				if (ff.f[i][j])
					return 0;
	}
	else if (!y1 && x1 == (X0 - 1))
	{
		for (int i = x1 - 1; i <= x1; ++i)
			for (int j = y1; j <= y1 + 1; ++j)
				if (ff.f[i][j])
					return 0;
	}
	else if (!x1 && y1 == (Y0 - 1))
	{
		for (int i = x1; i <= x1 + 1; ++i)
			for (int j = y1 - 1; j <= y1; ++j)
				if (ff.f[i][j])
					return 0;
	}
	else if (!y1 && x1)
	{
		for (int i = x1 - 1; i <= x1 + 1; ++i)
			for (int j = y1; j <= y1 + 1; ++j)
				if (ff.f[i][j])
					return 0;
	}
	else if (y1 && !x1)
	{
		for (int i = x1; i <= x1 + 1; ++i)
			for (int j = y1 - 1; j <= y1 + 1; ++j)
				if (ff.f[i][j])
					return 0;
	}
	else if (!y1 && !x1)
	{
		for (int i = x1; i <= x1 + 1; ++i)
			for (int j = y1; j <= y1 + 1; ++j)
				if (ff.f[i][j])
					return 0;
	}
	else if ((y1 == (Y0 - 1)) && (x1 != (X0 - 1)))
	{
		for (int i = x1 - 1; i <= x1 + 1; ++i)
			for (int j = y1 - 1; j <= y1; ++j)
				if (ff.f[i][j])
					return 0;
	}
	else if ((y1 != (Y0 - 1)) && (x1 == (X0 - 1)))
	{
		for (int i = x1 - 1; i <= x1; ++i)
			for (int j = y1 - 1; j <= y1 + 1; ++j)
				if (ff.f[i][j])
					return 0;
	}
	else if ((y1 == (Y0 - 1)) && (x1 == (X0 - 1)))
	{
		for (int i = x1 - 1; i <= x1; ++i)
			for (int j = y1 - 1; j <= y1; ++j)
				if (ff.f[i][j])
					return 0;
	}
	return 1;
}

// проверка на наличие кораблей, на месте создоваемого корабля
int clearship(field &f0, int length, int vector, int x, int y)
{
	int clear = 1;
	if (vector)
	{
		if ((X0 - x) < length)
			return 0;
		for (int i = 0; i < length; ++i)
			clear = clear * clearcube(f0, (x + i), y);
	}
	else
	{
		if ((Y0 - y) < length)
			return 0;
		for (int i = 0; i < length; ++i)
			clear = clear * clearcube(f0, x, (y + i));

	}
	return clear;
}

//Создание корабля 
void createship(field &f0, int numb, int length, int vector, int xy)
{
	++f0.n;
	f0.fleet[numb].length = length;
	if (vector)
		for (int i = 0; i < length; ++i)
		{
			f0.fleet[numb].segments[i].x = xy % 10 + i;
			f0.fleet[numb].segments[i].y = xy / 10;
			f0.fleet[numb].segments[i].whole = 1;
		}
	else
		for (int i = 0; i < length; ++i)
		{
			f0.fleet[numb].segments[i].x = xy % 10;
			f0.fleet[numb].segments[i].y = xy / 10 + i;
			f0.fleet[numb].segments[i].whole = 1;
		}
	f0.fleet[numb].vector = vector;
	if (vector)
		for (int i = f0.fleet[numb].segments[0].x; i < f0.fleet[numb].segments[0].x + length; ++i)
			f0.f[i][f0.fleet[numb].segments[0].y] = 1;
	else
		for (int i = f0.fleet[numb].segments[0].y; i < f0.fleet[numb].segments[0].y + length; ++i)
			f0.f[f0.fleet[numb].segments[0].x][i] = 1;
}

//определение дленны корабля
int lenghtship(int i)
{
	if (i <= 3)
		return 1;
	if (i <= 6)
		return 2;
	if (i <= 8)
		return 3;
	return 4;
}

//расстоновка кораблей в случайном порядке
field randomfleet()
{
	field f1 = nullfield();
	int randarr[X0Y0];
	for (int i = numship - 1; i >= 0; --i)
	{
		int ind = 0;
		int sclear;
		nullarr(randarr, X0Y0);
		int length = lenghtship(i);
		int vector = rand() % 2;
		for (int k = 0; k < X0; ++k)
			for (int j = 0; j < Y0; ++j)
			{
				sclear = clearship(f1, length, vector, k, j);
				if (sclear)
				{
					randarr[ind] = j * 10 + k;
					++ind;
				}
			}
		int XY0 = randarr[rand() % ind];
		createship(f1, i, length, vector, XY0);
	}
	return f1;
}

//вывод на экран одного поля
void output_field(field f)
{
	int indx = 0, indy = 0, ind = 0;
	cout << ' ' << ' ';
	for (int i = 0; i < 19; ++i)
		if (!(i % 2))
		{
			++ind;
			cout << ind;
		}
		else
			cout << ' ';
	cout << endl << ' '<< ' ';
	for (int i = 0; i < 19; ++i)
		cout << a_1;
	cout << endl;
	ind = 0;
	for (int i = 0; i < 19; ++i)
	{
		if (!(i % 2))
			for (int j = 0; j < 22; ++j)
				if (!j)
				{
					cout << char(A + ind);
					++ind;
				}
				else if ((j % 2))
					cout << aI1;
				else
				{
					if (f.death_f[indx][indy])
						cout << aS3;
					else if (f.f[indx][indy] && !f.strike[indx][indy])
						cout << aS1;
					else if (f.f[indx][indy] && f.strike[indx][indy])
						cout << aS2;
					else if (f.strike[indx][indy])
						cout << aN1;
					else
						cout << ' ';
					++indx;
				}
		else
		{
			for (int j = 0; j < 21; ++j)
				if (!j)
					cout << ' ' << aI1;
				else if (j == 20)
					cout << aI1;
				else if (!(j % 2))
					cout << aI2;
				else
					cout << a_2;
		}
		cout << endl;
		indx = 0;
		if (!(i % 2))
			++indy;
	}
	cout << ' ' << ' ';
	for (int i = 0; i < 19; ++i)
		cout << a_1;
	cout << endl;
}

//Вывод на экран 2х полей
void output_field(field f1, field f2)
{
	int indx1 = 0, indy = 0, z = 3, indx2 = 0, ind = 0, ind2 = 0;
	//Первое поле
	cout << ' '<< ' ';
	for (int i = 0; i < 19; ++i)
		if (!(i % 2))
		{
			++ind;
			cout << ind;
		}
		else
			cout << ' ';
	ind = 0;
	//Второе поле
	for (int l = 0; l < z; ++l)
		cout << '\t';
	cout << ' '<< ' ';
	for (int i = 0; i < 19; ++i)
		if (!(i % 2))
		{
			++ind;
			cout << ind;
		}
		else
			cout << ' ';
	//Первое поле
	cout << endl << ' ' << ' ';
	for (int i = 0; i < 19; ++i)
		cout << a_1;
	//Второе поле
	for (int l = 0; l < z; ++l)
		cout << '\t';
	cout << ' ' << ' ';
	for (int i = 0; i < 19; ++i)
		cout << a_1;
	//Первое поле
	cout << endl;
	ind = 0;
	for (int i = 0; i < 19; ++i)
	{
		if (!(i % 2))
		{
			//Первое поле
			for (int j = 0; j < 22; ++j)
				if (!j)
				{
					cout << char(A + ind);
					++ind;
				}
				else if (j % 2)
					cout << aI1;
				else
				{
					if (f1.death_f[indx1][indy])
						cout << aS3;
					else if (f1.f[indx1][indy] && !f1.strike[indx1][indy])
						cout << aS1;
					else if (f1.f[indx1][indy] && f1.strike[indx1][indy])
						cout << aS2;
					else if (f1.strike[indx1][indy])
						cout << aN1;
					else
						cout << ' ';
					++indx1;
				}
			//Второе поле
			for (int l = 0; l < z; ++l)
				cout << '\t';
			for (int j = 0; j < 22; ++j)
				if (!j)
				{
					cout << char(A + ind2);
					++ind2;
				}
				else if (j % 2)
					cout << aI1;
				else
				{
					if (f2.death_f[indx2][indy])
						cout << aS3;
					else if (f2.f[indx2][indy] && !f2.strike[indx2][indy])
						cout << ' '; //aS1 ' '
					else if (f2.f[indx2][indy] && f2.strike[indx2][indy])
						cout << aS2;
					else if (f2.strike[indx2][indy])
						cout << aN1;
					else
						cout << ' ';
					++indx2;
				}
		}
		else
		{
			//Первое поле
			for (int j = 0; j < 21; ++j)
				if (!j)
					cout << ' ' << aI1;
				else if (j == 20)
					cout << aI1;
				else if (!(j % 2))
					cout << aI2;
				else
					cout << a_2;
			//Второе поле
			for (int l = 0; l < z; ++l)
				cout << '\t';
			for (int j = 0; j < 21; ++j)
				if (!j)
					cout << ' ' << aI1;
				else if (j == 20)
					cout << aI1;
				else if (!(j % 2))
					cout << aI2;
				else
					cout << a_2;
		}
		cout << endl;
		indx1 = 0;
		indx2 = 0;
		if (!(i % 2))
			++indy;
	}
	//Первое поле
	cout << ' ' << ' ';
	for (int i = 0; i < 19; ++i)
		cout << a_1;
	//Второе поле
	for (int l = 0; l < z; ++l)
		cout << '\t';
	cout << ' ' << ' ';
	for (int i = 0; i < 19; ++i)
		cout << a_1;
	cout << endl;
}

//считывание координат будущего корабля
int read_coor_ship(bool &v, int length)
{
	bool end = 0;
	int x, y, res;
	char a, ax[4];
	if (length > 1)
	{
		cout << "Ручное расположение флота\n"
			"Необходимо расставить 10 кораблей, начиная с самого большого\n"
			"Укажите направление корабля, размера " << length << ", относительно его начала\n"
			"(1 - горизонтальное положение, 0 - вертикальное)\n";
		do
		{
			a = _getch();
		} while (a != '1'&&a != '0');
		v = a - '0';

		cout << "\nТеперь укажите координату начала корабля:\n";
	}
	else
		cout << "Ручное расположение флота\n"
		"Необходимо расставить 10 кораблей, ноачиная с самого большого\n"
		"Укажите координату корабля, размера 1:\n";
	do
	{
		for (int i = 0; i < 4 && !end; ++i)
		{
			if (!i)
			{
				do
				{
					a = _getch();
				} while ((a < 'A' || a > 'J') && (a < 'a' || a > 'j'));
				ax[i] = a;
				cout << a;
			}
			else if (i == 1)
			{
				do
				{
					a = _getch();
				} while (a < '1' || a > '9');
				ax[i] = a;
				cout << a;
			}
			else if (i == 2 && a == '1')
			{
				do
				{
					a = _getch();
				} while (a != '0' && a != 13);
				if (a == '0')
				{
					ax[i] = a;
					cout << a;
				}
				else
				{
					ax[i] = 1;
					end = 1;
				}
			}
			else
			{
				do
				{
					a = _getch();
				} while (a != 13);
				ax[i] = 1;
				end = 1;
			}
		}
		cout << "\nВы потверждаете, что хотите поместить корабль в эту точку?\n(enter - да, backspace - нет)\n";
		do
		{
			a = _getch();
		} while (a != 13 && a != 8);
		end = 0;
	} while (a != 13);
	if (ax[0] >= 'A' && ax[0] <= 'J')
		y = ax[0] - 'A';
	else
		y = ax[0] - 'a';
	if (ax[2] == '0')
		x = 9;
	else
		x = ax[1] - '1';
	res = y * 10 + x;
	return res;
}

//ручная расстановка кораблей
field manual_field()
{
	field fpM = nullfield();
	bool vec = 0, end;
	int xy, x, y, len;
	for (int i = 9; i >= 0; --i)
	{ 
		do
		{
			system("cls");
			output_field(fpM);
			len = lenghtship(i);
			xy = read_coor_ship(vec, len);
			x = xy % 10;
			y = xy / 10;
			end = clearship(fpM, len, vec, x, y);
			if (!end)
			{
				cout << "Корабль не может быть распаложен в этом месте. Повторите попытку:\n";
				Sleep(delay);
			}
		} while (!end);
		createship(fpM, i, len, vec, xy);
	}
	
	return fpM;
}
//Случайный выбор координат
int randomXY(int arrXY[], int length)
{
	int *temparr = new int[length], xy0 = 0, res;
	for (int i = 0; i < length; ++i)
		if (arrXY[i] != -1)
		{
			temparr[xy0] = arrXY[i];
			++xy0;
		}
	xy0 = rand() % xy0;
	res = temparr[xy0];
	delete[] temparr;
	return res;
}

//выбор стратегии
void choise_plan(int plan[]) //проверено
{
	int choise = (rand() % 4) + 1;
	plan[1] = 0;
	plan[2] = 4;
	plan[3] = 8;
	plan[4] = 11;
	plan[5] = 15;
	plan[6] = 19;
	plan[7] = 23;
	plan[8] = 27;
	plan[9] = 32;
	plan[10] = 36;
	plan[11] = 40;
	plan[12] = 44;
	plan[13] = 48;
	plan[14] = 51;
	plan[15] = 55;
	plan[16] = 59;
	plan[17] = 63;
	plan[18] = 67;
	plan[19] = 72;
	plan[20] = 76;
	plan[21] = 80;
	plan[22] = 84;
	plan[23] = 88;
	plan[24] = 91;
	plan[25] = 95;
	plan[26] = 99;
	if (choise <= 2)
	{
		plan[0] = 26;
		if (choise != 1)
		{
			for (int i = 1; i <= plan[0]; ++i)
			{
				if (i <= 3)
					++plan[i];
				else if ((i - 3) <= 5)
					--plan[i];
				else if ((i - 3) <= 10)
					++plan[i];
				else if ((i - 3) <= 15)
					--plan[i];
				else if ((i - 3) <= 20)
					++plan[i];
				else
					--plan[i];
			}
		}
	}
	else
	{
		plan[0] = 24;
		if (choise == 3)
			for (int i = 1; i <= plan[0]; ++i)
			{
				if (i <= 2)
					plan[i] = plan[i + 1] - 2;
				else if ((i - 2) <= 5)
					plan[i] = plan[i + 1] + 2;
				else if ((i - 2) <= 10)
					plan[i] = plan[i + 1] - 2;
				else if ((i - 2) <= 15)
					plan[i] = plan[i + 1] + 2;
				else if ((i - 2) <= 20)
					plan[i] = plan[i + 1] - 2;
				else
					plan[i] = plan[i + 1] + 2;
			}
		else
			for (int i = 1; i <= plan[0]; ++i)
			{
				if (i <= 2)
					plan[i] = --plan[i + 1];
				else if ((i - 2) <= 5)
					plan[i] = ++plan[i + 1];
				else if ((i - 2) <= 10)
					plan[i] = --plan[i + 1];
				else if ((i - 2) <= 15)
					plan[i] = ++plan[i + 1];
				else if ((i - 2) <= 20)
					plan[i] = --plan[i + 1];
				else
					plan[i] = ++plan[i + 1];
			}
	}
}

//разбиение на сектора
void choise_sector(int plan[], tn &turn_0)
{
	turn_0.phaseturn = 0;
	for (int i = 0; i < size_sr; ++i)
		turn_0.sector[i].turnphase = 0;
	for (int i = 1; i <= plan[0]; ++i)
		if ((plan[i] % 10) <= 2 && (plan[i] / 10 <= 2))
		{
			int n = 1;
			turn_0.sector[n].phase[turn_0.sector[n].turnphase] = plan[i];
			++turn_0.sector[n].turnphase;
			turn_0.sector[n].use = 0;
		}
		else if ((plan[i] / 10) <= 2 && (plan[i] % 10 <= 6))
		{
			int n = 5;
			turn_0.sector[n].phase[turn_0.sector[n].turnphase] = plan[i];
			++turn_0.sector[n].turnphase;
			turn_0.sector[n].use = 0;
		}
		else if ((plan[i] / 10) <= 2 && (plan[i] % 10 <= 9))
		{
			int n = 2;
			turn_0.sector[n].phase[turn_0.sector[n].turnphase] = plan[i];
			++turn_0.sector[n].turnphase;
			turn_0.sector[n].use = 0;
		}
		else if ((plan[i] / 10) <= 6 && (plan[i] % 10 <= 2))
		{
			int n = 8;
			turn_0.sector[n].phase[turn_0.sector[n].turnphase] = plan[i];
			++turn_0.sector[n].turnphase;
			turn_0.sector[n].use = 0;
		}
		else if ((plan[i] / 10) <= 6 && (plan[i] % 10 <= 6))
		{
			int n = 0;
			turn_0.sector[n].phase[turn_0.sector[n].turnphase] = plan[i];
			++turn_0.sector[n].turnphase;
			turn_0.sector[n].use = 0;
		}
		else if ((plan[i] / 10) <= 6 && (plan[i] % 10 <= 9))
		{
			int n = 6;
			turn_0.sector[n].phase[turn_0.sector[n].turnphase] = plan[i];
			++turn_0.sector[n].turnphase;
			turn_0.sector[n].use = 0;
		}
		else if ((plan[i] / 10) <= 9 && (plan[i] % 10 <= 2))
		{
			int n = 4;
			turn_0.sector[n].phase[turn_0.sector[n].turnphase] = plan[i];
			++turn_0.sector[n].turnphase;
			turn_0.sector[n].use = 0;
		}
		else if ((plan[i] / 10) <= 9 && (plan[i] % 10 <= 6))
		{
			int n = 7;
			turn_0.sector[n].phase[turn_0.sector[n].turnphase] = plan[i];
			++turn_0.sector[n].turnphase;
			turn_0.sector[n].use = 0;
		}
		else if ((plan[i] / 10) <= 9 && (plan[i] % 10 <= 9))
		{
			int n = 3;
			turn_0.sector[n].phase[turn_0.sector[n].turnphase] = plan[i];
			++turn_0.sector[n].turnphase;
			turn_0.sector[n].use = 0;
		}
}//работает

//выбор из 2х вар.
int rand_strike(int arr_strike[])
{
	if (arr_strike[0] != -1 && arr_strike[1] != -1)
	{
		int randz = rand() % 2;
		return arr_strike[randz];
	}
	else if (arr_strike[0] != -1)
		return arr_strike[0];
	else if (arr_strike[1] != -1)
		return arr_strike[1];
	else
		return 0;
} //работает

 //создание цепочки действий
void next_sector(tn &sec)
{
	int temp_ph = sec.phaseturn, z;
	int temp[2];
	if (temp_ph == 1)
	{
		if (!sec.sector[5].use)
			temp[0] = 5;
		else
			temp[0] = -1;
		if (!sec.sector[8].use)
			temp[1] = 8;
		else
			temp[1] = -1;
		z = rand_strike(temp);
	}
	else if (temp_ph == 2)
	{
		if (!sec.sector[5].use)
			temp[0] = 5;
		else
			temp[0] = -1;
		if (!sec.sector[6].use)
			temp[1] = 6;
		else
			temp[1] = -1;
		z = rand_strike(temp);
	}
	else if (temp_ph == 3)
	{
		if (!sec.sector[7].use)
			temp[0] = 7;
		else
			temp[0] = -1;
		if (!sec.sector[6].use)
			temp[1] = 6;
		else
			temp[1] = -1;
		z = rand_strike(temp);
	}
	else if (temp_ph == 4)
	{
		if (!sec.sector[7].use)
			temp[0] = 7;
		else
			temp[0] = -1;
		if (!sec.sector[8].use)
			temp[1] = 8;
		else
			temp[1] = -1;
		z = rand_strike(temp);
	}
	else if (temp_ph == 5)
	{
		if (!sec.sector[1].use)
			temp[0] = 1;
		else
			temp[0] = -1;
		if (!sec.sector[2].use)
			temp[1] = 2;
		else
			temp[1] = -1;
		z = rand_strike(temp);
	}
	else if (temp_ph == 6)
	{
		if (!sec.sector[2].use)
			temp[0] = 2;
		else
			temp[0] = -1;
		if (!sec.sector[3].use)
			temp[1] = 3;
		else
			temp[1] = -1;
		z = rand_strike(temp);
	}
	else if (temp_ph == 7)
	{
		if (!sec.sector[4].use)
			temp[0] = 4;
		else
			temp[0] = -1;
		if (!sec.sector[3].use)
			temp[1] = 3;
		else
			temp[1] = -1;
		z = rand_strike(temp);
	}
	else if (temp_ph == 8)
	{
		if (!sec.sector[4].use)
			temp[0] = 4;
		else
			temp[0] = -1;
		if (!sec.sector[1].use)
			temp[1] = 1;
		else
			temp[1] = -1;
		z = rand_strike(temp);
	}
	sec.sector[sec.phaseturn].next = z;
	sec.sector[sec.phaseturn].use = 1;
	if (!z)
		sec.phaseturn = -1;
	else
		sec.phaseturn = z;
} // работает

 //создание стратегии() 
void create_plan(tn &t_0)
{
	int sector_strike[size_sr];
	t_0.phaseturn = rand() % 4 + 1;
	t_0.sector[0].next = rand() % 4 + 1;
	t_0.sector[0].use = 1;
	t_0.phaseturn = t_0.sector[0].next;
	nullarr(sector_strike, size_sr);
	while (t_0.phaseturn != -1)
	{
		next_sector(t_0);
	}
	t_0.phaseturn = 0;
}

//подготовка ИИ к новой игре
void AI_newgame(tn &turn0)
{
	int strikeplan_1[30];
	choise_plan(strikeplan_1);
	choise_sector(strikeplan_1, turn0);
	create_plan(turn0);
}

//выбор клетки из сектора
int stike_sector(tn &turn_1) // работает
{
	int strike = rand() % (turn_1.sector[turn_1.phaseturn].turnphase);
	int res = turn_1.sector[turn_1.phaseturn].phase[strike];
	for (int i = strike; i < turn_1.sector[turn_1.phaseturn].turnphase; ++i)
		turn_1.sector[turn_1.phaseturn].phase[i] = turn_1.sector[turn_1.phaseturn].phase[i + 1];
	--turn_1.sector[turn_1.phaseturn].turnphase;
	return res;
}

//Поиск корабля игрока, путем уничтожения соседних клеток
int search_ship(tn &turn00, field &fp1)
{
	int xy = turn00.arrXY[0], randxy[4];
	if ((xy % 10 < 9) && (!fp1.strike[xy % 10 + 1][xy / 10]))
		randxy[0] = xy + 1;
	else
		randxy[0] = -1;
	if ((xy % 10 > 0) && (!fp1.strike[xy % 10 - 1][xy / 10]))
		randxy[1] = xy - 1;
	else
		randxy[1] = -1;
	if ((xy / 10 < 9) && (!fp1.strike[xy % 10][xy / 10 + 1]))
		randxy[2] = xy + 10;
	else
		randxy[2] = -1;
	if ((xy / 10 > 0) && (!fp1.strike[xy % 10][xy / 10 - 1]))
		randxy[3] = xy - 10;
	else
		randxy[3] = -1;
	xy = randomXY(randxy, 4);
	return xy;
}

//добивание корабля
int finish_off_ship(tn &turnS, field &fpS)
{
	int xy, r = 0, Rarr[2];

	for (int i = 0; i < turnS.hitship; ++i)
		for (int j = 0; j < i; ++j)
		{
			int temp = turnS.arrXY[i];
			if (temp < turnS.arrXY[j])
			{
				turnS.arrXY[i] = turnS.arrXY[j];
				turnS.arrXY[j] = temp;
				j = 0;
			}
		}

	if (turnS.supposed_vector)
	{
		if (((turnS.arrXY[turnS.hitship - 1] % 10) < 9) && !(fpS.strike[(turnS.arrXY[turnS.hitship - 1] + 1) % 10][turnS.arrXY[turnS.hitship - 1] / 10]))
		{
			++r;
			Rarr[0] = (turnS.arrXY[(turnS.hitship - 1)]) + 1;
		}
		else
			Rarr[0] = -1;
		if (((turnS.arrXY[0] % 10) > 0) && !(fpS.strike[(turnS.arrXY[0] - 1) % 10][turnS.arrXY[0] / 10]))
		{
			++r;
			Rarr[1] = turnS.arrXY[0] - 1;
		}
		else
			Rarr[1] = -1;
	}
	else
	{
		if (((turnS.arrXY[turnS.hitship - 1] / 10) < 9) && !(fpS.strike[turnS.arrXY[turnS.hitship - 1] % 10][(turnS.arrXY[turnS.hitship - 1] + 10) / 10]))
		{
			++r;
			Rarr[0] = turnS.arrXY[turnS.hitship - 1] + 10;
		}
		else
			Rarr[0] = -1;
		if (((turnS.arrXY[0] / 10) > 0) && !(fpS.strike[turnS.arrXY[0] % 10][(turnS.arrXY[0] - 10) / 10]))
		{
			++r;
			Rarr[1] = turnS.arrXY[0] - 10;
		}
		else
			Rarr[1] = -1;
	}
	xy = randomXY(Rarr, 2);
	turnS.arrXY[turnS.hitship] = xy;
	return xy;
}

//случайный выбор координат
int rand_coor(field fpR)
{
	int res, r = 0;
	for (int i = 0; i < X0; ++i)
		for (int j = 0; j <Y0; ++j)
			if (!fpR.strike[i][j])
			{
				int temp = rand();
				if (temp > r)
				{
					res = j * 10 + i;
					r = temp;
				}
				else if (temp == r)
				{
					temp = rand();
					if (temp > r)
					{
						res = j * 10 + i;
						r = temp;
					}
				}
			}
	return res;
}

// Проверка на окончание стратегии
bool strategy_end(tn &turn2)
{
	for (int i = 0; i < size_sr; ++i)
		if (turn2.sector[i].turnphase)
			return 0;
	return 1;
}

//Получение координат от ИИ
int AI_fight(tn &turnf, field &fp1)
{
	int t = turnf.phaseturn, xy;
	bool end = 0;
	if (!turnf.endstrat && !turnf.hitship)
	{
		while (!end)
			if (turnf.sector[t].turnphase)
			{
				do
				{

					xy = stike_sector(turnf);
					if (!turnf.sector[t].turnphase)
					{
						t = turnf.sector[t].next;
						turnf.phaseturn = t;
						end = strategy_end(turnf);
					}
				} while (fp1.strike[xy % 10][xy / 10] && !end);
				end = 1;
			}
			else
			{
				t = turnf.sector[t].next;
				turnf.phaseturn = t;
			}
	}
	else if (!turnf.hitship)
	{
		xy = rand_coor(fp1);
	}
	else
	{
		if (turnf.hitship == 1)
			xy = search_ship(turnf, fp1);
		else
			xy = finish_off_ship(turnf, fp1);
	}
	return xy;
}

//Поиск направления недобитого корабля по двум элементам
bool search_vec(tn &turnM)
{
	if (turnM.arrXY[0] % 10 == turnM.arrXY[1] % 10)
		return 0;
	else
		return 1;
}

//Уничтожение корабля для ИИ
void des_ship(tn &turnD, field &fD, int ind)
{

	int whol = 0;
	for (int i = 0; i < fD.fleet[ind].length; ++i)
		whol += fD.fleet[ind].segments[i].whole;
	if (!whol)
	{	
		for (int i = 0; i < fD.fleet[ind].length; ++i)
		fD.death_f[fD.fleet[ind].segments[i].x][fD.fleet[ind].segments[i].y] = 1;
		--fD.n;
		fD.fleet[ind].destroy = 1;
		turnD.hitship = 0;
		for (int i = 0; i < fD.fleet[ind].length; ++i)
		{
			if (fD.fleet[ind].segments[i].x > 0)
				fD.strike[fD.fleet[ind].segments[i].x - 1][fD.fleet[ind].segments[i].y] = 1;
			if (fD.fleet[ind].segments[i].x < 9)
				fD.strike[fD.fleet[ind].segments[i].x + 1][fD.fleet[ind].segments[i].y] = 1;
			if (fD.fleet[ind].segments[i].y < 9)
				fD.strike[fD.fleet[ind].segments[i].x][fD.fleet[ind].segments[i].y + 1] = 1;
			if (fD.fleet[ind].segments[i].y > 0)
				fD.strike[fD.fleet[ind].segments[i].x][fD.fleet[ind].segments[i].y - 1] = 1;
			if ((fD.fleet[ind].segments[i].x > 0) && (fD.fleet[ind].segments[i].y > 0))
				fD.strike[fD.fleet[ind].segments[i].x - 1][fD.fleet[ind].segments[i].y - 1] = 1;
			if ((fD.fleet[ind].segments[i].x > 0) && (fD.fleet[ind].segments[i].y < 9))
				fD.strike[fD.fleet[ind].segments[i].x - 1][fD.fleet[ind].segments[i].y + 1] = 1;
			if ((fD.fleet[ind].segments[i].x < 9) && (fD.fleet[ind].segments[i].y < 9))
				fD.strike[fD.fleet[ind].segments[i].x + 1][fD.fleet[ind].segments[i].y + 1] = 1;
			if ((fD.fleet[ind].segments[i].x < 9) && (fD.fleet[ind].segments[i].y > 0))
			fD.strike[fD.fleet[ind].segments[i].x + 1][fD.fleet[ind].segments[i].y - 1] = 1;
		}
	}

}

//Уничтожение корабля для игрока
void des_ship(field &fD, int ind)
{
	
	int whol = 0;
	for (int i = 0; i < fD.fleet[ind].length; ++i)
		whol += fD.fleet[ind].segments[i].whole;
	if (!whol)
	{
		--fD.n;
		fD.fleet[ind].destroy = 1;
		for (int i = 0; i < fD.fleet[ind].length; ++i)
			fD.death_f[fD.fleet[ind].segments[i].x][fD.fleet[ind].segments[i].y] = 1;
		for (int i = 0; i < fD.fleet[ind].length; ++i)
		{
			if (fD.fleet[ind].segments[i].x > 0)
				fD.strike[fD.fleet[ind].segments[i].x - 1][fD.fleet[ind].segments[i].y] = 1;
			if (fD.fleet[ind].segments[i].x < 9)
				fD.strike[fD.fleet[ind].segments[i].x + 1][fD.fleet[ind].segments[i].y] = 1;
			if (fD.fleet[ind].segments[i].y < 9)
				fD.strike[fD.fleet[ind].segments[i].x][fD.fleet[ind].segments[i].y + 1] = 1;
			if (fD.fleet[ind].segments[i].y > 0)
				fD.strike[fD.fleet[ind].segments[i].x][fD.fleet[ind].segments[i].y - 1] = 1;
			if ((fD.fleet[ind].segments[i].x > 0) && (fD.fleet[ind].segments[i].y > 0))
				fD.strike[fD.fleet[ind].segments[i].x - 1][fD.fleet[ind].segments[i].y - 1] = 1;
			if ((fD.fleet[ind].segments[i].x > 0) && (fD.fleet[ind].segments[i].y < 9))
				fD.strike[fD.fleet[ind].segments[i].x - 1][fD.fleet[ind].segments[i].y + 1] = 1;
			if ((fD.fleet[ind].segments[i].x < 9) && (fD.fleet[ind].segments[i].y < 9))
				fD.strike[fD.fleet[ind].segments[i].x + 1][fD.fleet[ind].segments[i].y + 1] = 1;
			if ((fD.fleet[ind].segments[i].x < 9) && (fD.fleet[ind].segments[i].y > 0))
				fD.strike[fD.fleet[ind].segments[i].x + 1][fD.fleet[ind].segments[i].y - 1] = 1;
		}
	}

}

//Нанесение удара на поле для ИИ
bool strike_field(tn &turnS, field &fpS, int x, int y)
{
	bool end = 0;
	int i;
	if (fpS.strike[x][y])
	{
		cout << "ERROR" << ' ' << y << x << endl;
	}
	fpS.strike[x][y] = 1;
	if (fpS.f[x][y])
	{
		turnS.arrXY[turnS.hitship] = y * 10 + x;
		++turnS.hitship;
		for (i = 9; i >= 0 && !end; --i)
		{
			for (int j = 0; j < fpS.fleet[i].length; ++j)
			{
				if (x == fpS.fleet[i].segments[j].x && y == fpS.fleet[i].segments[j].y)
				{
					fpS.fleet[i].segments[j].whole = 0;
					end = 1;
				}
			}
		}
		++i;
		des_ship(turnS, fpS, i);
		return 1;
	}
	return 0;
}

//Нанесение удара на поле для игрока
bool strike_field(field &fpS, int x, int y)
{
	bool end = 0;
	int i;
	if (fpS.strike[x][y])
	{
		cout << "ERROR" << ' ' << y << x << endl;
	}
	fpS.strike[x][y] = 1;
	if (fpS.f[x][y])
	{
		for (i = 9; i >= 0 && !end; --i)
		{
			for (int j = 0; j < fpS.fleet[i].length; ++j)
			{
				if (x == fpS.fleet[i].segments[j].x && y == fpS.fleet[i].segments[j].y)
				{
					fpS.fleet[i].segments[j].whole = 0;
					end = 1;
				}
			}
		}
		++i;
		des_ship(fpS, i);
		return 1;
	}
	return 0;
}

//Ход ИИ
bool AI_turn(tn &turnM, field &fpM)
{
	int xy = AI_fight(turnM, fpM), x1 = xy % 10, y1 = xy / 10;
	bool try_s = strike_field(turnM, fpM, x1, y1);
	turnM.endstrat = strategy_end(turnM);
	if (turnM.hitship)
	{
		if (turnM.hitship > 1)
			turnM.supposed_vector = search_vec(turnM);
	}
	return try_s;
}

//считывание координат
int read_coor()
{
	bool end = 0;
	char a, ax[4];
	int y, x, res, temp = 0;
	do
	{
		for (int i = 0; i < 4 && !end; ++i)
		{
			if (!i)
			{
				do
				{
					a = _getch();
				} while ((a < 'A' || a > 'J') && (a < 'a' || a > 'j'));
				ax[i] = a;
				cout << a;
			}
			else if (i == 1)
			{
				do
				{
					a = _getch();
				} while (a < '1' || a > '9');
				ax[i] = a;
				cout << a;
			}
			else if (i == 2 && a == '1')
			{
				do
				{
					a = _getch();
				} while (a != '0' && a != 13);
				if (a == '0')
				{
					ax[i] = a;
					cout << a;
				}
				else
				{
					ax[i] = 1;
					end = 1;
				}
			}
			else
			{
				do
				{
					a = _getch();
				} while (a != 13);
				ax[i] = 1;
				end = 1;
			}
		}
		cout << "\nВы потверждаете, что хотите нанести удар в эту точку?\n(enter - да, backspace - нет)\n";
		do
		{
			a = _getch();
		} while (a != 13 && a != 8);
		end = 0;
	} while (a != 13);
	if (ax[0] >= 'A' && ax[0] <= 'J')
		y = ax[0] - 'A';
	else 
		y = ax[0] - 'a';
	if (ax[2] == '0')
		x = 9;
	else
		x = ax[1] - '1';
	res = y * 10 + x;
	return res;
}

//Ход игрока
bool Pr_turn(field &fAIP)
{
	int xy, x , y;
	bool rep;
	do
	{
		xy = read_coor();
		x = xy % 10;
		y = xy / 10;
		if (fAIP.strike[x][y])
			cout << "Эта точка уже атакована, выберите другую\n";
	} while (fAIP.strike[x][y]);
	rep = strike_field(fAIP, x, y);
	return rep;
}

//Начало новой игры
void new_game(field &fp1, field &fAI2, tn &turnNG, int nomb)
{
	fAI2 = randomfleet();
	if (nomb == 1)
		fp1 = manual_field();
	else
		fp1 = randomfleet();
	AI_newgame(turnNG);
}

//Выбор пункта главного меню
int menu_main_read()
{
	char a;
	do
	{
		a = _getch();
	} while (a != '1' && a != '2' && a != '3');
	int res = a - '0';
	return res;
}

//Выбор пункта меню новой игры
int menu_newgame_read()
{
	char a;
	do
	{
		a = _getch();
	} while (a != '1' && a != '2');
	int res = a - '0';
	return res;
}

//главное меню
int menu_main()
{
	cout << "Le ultimato battleship extraordinaire приветствует вас!\n"
		"Выберите необходимый пункт меню нажатием соответствующей цифры:\n"
		"1. Новая игра\n"
		"2. Справка\n"
		"3. Выход\n";
	return menu_main_read();
}

//Меню новой игры
int menu_newgame()
{
	system("cls");
	cout << "Новая игра\nРасстановка кораблей\n"
		"Выберите необходимый пункт меню нажатием соответствующей цифры:\n"
		"1. Расставить корабли вручную\n"
		"2. Расставить случайно\n";
	return menu_newgame_read();
}

//функция, описывающия игровые ходы
bool gen_step(tn &turnG, field &fpG, field &fAIG)
{
	bool repeat;
	int r;
	if (turnG.numb == 1)
	{
		r = rand() % 2; //1 - первый ход игрока; 0 - первый ход компьютера
		turnG.p_turn = r;
	}
	if(turnG.numb == 1 && !r)
	{
		do
		{
			system("cls");
			output_field(fpG, fAIG);
			cout << "\n\tХод компьютера\n";
			Sleep(delay);
			repeat = AI_turn(turnG, fpG);
			if (!repeat)
				++turnG.numb;
		} while (repeat && fpG.n);
	}
	else
		if (turnG.numb % 2 == turnG.p_turn)
		{
			do
			{
				system("cls");
				output_field(fpG, fAIG);
				cout << "\n\tВаш ход. Введите координату для атаки. Например: B2\n";
				repeat = Pr_turn(fAIG);
				if (!repeat)
					++turnG.numb;
				
			} while (repeat && fAIG.n);
		}
		if (turnG.numb % 2 != turnG.p_turn)
		{
			do
			{
				system("cls");
				output_field(fpG, fAIG);
				cout << "\n\tХод компьютера\n";
				Sleep(delay);
				repeat = AI_turn(turnG, fpG);
				if (!repeat)
					++turnG.numb;
			} while (repeat && fpG.n);
		}
	
	if (!fAIG.n || !fpG.n)
	{
		if (!fAIG.n)
			cout << "\nВы победили!!))0\n";
		else
			cout << "\nВы проиграли((9\n";
		_getch();
		return 1;
	}
	return 0;
}

//Справка
void menu_reference()
{
	system("cls");
	cout << "Вас приветствует справка игры \"Морской бой\"!\n"
		"Обозначение на полях:\n"
		"1. " << aS1 << " - обозначается целый сегмент корабля\n"
		"2. " << aS2 << " - обозначается поврежденный сегмент корабля\n"
		"3. " << aS3 << " - обозначается сегмент потопленного корабля\n"
		"4. " << aN1 << " - обозначается промах\n"
		"Координаты\nКоординаты следует указывать начиная заглавной или прописной буквой.\n"
		"Буквы: от a до j.\nПосле указания буквы, необходимо указать цифру, от 1 до 10\n"
		"Примеры ввода координат: A7, a6, j10, J1.\n"
		"Для комфортной игры советуется немного оттянуть окно командной строки вниз.\n"
		"Приятной игры!\n Что-бы выйти из справки нажмите любую клавишу";
	_getch();
}

//объединяющая функция
void vvfunc()
{
	field fp = nullfield(), fAI = nullfield();
	tn turn = nullturn();
	bool end = 0;
	do
	{
		int n = menu_main();
		switch (n)
		{
		case 1:
			new_game(fp, fAI, turn, menu_newgame());
			while (!gen_step(turn, fp, fAI));
			break;
		case 2:
			menu_reference();
			break;
		case 3:
			end = 1;
			break;
		}
		system("cls");
	} while (!end);
}

//
bool endfunc()
{
	cout << "\nДля выхода из программы нажмите esc или нажмите любую другую клавишу для повтора\n";
	bool e = _getch() == 27;
	if (e == 0)
		system("cls");
	return e;
}

//
int main()
{
	setlocale(0, "");
	srand(time(0));
	do
	{
		vvfunc();
	} while (endfunc() == 0);
}