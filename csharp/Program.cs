public class Program
{
    public static void Main(string[] args)
    {
        using (EDFReader edf = new EDFReader(@"D:\code\X.edf"))
        {
            var header = edf.ReadHeader();
            for (int i = 0; i < header.SignalCount; i++)
            {
                System.Console.WriteLine(header.Signals[i].Samples);
            }
        }

        // System.Console.ReadKey();
    }
}
