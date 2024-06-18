#include <iostream>
#include <fstream>
#include <cstdio>
#include <thread>
#include <vector>
#include <future>
#include <random>


using namespace std;

//реализован класс Timer для автоматизации процесса подсчета времени выполнения функции
class Timer
{
public:
    Timer()
    {
        start = std::chrono::system_clock::now();
    }
    ~Timer()
    {
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> duration = end - start;
        cout << " Work time: " << duration.count() << endl << endl;
    }
private:
    std::chrono::_V2::system_clock::time_point start, end;};

template <typename t>
class Matrix 
{
private:
    
    int str;
    int col;
    
    /*int Block(int st, int co) 
    {
        for (int i=100; i>1; --i) 
        {
            if (st % i == 0 && co % i == 0)
                return i;
        }
        return 1;
    }*/

public:
    t **matr;
    Matrix ()                           //пустая матрица
    {
        matr = 0;
        str = 0;
        col = 0;
    }   
    static Matrix NullMatrix(int N, int M) {    //нулевая
        t **matrix = new t *[N];
        for (int i = 0; i < N; ++ i)
        {
            matrix[i] = new t [M];
            for (int j = 0; j < M; ++ j)
                matrix[i][j] = 0;
        }
        return matrix;
    }
    static Matrix EdMatrix(int N) {         //единичная
        t **matrix = new t *[N];
        for (int i = 0; i < N; ++ i)
        {
            matrix[i] = new t[N];
            for (int j = 0; j < N; ++ j)
                matrix[i][j] = (i == j);
        }
        return matrix;
    }
    Matrix (int N, int M)           //матрица заданного размера
    {
        str = N;
        col = M;
        matr = new t *[str];
        for (int i = 0; i < str; ++ i)
        {
            matr[i] = new t[col];
        }
    }        
    Matrix(const string& name)     //матрица из файла
    {
        ifstream file(name);
        if (file.is_open())
        {
            file >> str >> col;
            matr = new t*[str];
            for (int i = 0; i < str; ++i)
            {
                matr[i] = new t [col];
                for (int j = 0; j < col; ++j)
                    file >> matr[i][j];
            }
            file.close();
        }
        else 
        {
            cerr << "File cannot be opened.";
        }   
    }

    Matrix(const Matrix &other)         //конструктор копирования
    {
        this->str = other.str;
        this->col = other.col;
        this->matr = new t *[other.str];
        for (int i = 0; i < other.str; ++ i) 
        {
            this->matr[i] = new t[other.col];
            for (int j = 0; j < other.col; ++j)
                this->matr[i][j] = other.matr[i][j];
        }
    }    
    ~Matrix()                           //деструkтор
    {
        for (int i = 0; i < str; ++i)
            delete[] matr[i];
        delete[] matr;
    }
    void inputMatrix()                  //ввод с косоли
    {
        cout << "Input matrix: " << str << " rows, " << col << " columns." << endl;
        for (int i = 0; i < str; ++i)
        {
            for (int j = 0; j < col; ++j)
                scanf ("%lf", & matr[i][j]);
        }
        cout << "Matrix created." << endl;
    }
    friend ostream& operator <<(ostream& os, Matrix& obj)
    {
        for (int i = 0; i < obj.str; ++i)
        {
            for (int j = 0; j < obj.col; ++j)
                os << obj.matr[i][j] << "  ";
            os << endl;
        }
        return os;
    }
    friend istream& operator >>(istream& in, Matrix& obj)
    {
        cout << "Enter nuber of rows and columns:  ";
        in >> obj.str >> obj.col;
        obj.matr = new t *[obj.str];
        for (int i = 0; i < obj.str; ++i)
        {
            obj.matr[i] = new t[obj.col];
            for (int j = 0; j < obj.col; ++j)
                in >> obj.matr[i][j];
        }
        return in;
    }
    friend ofstream& operator <<(ofstream& ofs, Matrix& obj)
    {
        for (int i = 0; i < obj.str; ++i)
        {
            for (int j = 0; j < obj.col; ++j)
                ofs << obj.matr[i][j] << "  ";
            ofs << endl;
        }
        return ofs;
    }
    friend ifstream& operator >>(ifstream& ifs, Matrix& obj)
    {
        ifs >> obj.str >> obj.col;
        obj.matr = new t[obj.str];
        for (int i = 0; i < obj.str; ++i)
        {
            obj.matr[i] = new t[obj.col];
            for (int j = 0; j < obj.col; ++j)
                ifs >> obj.matr[i][j];
        }
        return ifs;
    }

