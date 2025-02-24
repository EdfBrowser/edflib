using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.Remoting.Messaging;

public sealed class EDFReader : IDisposable
{

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct SignalInfo
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 17)]
        public char[] Label;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 9)]
        public char[] PhysicalDim;
        public double PhysicalMin;
        public double PhysicalMax;
        public int DigitalMin;
        public int DigitalMax;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 81)]
        public char[] Prefiltering;
        public uint Samples;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct EDFHeader
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 81)]
        public char[] PatientID;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 81)]
        public char[] RecordingID;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 9)]
        public char[] StartDate;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 9)]
        public char[] StartTime;
        [MarshalAs(UnmanagedType.U4)]
        public uint RecordCount;
        [MarshalAs(UnmanagedType.R8)]
        public double RecordDuration;
        [MarshalAs(UnmanagedType.U4)]
        public uint SignalCount;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4096)]
        public SignalInfo[] Signals;
    }

    // TODO: T4模板替换
    [DllImport("../edflib", EntryPoint = "edf_open", CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr EdfOpen(string filepath);

    [DllImport("../edflib", EntryPoint = "edf_close", CallingConvention = CallingConvention.Cdecl)]
    private static extern int EdfClose(IntPtr handle);

    [DllImport("../edflib", EntryPoint = "edf_read_header", CallingConvention = CallingConvention.Cdecl)]
    private static extern int EdfReadHeader(IntPtr handle, IntPtr ptr);

    private IntPtr _handle;
    private bool _disposed = false;


    public EDFReader(string filepath)
    {
        _handle = EdfOpen(filepath);
        if (_handle == IntPtr.Zero)
            throw new EDFException("Failed to open EDF file");
    }

    public EDFHeader ReadHeader()
    {
        IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf<EDFHeader>());

        try
        {
            int result = EdfReadHeader(_handle, ptr);
            if (result != 0)
                throw new EDFException($"Header read error: {result}");

            return Marshal.PtrToStructure<EDFHeader>(ptr);
        }
        finally
        {
            if (ptr != IntPtr.Zero)
                Marshal.FreeHGlobal(ptr);
        }
    }

    public void Dispose(bool disposing)
    {
        if (_disposed)
            return;

        if (disposing)
        {

        }

        if (_handle != IntPtr.Zero)
        {
            EdfClose(_handle);
            _handle = IntPtr.Zero;
        }

        _disposed = true;
    }

    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    ~EDFReader() => Dispose(false);
}

public class EDFException : Exception
{
    public EDFException(string message) : base(message) { }
}


