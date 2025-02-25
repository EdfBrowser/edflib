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

    [DllImport("../edflib", EntryPoint = "edf_read_signal_data", CallingConvention = CallingConvention.Cdecl)]
    private static extern int EdfReadSignalData(
                                                IntPtr hadnle,
                                                IntPtr ptr,
                                                uint signalIndex,
                                                uint startRecord = 0,
                                                uint recordCount = 0);

    private IntPtr _handle;
    private bool _disposed = false;


    public readonly struct SignalDataInfo
    {
        private readonly double _pMin;
        private readonly double _pMax;
        private readonly double _dMin;
        private readonly double _dMax;

        private readonly uint _index;
        private readonly uint _startRecord;
        private readonly uint _readCount;

        private readonly double _unit;
        private readonly double _offset;

        public SignalDataInfo(double pMin, double pMax, double dMin, double dMax,
            uint index = 0, uint startRecord = 0, uint readCount = 1)
        {
            _pMin = pMin;
            _pMax = pMax;
            _dMin = dMin;
            _dMax = dMax;
            _index = index;
            _startRecord = startRecord;
            _readCount = readCount;

            _unit = (_pMax - _pMin) / (_dMax - _dMin);
            _offset = (_pMax / _unit - _dMax);
        }

        public SignalDataInfo(SignalInfo info, uint index = 0, uint startRecord = 0, uint readCount = 1)
        : this(info.PhysicalMin, info.PhysicalMax, info.DigitalMin, info.DigitalMax,
                index, startRecord, readCount)
        { }

        public double PMin => _pMin;
        public double PMax => _pMax;
        public double DMin => _dMin;
        public double DMax => _dMax;

        public uint Index => _index;
        public uint StartRecord => _startRecord;
        public uint RecordCount => _readCount;

        public double Unit => _unit;
        public double Offset => _offset;
    }

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

    public void ReadPhysicalData(double[] buf, SignalDataInfo dataInfo)
    {
        ReadDigitalData(buf, dataInfo);

        for (int i = 0; i < buf.Length; i++)
        {
            buf[i] = dataInfo.Unit * (buf[i] + dataInfo.Offset);
            // buf[i] = (raw - dataInfo.DMin) * dataInfo.Unit + dataInfo.PMin;
        }
    }


    public void ReadDigitalData(double[] buf, SignalDataInfo dataInfo)
    {
        if (buf == null) throw new ArgumentNullException(nameof(buf));

        // sizeof(short) == 2;
        IntPtr ptr = Marshal.AllocHGlobal(buf.Length * 2);

        try
        {
            int result = EdfReadSignalData(_handle, ptr, dataInfo.Index, dataInfo.StartRecord, dataInfo.RecordCount);
            if (result != 0)
                throw new EDFException($"Read signal data error: {result}");

            // 补码
            // foreach (byte b in buf)
            // {
            //     System.Console.WriteLine(Convert.ToString(b, 2));
            // }

            for (int i = 0; i < buf.Length; i++)
            {
                byte one = Marshal.ReadByte(ptr, 2 * i);
                byte two = Marshal.ReadByte(ptr, 2 * i + 1);

                // 小端
                short raw = (short)((one) | (two << 8));
                // raw = Marshal.ReadInt16(ptr, 2 * i);

                buf[i] = raw;
                //buf[i] = dataInfo.Unit * (raw + dataInfo.Offset);
                // buf[i] = (raw - dataInfo.DMin) * dataInfo.Unit + dataInfo.PMin;
            }
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