    const Matrix operator *(const Matrix &other)
    {
        Timer t1;
        if (this->col != other.str)
            throw "The number of columns of the first matrix is not equal to the number of rows of the second. The operation cannot be completed.";
        Matrix z(this->str,other.col);
        vector<thread> threads;
        for (int i = 0; i < this->str; ++i)
        {
            threads.push_back(thread([&, i](){
                for (int j = 0; j < other.col; ++j)
                {
                    int a = 0;
                    for (int n = 0; n < this->col; ++n)
                    {
                        a += this->matr[i][n]*other.matr[n][j];
                    }
                    z.matr[i][j] = a;
                }
            }));           
        }
        for (thread &potok : threads)
            potok.join();
        cout << "* with threads complete. ";
        return z;
    }
    const Matrix mult_async (const Matrix &other, int block)
    {
        Timer t2;
        if (this->col != other.str)
            throw "The number of columns of the first matrix is not equal to the number of rows of the second. The operation cannot be completed.";
        Matrix z(this->str,other.col);
        vector<future<void>> futures;
        for (int i = 0; i < this->str; i+=block)
        {    
            for (int j = 0; j < other.col; j+=block)
            {
                futures.push_back(async(launch::async, [&, i, j]()
                {
                    for (int x = i; x < min(i+block, this->str); ++x)
                    {
                        for (int y = j; y < min(j+block, this->col); ++y)
                        {
                            int a = 0;
                            for (int n = 0; n < this->col; ++n)
                            {
                                a += this->matr[x][n]*other.matr[n][y];
                            }
                            z.matr[x][y] = a;
                        }
                    }
                }));
            }             
        }
        for(auto &async_potok : futures)
            async_potok.get();
        return z;
    }
    const Matrix operator *(t skal)
    {
        Timer t3;
        vector<thread> threads;
        Matrix z(*this);
        for (int i = 0; i < this->str; ++i)
        {
            threads.push_back(thread([this,&z, skal, i](){
                for (int j = 0; j < this->col; ++j)
                {
                    z.matr[i][j] = this->matr[i][j] * skal;
                }
            }));
            
        }
        for (thread &potok : threads)
            potok.join();
        cout << "*skal with threads complete. ";
        return z;
    }
    const Matrix skalmult_async (t skal, int block) 
    {
        Timer t4;
        vector<future<void>> futures;
        Matrix z(*this);
        for (int i = 0; i < this->str; i+=block)
        {
            for (int j = 0; j < this->col; j+=block)
            {
                futures.push_back(async(launch::async, [&, i, j]()
                {
                    for (int x = i; x < min(i+block, this->str); ++x)
                    {
                        for (int y = j; y < min(j+block, this->col); ++y)
                            z.matr[x][y] = this->matr[x][y] * skal;
                    }
                }));
            }            
        }
        for(auto &async_potok : futures)
            async_potok.get();
        return z;
    }
    const Matrix operator +(const Matrix &other)
    {
        Timer t5;
        Matrix z(*this);
        vector<thread> threads;
        if (this->str != other.str || this->col != other.col)
            throw "Matrices have different dimensions!";
        for (int i = 0; i < this->str; ++i)
        {
            threads.push_back(thread([this, &other, &z, i]()
            {
                for (int j = 0; j < this->col; ++j)
                {
                    z.matr[i][j] = this->matr[i][j] + other.matr[i][j];
                }
            }));
        }
        for (thread& potok : threads)
            potok.join();
        cout << "+ with threads complete. ";
        return z;
    }
    const Matrix operator -(const Matrix &other)
    {
        Timer t6;
        Matrix z(*this);
        if (this->str != other.str || this->col != other.col)
            throw "Matrices have different dimensions!";
        vector<thread> threads;
        for (int i = 0; i < this->str; ++i)
        {
            threads.push_back(thread([this, &other, &z, i] () {
                for (int j = 0; j < this->col; ++j)
                {
                    z.matr[i][j] = this->matr[i][j] - other.matr[i][j];
                }
            }));    
        }
        for (thread& potok : threads)
            potok.join();
        cout << "- with threads complete. ";
        return z;
    }
    const Matrix add_async (const Matrix &other, int block)
    {
        Timer t7;
        if (this->str != other.str || this->col != other.col)
            throw "Matrices have different dimensions!";
            return Matrix();
        Matrix z(*this);
        vector<future<void>> futures;
        for (int i = 0; i < this->str; i += block)
        {
            for (int j = 0; j < this->col; j += block)
            {
                futures.push_back(async(launch::async, [this, &other, &z, block, i, j]()
                {
                    for (int x = i; x < min(i+block, this->str); ++x)
                    {
                        for (int y = j; y < min(j+block, this->col); ++y)
                            z.matr[x][y] = this->matr[x][y] + other.matr[x][y];
                    }
                }));
            }
        }
        for(auto &async_potok : futures)
            async_potok.get();
        return z;
    }   
    const Matrix razn_async (const Matrix &other, int block)
    {
        Timer t8;
        if (this->str != other.str || this->col != other.col)
            throw "Matrices have different dimensions!";
            return Matrix();
        Matrix z(*this);
        vector<future<void>> futures;
        for (int i = 0; i < this->str; i += block)
        {
            for (int j = 0; j < this->col; j += block)
            {
                futures.push_back(async(launch::async, [this, &other, &z, block, i, j]()
                {
                    for (int x = i; x < min(i+block, this->str); ++x)
                    {
                        for (int y = j; y < min(j+block, this->col); ++y)
                            z.matr[x][y] = this->matr[x][y] - other.matr[x][y];
                    }
                }));
            }
        }
        for(auto &async_potok : futures)
            async_potok.get();
        return z;
    }     
    Matrix operator !() {
        Matrix A(str, col);
        for (int i = 0; i < str; ++i) {
            for (int j = 0; j < col; ++j) {
                A.matr[i][j] = matr[i][j];
            }
        }
        if (str != col) {
            throw "The matrix is not square";
        }
        else {
            double determinant = A.determinant();
            if (determinant == 0)
                throw "The matrix is ​​degenerate";
            else {
                Matrix B = A.MatrixDopolnenii();
                B.transposition();
                Matrix C = B * (1/determinant);
                cout << "The inverse matrix was successfully calculated" << endl;
                return C;
            }
        }
    }
    bool operator ==(const Matrix &other)
    {
        if (this->col != other.col || this->str != other.str)
            return false;
        for (int i = 0; i < this->str; ++i)
        {
            for (int j = 0; j < this->col; ++j)
            {
                if (this->matr[i][j] != other.matr[i][j])
                {
                    return false;
                }
            }
        }
        return true;
    }
    bool operator ==(t skal)
    {
        Matrix a(this->str,this->col,skal);

        for (int i = 0; i < this->str; ++i)
        {
            for (int j = 0; j < this->col; ++j)
            {
                if (this->matr[i][j] != a[i][j])
                    return false;  
            }
        }
        return true;
    }
    bool operator !=(const Matrix &other)
    {
        if (this->str != other.str || this->col != other.col)
            return true;
        for (int i = 0; i < this->str; ++i)
        {
            for (int j = 0; j < this->col; ++j)
            {
                if (this->matr[i][j] != other.matr[i][j])
                    return true;
            }
        }
        return false;
    }
    bool operator !=(t skal)
    {
        Matrix a(this->str,this->col,skal);
        for (int i = 0; i < this->str; ++i)
        {
            for (int j = 0; j < this->col; ++j)
            {
                if (this->matr[i][j] != a[i][j])
                    return true;  
            }
        }
        return false;
    }
    Matrix& operator=(const Matrix& matrix) { //оператор присвоения
		if (matr != nullptr) {
			for (int i = 0; i < str; ++i) {
				delete[] matr[i];
			}
			delete[] matr;
		}
		str = matrix.str;
		col = matrix.col;
		matr = new t*[str];
		for (int i = 0; i < str; ++i) {
			matr[i] = new t[col];
			for (int j = 0; j < col; ++j) {
				matr[i][j] = matrix.matr[i][j];
			}
		}
		return *this;
    }
    
    
    int GetCol () {                     //метод возвращающий количество колонок
        return col;
    }    
    int GetStr () {                     //метод возвращающий количество строк
        return str;
    }        
    t* operator [] (int index)     //перегрузка оператора [] для возвращения строки
    {
        return getRow (index);
    }
    t* getRow (int index)
    {
        if (index >= 0 && index < str){
            return matr[index];
        }
        return 0;
    }
    t* getColumn (int index)
    {
        if (index < 0 || index >= col)
            return 0;
        double * c = new double [str];
        for (int i = 0; i < str; ++ i)
            c[i] = matr[i][index];
        return c;
    }  

