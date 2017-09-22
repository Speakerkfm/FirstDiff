// FirstDiff.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <math.h>
#include <fstream>

using namespace std;

struct TNode //структура дерева выражения
{
	string Data;
	TNode* Left;
	TNode* Right;
	bool PeremExist;
};

typedef TNode* PNode;

string Perem; //переменная, от которой вычисляется функция f(Perem)
bool err = false;

int skip(string s, int i) //функция пропуска пробела, обход строки с коца, возвращает i перед скобкой
{
	int k = 1;
	i--;
	while (i >= 0 && k > 0)
	{
		if (s[i] == ')')
			k++;
		if (s[i] == '(')
			k--;
		i--;
	}
	return i;
}

bool IsOper(string oper, int level) //проверка, является ли операция заданной (с учетом уровня)
{
	switch (level)
	{
	    case 1: return oper == "+" || oper == "-";
	    case 2: return oper == "*" || oper == "/";
	    case 3: return oper == "^";
	}
}

bool IsDigit(string s) //проверка, является ли строка числом
{
	if (s.length() == 0)
		return false;
	for (int i = 0; i < s.length(); i++)
		if (s[i] < '0' || s[i] > '9')
			return false;
	return true;
}

bool IsOperation(string s) //проверка, является ли операция заданной
{
	return s == "+" || s == "-" || s == "*" || s == "/" || s == "^";
}

bool IsFunction(string s)
{
	return s == "exp" || s == "sin" || s == "cos" || s == "tg" || s == "ln" || s == "ctg" || s == "arcsin" || s == "arccos" || s == "arctg" || s == "arcctg";
}

