public class Program
{
    public static void Main(string[] args)
    {
        using (EDFReader edf = new EDFReader(@"D:\code\X.edf"))
        {
            var header = edf.ReadHeader();
            // for (int i = 0; i < header.SignalCount; i++)
            // {
            //     System.Console.WriteLine(header.Signals[i].Samples);
            // }

            edf.ReadSignalData(0, 0, 1);
        }

        // System.Console.ReadKey();
    }
}