    void swapRows (int index1, int index2)                     //First  elemental
    {
        if (index1-1 < 0 || index2-1 < 0 || index1 > str || index2 > str)
            throw "Invalid row number.";
        for (int i = 0; i < col; ++ i)
            swap (matr[index1-1][i], matr[index2-1][i]);
    }
    void skalRow (int index, double skal)                      //Second elemental
    {
        if (index <= 0 || index > str)
            throw "Invalid row number.";
        for (int i = 0; i < col; ++i)
        {
            matr[index-1][i] = matr[index-1][i]*skal;
        }
    }
    void skaladdRows (int index1, int index2, double skal = 1) //Third  elemental
    {
        if (index1 <= 0 || index2 <= 0 || index1 > str || index2 > str)
            throw "Invalid row number.";
        for (int i = 0; i < col; ++i)
        {
            matr[index2-1][i] = matr[index2-1][i] + (matr[index1-1][i]*skal);
        }    
    }
    
    Matrix transposition() 
    {
        Timer t9;
        Matrix new_matrix(col, str);

		vector<thread> threads;
		for (int i = 0; i < col; ++i) 
		{
			threads.push_back(thread([&,i]() 
			{
				for (int j = 0; j < str; ++j) 
				{
					new_matrix.matr[i][j] = matr[j][i];
				}
			}));
		}
		for (thread& thread : threads) 
			thread.join();
        cout << "transposition() with threads complete. ";
		return new_matrix;
    }
    Matrix transposition_async(int block) 
    {
        Timer t10;
        Matrix new_matrix(col, str);

		vector<future<void>> futures;
		for (int i = 0; i < str; i+=block) 
		{
			for (int j = 0; j < col; j+=block) 
			{
				futures.push_back(async(launch::async, [&, i, j]()
                {
                    for (int x = i; x < min(i+block, this->str); ++x)
                    {
                        for (int y = j; y < min(j+block, this->col); ++y)
                            new_matrix.matr[y][x] = matr[x][y];
                    }
                }));
        
			}	
		}
		for(auto &async_potok : futures)
            async_potok.get();
		return new_matrix;
    }
    
