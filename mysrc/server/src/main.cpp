#include <iostream>
#include "MyWorkers.h"

int main(int argc, char * argv)
{
	MyWorker worker;
	worker.start();
	clock_t beg = clock();
	for (int i = 0; i < 100000000; ++i)
	{
		worker.push_back([i]() {
			int m = i * i * i;
			m = m / 1234;
		});
	}
	clock_t end = clock();
	worker.stop();
	clock_t totalEnd = clock();
	std::cout << beg << std::endl;
	std::cout << end << std::endl;
	std::cout << totalEnd << std::endl;
	char c = getchar();
	return 0;
}