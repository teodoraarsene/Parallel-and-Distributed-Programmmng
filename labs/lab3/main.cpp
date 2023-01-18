#include <iostream>
#include <thread>
#include "Matrix.hpp"
#include "MatrixMultiplication.hpp"
#include "ThreadPool.hpp"

void multiplyMatrices(int startIdx, int stopIdx, MatrixMultiplication multiplication)
{
	multiplication.multiplySection(startIdx, stopIdx);
}
int main()
{
	const int ROWS_1 = 1000;
	const int COLS_1 = 1000;

	const int ROWS_2 = COLS_1;
	const int COLS_2 = 1000;

	const int ROWS_RESULT = ROWS_1;
	const int COLS_RESULT = COLS_2;

	const int NO_TASKS = 50;
	const int NO_ELEMS_PER_TASK = (ROWS_RESULT * COLS_RESULT) / NO_TASKS;
	std::string METHOD = "threads";

	Matrix a = Matrix(ROWS_1, COLS_1);
	a.initializeWithValues();
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	Matrix b = Matrix(ROWS_2, COLS_2);
	b.initializeWithValues();

	//a.print();
	std::cout << std::endl;
	//b.print();
	std::cout << std::endl;

	Matrix result = Matrix(ROWS_RESULT, COLS_RESULT);
	MatrixMultiplication multiplication = MatrixMultiplication(a, b, result);

	std::chrono::milliseconds start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	if (METHOD == "threads")
	{
		std::vector<std::thread> threads;
		for (int i = 0; i < NO_TASKS; ++i)
		{
			if (i == NO_TASKS - 1)
			{
				threads.emplace_back(multiplyMatrices, i * NO_ELEMS_PER_TASK, ROWS_RESULT * COLS_RESULT - 1, multiplication);
			}
			else {
				threads.emplace_back(multiplyMatrices, i * NO_ELEMS_PER_TASK, (i + 1) * NO_ELEMS_PER_TASK - 1, multiplication);
			}
		}
		for (int i = 0; i < NO_TASKS; ++i)
		{
			threads[i].join();
		}
	}
	else if(METHOD == "thread pool")
	{
		ThreadPool pool(NO_TASKS);
		for (int i = 0; i < NO_TASKS; ++i)
		{
			if (i == NO_TASKS - 1)
			{
				pool.enqueue([i, NO_ELEMS_PER_TASK, ROWS_RESULT, COLS_RESULT, multiplication]() { multiplyMatrices(i * NO_ELEMS_PER_TASK, ROWS_RESULT * COLS_RESULT - 1, multiplication);  });
			}
			else {
				pool.enqueue([i, NO_ELEMS_PER_TASK, ROWS_RESULT, COLS_RESULT, multiplication]() { multiplyMatrices(i * NO_ELEMS_PER_TASK, (i + 1) * NO_ELEMS_PER_TASK - 1, multiplication);  });
			}
		}
		pool.close();
	}
	std::chrono::milliseconds end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	//result.print();
	std::cout << "Time: " << (end - start).count() << " ms\n";

	return 0;
}