    double determinant() 
    {
        Timer t11;
        if (str != col) {
            throw "Not possible to calculate the determinant";
        }
        else if (str == 1) {
            return matr[0][0];
        }
        else if (str == 2) {
            return matr[0][0] * matr[1][1] - matr[1][0] * matr[0][1];
        }
        else {
            double determinant = 0;
            vector<thread> threads;
            
            for (int i = 0; i < col; ++i) 
            {
                threads.push_back(thread([&,i]()
                {
                    Matrix minor(str - 1, col - 1);
                    for (int j = 1; j < str; ++j) 
                    {
                        int k = 0;
                        for (int l = 0; l < col; ++l) {
                            if (l != i) {
                                minor.matr[j - 1][k] = matr[j][l];
                                k++;
                            }
                        }
                    }
                    if (i%2 == 0)
                        determinant += matr[0][i] * minor.determinant();
                    else
                        determinant += -1 * matr[0][i] * minor.determinant();
                }));
                
                
            }
            for (thread& thread : threads) 
			    thread.join();
            cout << "determinant() with threads complete. ";
            return determinant;
        }
    }
    double determinant_async(int block) 
    {
        Timer t12;
        if (str != col) {
            throw "Not possible to calculate the determinant";
        }
        else if (str == 1) {
            return matr[0][0];
        }
        else if (str == 2) {
            return matr[0][0] * matr[1][1] - matr[1][0] * matr[0][1];
        }
        else {
            double determinant = 0;
            vector<future<double>> futures;
            
            for (int i = 0; i < col; ++i) 
            {
                futures.push_back(async(launch::async,[&,i]()
                {
                    double async_det = 0;
                    for (int x = i; x < min(i + block, col); ++x)
                    {
                        Matrix minor(str - 1, col - 1);
                        for (int j = 1; j < str; ++j) 
                        {
                            int k = 0;
                            for (int l = 0; l < col; ++l) 
                            {
                                if (l != i) 
                                {
                                    minor.matr[j - 1][k] = matr[j][l];
                                    k++;
                                }
                            }
                        }
                        if (i%2 == 0)
                            async_det += matr[0][i] * minor.determinant();
                        else
                            async_det += -1 * matr[0][i] * minor.determinant();
                
                    }
                    return async_det; 
                }));
            }
            for(auto &async_potok : futures)
                determinant += async_potok.get();
            return determinant;
        }
    }
    
