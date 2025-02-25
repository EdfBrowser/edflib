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

            var sig = header.Signals[0];
            double[] buf = new double[sig.Samples];
            var signalDataInfo = new EDFReader.SignalDataInfo(sig, 0, 0, 1);
            edf.ReadPhysicalData(buf, signalDataInfo);

            foreach (double val in buf)
            {
                System.Console.WriteLine(val);
            }
        }

        // System.Console.ReadKey();
    }
}