bool IsConst(string s) //проверка, является ли строка переменной(латинские заглавные и строчные буквы)
{
	if (s == Perem || IsFunction(s))
		return false;
	for (int i = 0; i < s.length(); i++)
		if ((s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'))
			return true;
	return false;
}

bool IsUnarnMinus(PNode Root) //проверка, является ли минус унарным (унарный минус представляется, как (0-x))
{
	if (Root != NULL && Root->Right != NULL && Root->Right->Left != NULL)
		return (Root->Data == "()" && Root->Right->Data == "-" && Root->Right->Left->Data == "0");
	else
		return false;
}

bool ExceptionInBrackets(string s) //проверка, находится ли выражение в скобках
{
	if (s.length() > 2)
		return s[s.length() - 1] == ')' && skip(s, s.length() - 1) == -1;
	else
		return false;
}

void Parse(string s, string &Left, string &Right, string &oper, int level) //пытается разбить строку на 2 подстроки, разделенных операцией заданного уровня
{
	int i = s.length() - 1;
	while (i >= 0 && !IsOper(s.substr(i, 1), level)) //пропускает выражения в скобках
	{
		if (s[i] == ')')
			i = skip(s, i);
		else
			i--;
	}
	if (i >= 0 && IsOper(s.substr(i, 1), level))
	{
		oper = s[i];
		if (i != 0)
			Left = s.substr(0, i);
		else
			Left = "0";
		if (i != s.length() - 1)
			Right = s.substr(i + 1);
		else
			Right = "0";
	}
}

void TryFunction(string s, string &Right, string &Left, string &oper) //определяет, является ли строка сложной функцией
{
	if (s.length() < 4) return;
	if (s.substr(0, 2) == "ln")
		oper = "ln";
	if (s.substr(0, 3) == "exp")
		oper = "exp";
	if (s.substr(0, 3) == "sin")
		oper = "sin";
	if (s.substr(0, 3) == "cos")
		oper = "cos";
	if (s.substr(0, 2) == "tg")
		oper = "tg";
	if (s.substr(0, 3) == "ctg")
		oper = "ctg";
	if (s.substr(0, 3) == "arc")
	{
		if (s.substr(0, 5) == "arctg")
			oper = "arctg";
		if (s.substr(0, 6) == "arcctg")
			oper = "arcctg";
		if (s.substr(0, 6) == "arcsin")
			oper = "arcsin";
		if (s.substr(0, 6) == "arccos")
			oper = "arccos";
	}
	if (oper != "")
	{
		int i = 0;
		while (s[i] != '(')
			i++;
		int k = i + 1;
		while (s[k] != ')')
			k++;
		Right = s.substr(i); //аргумент функции будет находиться в правом поддереве
		Left = "NULL"; //левое будет пустым
	}
}

string Res(PNode Root) //превращает дерево выражения в строку в инфиксной форме
{
	if (Root != NULL)
	{
		if (Root->Data == "()")
			return "(" + Res(Root->Right) + ")";
		else
		{
			if (Root->Data != "0")
				return Res(Root->Left) + Root->Data + Res(Root->Right);
			else
				return "";
		}
	}
	else
		return "";
}

string Dec(string s) //уменьшаем строковое выражение на единицу
{
	if (IsDigit(s))
		s[s.length() - 1] -= 1;
	else
		s = "(" + s + "-1)";
	return s;
}

//string Inc(string s) //увеличиваем строковое выражение на единицу
//{
//	if (IsDigit(s))
//		s[s.length() - 1] += 1;
//	else
//		s = "(" + s + ")-1";
//	s = "(-" + s + ")";
//	return s;
//}

string Diff(PNode Root) //вычисление дифференциала по заданному дереву выражения, дифференциал всегда ставится перед получаемым выражением, т.к. это многое упрощает
{
	if (Root->Data == "()") //если выражение в скобках, проваливаемся дальше
		return "(" + Diff(Root->Right) + ")";
	if (!Root->PeremExist) //если выражение цифра, возвращаем 0
		return "0";
	if (Root->Data == Perem) //если выражение заданная переменна, возвращаем 1
		return "1";
	if (Root->Data == "+") //производная суммы - сумма производных
		return Diff(Root->Left) + "+" + Diff(Root->Right);
	if (Root->Data == "-") //производная разности - разность производных
		return Diff(Root->Left) + "-" + Diff(Root->Right);
	if (Root->Data == "*") //(u*v)' = u'*v + v'*u
	{
		if (IsDigit(Root->Right->Data) && IsDigit(Root->Left->Data))
			return "0";
		if (!Root->Left->PeremExist) //(IsDigit(Root->Left->Data))
			return Res(Root->Left) + "*" + Diff(Root->Right);
		if (!Root->Right->PeremExist)
			return Res(Root->Right) + "*" + Diff(Root->Left);
		return  "(" + Diff(Root->Left) + "*" + Res(Root->Right) + "+" + Diff(Root->Right) + "*" + Res(Root->Left) + ")";
	}
	if (Root->Data == "/") //(u/v)' = (u'v - v'u)/v^2
	{
		if (!Root->Left->PeremExist)
			return  Diff(Root->Right) + "*" + "(-" + Root->Left->Data + ")/(" + Res(Root->Right) + "^2)";
		if (!Root->Left->PeremExist)
			return Diff(Root->Left) + "/" + Root->Right->Data;
		return "((" + Diff(Root->Left) + "*" + Res(Root->Right) + "-" + Diff(Root->Right)  + "*" + Res(Root->Left) + ")/(" + Res(Root->Right) + ")^2)";
	}
	if (Root->Data == "^") //производная от степени
	{
		/*if (IsUnarnMinus(Root->Right))
			return Res(Root->Right) + "*" + Diff(Root->Left) + "*" + Res(Root->Left) + "^" + Inc(Root->Right->Right->Right->Data);*/
		//if (Root->Right->PeremExist && Root->Left->PeremExist)
		//	return 
		if (!Root->Right->PeremExist)
			return Res(Root->Right) + "*" + Diff(Root->Left) + "*" + Res(Root->Left) + "^" + Dec(Res(Root->Right)); //(x^a)' = a*x^(a-1)
		else
			return Diff(Root->Right) + "*" + Res(Root->Left) + "^" + Res(Root->Right) + "*ln(" + Res(Root->Left) + ")"; //(a^x) = a^x * ln(a)
	}
	if (Root->Data == "ln") //производные сложных функций
		return  "(" + Diff(Root->Right) + "/" + Res(Root->Right) + ")";
	if (Root->Data == "sin")
		return  "(" + Diff(Root->Right) + "*" + "cos" + Res(Root->Right) + ")";
	if (Root->Data == "cos")
		return  "(" + Diff(Root->Right) + "*" + "(-sin" + Res(Root->Right) + "))";
	if (Root->Data == "exp")
		return  "(" + Diff(Root->Right) + "*" + "exp" + Res(Root->Right) + ")";
	if (Root->Data == "tg")
		return "(" + Diff(Root->Right) + "/(" + "cos(" + Res(Root->Right) + ")^2))";
	if (Root->Data == "ctg")
		return "(-" + Diff(Root->Right) + "/(" + "sin(" + Res(Root->Right) + ")^2))";
	if (Root->Data == "arcsin")
		return "(" + Diff(Root->Right) + "/(1-" + Res(Root->Right) + "^2)^(1/2))";
	if (Root->Data == "arccos")
		return "(-" + Diff(Root->Right) + "/(1-" + Res(Root->Right) + "^2)^(1/2))";
	if (Root->Data == "arctg")
		return "(" + Diff(Root->Right) + "/(1+" + Res(Root->Right) + "^2))";
	if (Root->Data == "arcctg")
		return "(-" + Diff(Root->Right) + "/(1+" + Res(Root->Right) + "^2))";
	return "0";
}

string Count(string arg1, string arg2, string oper) //вычисляем строку - значение выражения от заданных аргументов и операции
{
	bool otr = false;
	int a1 = stoi(arg1);
	int a2 = stoi(arg2);
	int result = 0;
	if (oper == "+")
		result = a1 + a2;
	if (oper == "-")
		result = a1 - a2;
	if (oper == "*")
		result = a1 * a2;
	if (oper == "/") //деление только нацело
	{
		if (a1 % a2 == 0)
			result = a1 / a2;
		else
			return arg1 + oper + arg2;
	}
	if (oper == "^")
		result = pow(a1, a2);
	string res = "";
	if (result < 0) //если результат получился отрицательным
	{
		otr = true;
		result *= -1;
	}
	if (result == 0)
		return "0";
	while (result > 0) //переводим результат в строку
	{
		char c = ((result % 10) + '0');
		res = c + res;
		result /= 10;
	}
	if (otr)
	    res = "(-" + res + ")";
	return res;
}

void CreateTree(PNode &Root, string s);

void SimpleTree(PNode &Root);

void UselessZero(PNode &Root) //избавляется от незачащих нулей в дереве
{
	if (Root->Data == "+" && Root->Left->Data == "0") //0+x -> x
	{
		PNode L = Root->Left;
		Root = Root->Right;
		delete L;
	}
	if (Root->Data == "+" && Root->Right->Data == "0") //x+0 -> x
	{
		PNode R = Root->Right;
		Root = Root->Left;
		delete R;
	}
	if (Root->Data == "-" && Root->Right->Data == "0") //x-0 -> x
	{
		delete Root->Right;
		Root = Root->Left;
	}
	if (Root->Data == "*" && Root->Left->Data == "0") //0*x -> 0
	{
		PNode L = Root->Left;
		PNode R = Root->Right;
		Root->Data = "0";
		delete L;
		delete R;
		Root->Right = NULL;
		Root->Left = NULL;
	}
	if (Root->Data == "*" && Root->Right->Data == "0") //x*0 -> 0
	{
		PNode L = Root->Left;
		PNode R = Root->Right;
		Root->Data = "0";
		delete L;
		delete R;
		Root->Right = NULL;
		Root->Left = NULL;
	}
}

void UselessBrackets(PNode &Root) //избавляется от лишних скобок в дереве
{
	if (IsOperation(Root->Data) && Root->Left->Data == "()" && !IsUnarnMinus(Root->Left)) //(x)+x -> x+x 
	{
		if (IsDigit(Root->Left->Right->Data) || IsConst(Root->Left->Right->Data) || Root->Left->Right->Data == Perem)
		{
			PNode L = Root->Left->Left;
			Root->Left = Root->Left->Right;
			delete L;
			SimpleTree(Root);
		}
	}
	if (IsOperation(Root->Data) && Root->Right->Data == "()" && !IsUnarnMinus(Root->Right)) //x+(x) -> x+x
	{
		if (IsDigit(Root->Right->Right->Data) || IsConst(Root->Right->Right->Data) || Root->Right->Right->Data == Perem)
		{
			PNode R = Root->Right->Left;
			Root->Right = Root->Right->Right;
			delete R;
			SimpleTree(Root);
		}
	}
	if (Root->Data == "()" && Root->Right->Data == "()") //((x)) -> (x)
	{
		PNode R = Root->Left;
		Root = Root->Right;
		delete R;
	}
	/*if (Root->Data == "()" && (IsConst(Root->Right->Data) || IsDigit(Root->Right->Data) || Root->Right->Data == Perem))
		Root = Root->Right;*/
}

void UselessOne(PNode &Root) //избавляется от незначащих единиц в дереве
{
	if (Root->Data == "*" && Root->Left->Data == "1") //1*x -> x
	{
		PNode L = Root->Left;
		Root = Root->Right;
		delete L;
	}
	if (IsOper(Root->Data, 2) && Root->Right->Data == "1") //x/1 -> x, x*1 -> x
	{
		PNode R = Root->Right;
		Root = Root->Left;
		delete R;
	}
	if (Root->Data == "^" && Root->Right->Data == "1") //x^1 -> x
	{
		PNode R = Root->Right;
		Root = Root->Left;
		delete R;
	}
}

void SimplePower(PNode &Root) //упрощает степень в дереве
{
	if (Root->Data == "^")
	{
		if (Root->Left->Data == "^") //x^2^5 -> x^10
		{
			PNode R = Root->Right;
			Root->Left->Right->Data = Count(Root->Left->Right->Data, Root->Right->Data, "*");
			Root = Root->Left;
			delete R;
		}
		if (Root->Left->Data == "()" && Root->Left->Right->Data == "^") //(x^2)^5 -> x^10
		{
			Root->Left = Root->Left->Right;
			PNode R = Root->Right;
			Root->Left->Right->Data = Count(Root->Left->Right->Data, Root->Right->Data, "*");
			Root = Root->Left;
			delete R;
		}
	}
}

void UselessUnarnMinus(PNode &Root, string op) //упрощает выражения, содержащие унарный минус
{
	string ChangeOp;
	if (op == "-")
		ChangeOp = "+";
	if (op == "+")
		ChangeOp = "-";
	if (Root->Data == op)
	{
		if (IsUnarnMinus(Root->Right) && !IsUnarnMinus(Root->Left)) //x-(0-x) -> x+x, x+(0-x) -> x-x
		{
			PNode q = Root->Right->Right->Left, p = Root->Right->Right;
			Root->Right = Root->Right->Right->Right;
			Root->Data = ChangeOp;
			delete q;
			delete p;
			SimpleTree(Root);
		}
		if (IsUnarnMinus(Root->Left) && !IsUnarnMinus(Root->Right)) //(0-x)+x -> 0-(x-x), (0-x)-x -> 0-(x+x)
		{
			swap(Root->Left, Root->Right);
			swap(Root->Left, Root->Right->Right->Left);
			Root->Data = "-";
			if (op == "+")
				swap(Root->Right->Right->Left, Root->Right->Right->Right);
			Root->Right->Right->Data = ChangeOp;
			SimpleTree(Root->Right->Right);
		}
		if (IsUnarnMinus(Root->Left) && IsUnarnMinus(Root->Right)) //(0-x)-(0-x) -> 0-(x-x), (0-x)+(0-x) -> 0-(x+x)
		{
			Root->Data = "-";
			swap(Root->Left->Right->Right, Root->Right->Right->Left);
			Root->Left->Data = "0";
			Root->Right->Right->Data = op;
			delete Root->Left->Right->Left;
			delete Root->Left->Right->Right;
			delete Root->Left->Right;
			Root->Left->Left = NULL;
			Root->Left->Right = NULL;
			SimpleTree(Root);
		}
	}
}

void TryCount(PNode &Root) //пытается посчитать значение выражения в дереве
{
	if (Root->Data == "-" && Root->Left->Data == "0")
		return;
	if (IsOperation(Root->Data) && IsDigit(Root->Left->Data) && IsDigit(Root->Right->Data)) //если возможно вычислить, вычисляем
	{
		string s = Count(Root->Left->Data, Root->Right->Data, Root->Data);
		delete Root->Left;
		delete Root->Right;
		Root->Data = s;
		Root->Left = NULL;
		Root->Right = NULL;
		//CreateTree(Root, s);
	}
}

void UselessDoubleUnarnMinus(PNode &Root) //избавляется от двойного унарного минуса
{
	if (Root->Data == "-" && Root->Left->Data == "0" && IsUnarnMinus(Root->Right)) //0-(0-x) -> x
	{
		delete Root->Left;
		PNode q = Root->Right, p = q->Right;
		Root = Root->Right->Right->Right;
		delete q;
		delete p;
	}
}

void UselessMultMinus(PNode &Root) //избавляется от минуса в произведении
{
	if (IsOper(Root->Data, 2) && IsUnarnMinus(Root->Left) && IsUnarnMinus(Root->Right)) //(0-x)*(0-x) -> (0+x)*(0+x)
	{
		Root->Left->Right->Data = "+";
		Root->Right->Right->Data = "+";
		SimpleTree(Root);
		SimpleTree(Root->Right);
		SimpleTree(Root->Left);
	}
}

void SimpleTree(PNode &Root) //процедура упрощения дерева
{
	if (Root != NULL)
	{
		UselessZero(Root);
		UselessBrackets(Root);
		UselessOne(Root);
		SimplePower(Root);
		UselessUnarnMinus(Root, "+");
		UselessUnarnMinus(Root, "-");
		UselessDoubleUnarnMinus(Root);
		UselessMultMinus(Root);
		TryCount(Root);
	}
}

void CreateTree(PNode &Root, string s) //процедура создания дерева выражения
{
	if (s == "") return;
	if (s == "NULL")
	{
		Root = NULL;
		return;
	}
	string oper = "";
	string Left = "", Right = "";
	if (ExceptionInBrackets(s)) //опускаем скобки
	{
		oper = "()";
		Right = s.substr(1, s.length() - 2);
		Left = "NULL";
	}
	for (int l = 1; l <= 3 && oper == ""; l++) //пытаемся найти простую операцию
		Parse(s, Left, Right, oper, l);
	if (oper == "")
		TryFunction(s, Right, Left, oper); //если не нашли, пытаемся найти сложную операцию
	if (oper != "") //если операция найдена, создаем дерево
	{
		Root = new TNode;
		Root->Data = oper;
		CreateTree(Root->Left, Left);
		CreateTree(Root->Right, Right);
		if ((Root->Left != NULL && Root->Left->PeremExist) || (Root->Right != NULL && Root->Right->PeremExist))
			Root->PeremExist = true;
		else
			Root->PeremExist = false;
	}
	else //иначе, если строка не NULL, записываем в дерево в том же виде
	{
		Root = new TNode;
		if (s != "NULL")
			Root->Data = s;
		if (s == Perem)
			Root->PeremExist = true;
		else
			Root->PeremExist = false;
		Root->Left = NULL;
		Root->Right = NULL;
	}
	SimpleTree(Root);
}

void PrintTree(PNode Root, int lvl) //функция распечатывания дерева (для проверки)
{
	if (Root != NULL)
	{
		PrintTree(Root->Left, lvl + 1);
		for (int i = 0; i < lvl; i++)
			cout << "   ";
		cout << Root->Data << endl;
		PrintTree(Root->Right, lvl + 1);
	}
}

PNode DelTree(PNode &Root) //процедура освобождения памяти из под дерева
{
	if (Root != NULL)
	{
		delete (DelTree(Root->Left));
		delete (DelTree(Root->Right));
	}
	return Root;
}

bool CheckInputArg(string s) //проверка входных данных (переменная)
{
	if (IsFunction(s) || IsOperation(s))
		err = true;
	bool P = false;
	for (int i = 0; i < s.length() && !err; i++)
	{
		if (s[i] == ' ')
			s.erase(i, 1);
		if (s[i] >= 'a' && s[i] <= 'z')
			P = true;
		if (s[i] >= 'A' && s[i] <= 'Z')
			P = true;
		if ((s[i] < 'a' || s[i] > 'z') && (s[i] < 'A' || s[i] > 'Z') && (s[i] < '0' || s[i] > '9'))
			err = true;
	}
	if (!P)
		err = true;
	return err;
}

bool CheckInputF(string &s) //проверка входных данных (выражение)
{
	int opened = 0;
	int closed = 0;
	for (int i = 0; i < s.length() && !err; i++)
	{
		if (s[i] == ' ')
			s.erase(i, 1);
		if (s[i] == ')')
			closed++;
		if (s[i] == '(')
			opened++;
		if ((s[i] < 'a' || s[i] > 'z') && (s[i] < 'A' || s[i] > 'Z') && (s[i] < '0' || s[i] > '9') && !IsOperation(s.substr(i, 1)) && s[i] != '(' && s[i] != ')')
			err = true;
	}
	while (opened > closed)
	{
		s += ")";
		closed++;
	}
	while (closed > opened)
	{
		s = "(" + s;
		opened++;
	}
	return err;
}

int main() //основная программа
{
	ifstream fi("input.txt");
	ofstream fo("output.txt");
	string s;
	getline(fi, Perem);
	cout << "Argument = " + Perem << endl;
	//getline(cin, Perem);
	if (CheckInputArg(Perem))
	{
		cout << "Error" << endl;
		system("pause");
		return 0;
	}
	getline(fi, s);
	cout << endl << "f(" + Perem + ") = " + s << endl;
	//getline(cin, s);
	if (CheckInputF(s))
	{
		cout << "Error" << endl;
		system("pause");
		return 0;
	}
	PNode Root = NULL, ResultRoot = NULL;
	CreateTree(Root, s);
	CreateTree(ResultRoot, Diff(Root));
	//PrintTree(Root, 0);
	delete (DelTree(Root));
	string Result = Res(ResultRoot);
	delete (DelTree(ResultRoot));
	if (ExceptionInBrackets(Result))
	    Result = Result.substr(1, Result.length() - 2);
	if (Result == "" || Result == "()" || IsOperation(Result) || Result == "-()")
		Result = "0";
	cout << endl << "f'(" + Perem + ") = " + Result << endl;
	fo << Result;
	fo.close();
	fi.close();
	//PNode Result2Root = NULL;
	//CreateTree(Result2Root, Diff(ResultRoot));
	//string Result2 = Res(Result2Root);
	//if (ExceptionInBrackets(Result2))
	//	Result2 = Result2.substr(1, Result2.length() - 2);
	//if (Result2 == "" || Result == "()" || IsOperation(Result2) || Result2 == "-()")
	//	Result2 = "0";
	//cout << "f''(" + Perem + ") = " + Result2 << endl;
	system("pause");
	return 0;
}