    double dopolnenie(int n, int m) 
    {
        Timer t13;
        Matrix A(str - 1, col - 1);
        int x, y = 0;

        vector<thread> threads;

        for (int i = 0; i < (str - 1); ++i) 
        {
            threads.emplace_back([&, i] 
            {
                if (i == n - 1)
                    x = 1;
                for (int j = 0; j < (str - 1); ++j) {
                    if (j == m - 1) 
                        y = 1;
                
                    A.matr[i][j] = matr[i + x][j + y];
                }
                y = 0;
            });            
        }
        int sign;
        if ((n + m) % 2 == 0) 
            sign = 1;
        else 
            sign = -1;
        for (thread& thread : threads) 
			thread.join();
        cout << "dopolnenie() with threads complete. ";
        return sign * A.determinant();
    }
    double dopolnenie_async(int n, int m, int block) 
    {
        Timer t14;
        Matrix A(str - 1, col - 1);
        int x, y = 0;

        vector<future<void>> futures;

        for (int i = 0; i < (str - 1); i+=block) 
        {
            futures.push_back(async(launch::async, [&, i]() 
            {
                for (int k = i; k < min(i+block, (str-1)); ++k)
                {
                    if (i == n - 1)
                        x = 1;
                    for (int j = 0; j < (col - 1); ++j) {
                        if (j == m - 1) 
                            y = 1;
                
                        A.matr[k-i][j] = matr[k + x][j + y];
                    }
                    y = 0;
                }                
            }));            
        }
        int sign;
        if ((n + m) % 2 == 0) 
            sign = 1;
        else 
            sign = -1;
        for(auto &async_potok : futures)
            async_potok.get();

        return sign * A.determinant_async(block);
    }
    
