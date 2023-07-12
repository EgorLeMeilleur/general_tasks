internal class Program
{
    public static double task2()
    {
        Random random = new Random();
        List<Tuple<double, double>> random_numbers = new List<Tuple<double, double>>();
        List<Tuple<double, double>> random_values = new List<Tuple<double, double>>();
        List<Tuple<double, double>> discret = new List<Tuple<double, double>> { new Tuple<double, double>(0.1, 13.0),
            new Tuple<double, double>(0.5, 14.0), new Tuple<double, double>(0.4, 16.0) };
        //double n = double.Parse(Console.ReadLine());
        double n = 10000.0;
        for (int i = 0; i < n; i++)
        {
            random_numbers.Add(new Tuple<double, double>(random.NextDouble(), random.NextDouble()));
        }
        foreach (var value in random_numbers)
        {
            double random_discret_value = 0.0;
            for (int i = 0; i < discret.Count; i++)
            {
                double sum1 = 0.0;
                double sum2 = 0.0;
                for (int j = 0; j < i; ++j)
                {
                    sum1 += discret[j].Item1;
                }
                for (int j = 0; j < i + 1; ++j)
                {
                    sum2 += discret[j].Item1;
                }
                if (value.Item2 <= sum2 && value.Item2 > sum1)
                {
                    random_discret_value = discret[i].Item2;
                    break;
                }
            }
            random_values.Add(new Tuple<double, double>(20 * Math.Pow(value.Item1, 30.0 / 43.0), random_discret_value));
        }
        double count = 0;
        foreach (var value in random_values)
        {
            if (17.0 * value.Item2 / 30.0 + 13.0 * value.Item1 / 30.0 > 17.0 * 16.0 / 30.0)
            {
                count++;
            }
        }
        return count / n;
    }

    private static void Main(string[] args)
    {
        //Console.WriteLine(task2());
        double n = double.Parse(Console.ReadLine());
        double count = 0.0;;
        for (int i = 0; i < n; ++i)
        {
            double j = task2();
            if (Math.Abs(j - 0.963237) <= 0.05)
            {
                count++;
            }
        }
        Console.WriteLine(count / n);
    }
}


        

