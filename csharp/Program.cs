public class Program {
 public static void Main(string[] args) {
    using(EDFReader edf = new EDFReader(@"D:\code\X.edf")) {
        var header = edf.ReadHeader();
        System.Console.WriteLine(header.PatientID);
        System.Console.WriteLine(header.RecordingID);
    }
 }
}