    Matrix MatrixDopolnenii() 
    {
        Timer t15;
        Matrix A(str, col);
        for (int i = 0; i < str; ++i) 
        {
            for (int j = 0; j < col; ++j) 
                A.matr[i][j] = matr[i][j];
        }
        Matrix result(str, col);
        
        vector<thread> threads;
        
        if (str != col) 
            throw "The matrix is not square";
        else 
        {
            for (int i = 0; i < str; ++i) 
            {
                for (int j = 0; j < col; ++j)
                {
                    threads.emplace_back([&,i,j]{
                        result.matr[i][j] = A.dopolnenie(i + 1, j + 1);
                    });    
                }
            }
        }
        for (thread& thread : threads) 
			thread.join();
        cout << "MatrixDopolnenii() with threads complete. ";
        return result;
    }
    Matrix MatrixDopolnenii_async(int block) 
    {
        Timer t16;
        Matrix A(str, col);
        for (int i = 0; i < str; ++i) 
        {
            for (int j = 0; j < col; ++j) 
                A.matr[i][j] = matr[i][j];
        }
        Matrix result(str, col);
        
        vector<future<void>> futures;
        
        if (str != col) 
            throw "The matrix is not square";
        else 
        {
            for (int i = 0; i < str; i+=block) 
            {
                for (int j = 0; j < col; j+=block)
                {
                    futures.push_back(async(launch::async, [&,i,j](){
                        for (int x = i; x < min(i+block, str); ++x)
                        {
                            for (int y = j; y < min(j+block, col); ++y)
                                result.matr[x][y] = A.dopolnenie_async(x + 1, y + 1, block);
                        }                        
                    }));    
                }
            }
        }
        for(auto &async_potok : futures)
            async_potok.get();
        return result;
    }

};

int main() {
    random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> dist(0, 10);
    //cout << thread::hardware_concurrency();

    //16.1

    /*int block = 0;
    for (int x = 0; x <= 2000; x += 200)
    {
        Matrix<double> A(x, x);
        for (int i = 0; i < x; ++i)
        {
            for (int j = 0; j < x; ++j)
                A.matr[i][j] = dist(gen);
        }
        Matrix<double> B(A);
        cout << "Matrix size: " << x << "×" << x;
        Matrix<double> Result = A.add_async(B, block);
        block += 50;
    }
    block = 0;
    for (int x = 0; x <= 2000; x += 200)
    {
        Matrix<double> A(x, x);
        for (int i = 0; i < x; ++i)
        {
            for (int j = 0; j < x; ++j)
                A.matr[i][j] = dist(gen);
        }
        Matrix<double> B(A);
        cout << "Matrix size: " << x << "×" << x;
        Matrix<double> Result = A.mult_async(B, block);
        block += 50;
    }*/


   //16.2

   int block = 0;
   for (int y = 0; y <= 1080; y += 120)
   {
        Matrix<double> A(y, y);
        for (int i = 0; i < y; ++i)
        {
            for (int j = 0; j < y; ++j)
                A.matr[i][j] = dist(gen);
        }
        Matrix<double> B(A);
        for (int z = 1; z <= 4; ++z)
        {
            block = y/z;
            cout << "Matrix size: " << y << "×" << y << "   Number of treads: " << z;
            Matrix<double> Result = A.add_async(B, block);
        }
   }
   for (int y = 0; y <= 1080; y += 120)
   {
        Matrix<double> A(y, y);
        for (int i = 0; i < y; ++i)
        {
            for (int j = 0; j < y; ++j)
                A.matr[i][j] = dist(gen);
        }
        Matrix<double> B(A);
        for (int z = 1; z <= 4; ++z)
        {
            block = y/z;
            cout << "Matrix size: " << y << "×" << y << "   Number of treads: " << z;
            Matrix<double> Result = A.mult_async(B, block);
        }
   }
}




    
    

