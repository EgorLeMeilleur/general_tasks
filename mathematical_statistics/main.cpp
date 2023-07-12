#include <iostream>
#include <random>
#include <chrono>
#include <fstream>

double function(double x)
{
	return std::pow((3 * x - 1), 2);
}

int main()
{
	int a = 1;
	int b = 3;
	std::uniform_real_distribution<double> random(0, 1);
	std::default_random_engine generator(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
	unsigned int n = 100;
	unsigned int m = 200;
	double general_sum = 0.0;
	double general_error = 0.0;
	std::vector<double> error_vector;
	std::vector<double> value_vector;
	for (unsigned int i = 0; i < m; ++i)
	{
		double sum = 0.0;
		double sum_sq = 0.0;
		double error = 0.0;
		for (unsigned int j = 0; j < n; ++j)
		{
			double x = (b - a) * random(generator) + a;
			double y = function(x);
			sum += y;
		}
		sum = sum * (b - a) / n;
		error = 56 - sum;
		general_sum += sum;
		general_error += error;
		error_vector.push_back(error);
		value_vector.push_back(sum);
		std::cout << i + 1 << " imitation: " << sum << '\n';
		std::cout << i + 1 << " error: " << error << '\n';
	}
	std::cout << "Average value for " << m << " imitations: " << general_sum / m << std::endl;
	std::cout << "Average error for " << m << " imitations: " << general_error / m << std::endl;
	std::ofstream output("data.txt");
	for (size_t i = 0; i < error_vector.size(); ++i)
	{
		if (i != error_vector.size() - 1)
		{
			output << error_vector[i] << '\n';
		}
		else 
		{
			output << error_vector[i];
		}
	}
	return 0;
}