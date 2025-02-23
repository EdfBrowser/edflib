using System;
using System.Runtime.InteropServices;

public sealed class EDFReader : IDisposable
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct EDFHeader
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 80)]
        public string PatientID;
        
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 80)]
        public string RecordingID;
    }

    [DllImport("../edflib", CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr edf_open(string filepath);

    [DllImport("../edflib", CallingConvention = CallingConvention.Cdecl)]
    private static extern int edf_close(IntPtr handle);

    [DllImport("../edflib", CallingConvention = CallingConvention.Cdecl)]
    private static extern int edf_read_header(IntPtr handle, out EDFHeader header);

    private IntPtr _handle;

    public EDFReader(string filepath) {
        _handle = edf_open(filepath);
        if (_handle == IntPtr.Zero)
            throw new EDFException("Failed to open EDF file");
    }

    public EDFHeader ReadHeader() {
        EDFHeader header;
        int result = edf_read_header(_handle, out header);
        if (result != 0) 
            throw new EDFException($"Header read error: {result}");
        return header;
    }

    public void Dispose() {
        if (_handle != IntPtr.Zero) {
            edf_close(_handle);
            _handle = IntPtr.Zero;
        }
        GC.SuppressFinalize(this);
    }

    ~EDFReader() => Dispose();
}

public class EDFException : Exception {
    public EDFException(string message) : base(message) {}
}


