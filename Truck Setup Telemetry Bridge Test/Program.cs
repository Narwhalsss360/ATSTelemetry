using System;
using System.IO.MemoryMappedFiles;
using System.IO;
using System.IO.Ports;
using NStreamCom;
using static System.Environment;
using System.Runtime.InteropServices;
using System.Threading;

namespace Truck_Setup_Telemetry_Bridge_Test
{
    static class GameDataContainer
    {
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct TruckDashboardData
        {
            public float Speed;
            public float Rpm;
            public float Fuel;
            public float FuelRange;
            public float Odometer;
            public float CruiseControl;
        } //24

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct TruckEngineData
        {
            public float Retarder;
            public float OilPressure;
            public float OilTemperature;

            public int Gear;

            public byte Enabled;
            public byte Brake;
            public byte DifferentialLock;
        } //19 - 20. Mismatch, C++: 20, C#:19

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct TruckLightData
        {
            public byte Hazard;
            public byte LowBeam;
            public byte HighBeam;
            public byte LeftBlinker;
            public byte RightBlinker;
        } //5

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct TruckGeneralData
        {
            public float BatteryVoltage;
            public float BrakeAirPressure;
            public float BrakeTemperature;

            public byte Wipers;
            public byte ParkingBrake;
        } //14, Mismatch, C++:16, C#:14

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct TruckWearData
        {
            public float Engine;
            public float Transmission;
            public float Cabin;
            public float Chassis;
            public float Wheel;
        } //20

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct TruckData
        {
            [MarshalAs(UnmanagedType.Struct)]
            public TruckDashboardData Dashboard;
            [MarshalAs(UnmanagedType.Struct)]
            public TruckEngineData Engine;
            [MarshalAs(UnmanagedType.Struct)]
            public TruckLightData Lights;
            [MarshalAs(UnmanagedType.Struct)]
            public TruckGeneralData General;
            [MarshalAs(UnmanagedType.Struct)]
            public TruckWearData Wear;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct TrailerGeneralData
        {
            public byte Connected;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct TrailerWearData
        {
            public float Body;
            public float Chassis;
            public float Wheels;
            public float Cargo;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct TrailerData
        {
            [MarshalAs(UnmanagedType.Struct)]
            public TrailerGeneralData General;
            [MarshalAs(UnmanagedType.Struct)]
            public TrailerWearData Wear;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct CommonData
        {
            public byte Paused;
            public uint GameTime;
            public uint ShifterType;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct GameplayEventFlags
        {
            public byte Fined;
            public byte TollPayed;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct GameData
        {
            [MarshalAs(UnmanagedType.Struct)]
            public TruckData Truck;
            [MarshalAs(UnmanagedType.Struct)]
            public TrailerData Trailer;
            [MarshalAs(UnmanagedType.Struct)]
            public CommonData Common;
            [MarshalAs(UnmanagedType.Struct)]
            public GameplayEventFlags Events;
        }
    }

    internal class Program
    {
        const string FileName = "Truck Setup Telemetry.dat";
        const string ObjectName = "TruckSetupTelemetryExtenstion";
        static string FilePath = $"{ GetFolderPath(SpecialFolder.LocalApplicationData) }\\Temp\\{ FileName }";
        static MemoryMappedFile Map;
        static MemoryMappedViewAccessor Accessor;
        static GameDataContainer.GameData Game = new GameDataContainer.GameData();
        static SerialPort Port;

        static T BytesToStructure<T>(byte[] Buffer)
        {
            IntPtr Pointer = Marshal.AllocHGlobal(Buffer.Length);
            Marshal.Copy(Buffer, 0, Pointer, Buffer.Length);
            T Structure = (T)Marshal.PtrToStructure(Pointer, typeof(T));
            Marshal.FreeHGlobal(Pointer);
            return Structure;
        }

        static void Initilize()
        {
            foreach (string PortName in SerialPort.GetPortNames()) Console.Write($"| { PortName } |");
            Console.WriteLine(" Select Serial Port.");

            Port = new SerialPort(Console.ReadLine(), 1000000);
            Port.StopBits = StopBits.One;
            Port.Parity = Parity.None;
            Port.Open();
            Console.WriteLine("Port opened");

            Map = MemoryMappedFile.OpenExisting(ObjectName);
            Accessor = Map.CreateViewAccessor();
            Console.WriteLine("Game data Mapped");
        }

        static void Run()
        {
            while(File.Exists(FilePath))
            {
                Thread.Sleep(88);
                byte[] GameDataBuffer = new byte[110];
                Accessor.ReadArray(0, GameDataBuffer, 0, 110);
                Game = BytesToStructure<GameDataContainer.GameData>(GameDataBuffer); //Broken, maybe-

                byte[] OutBytes = NStreamComParser.Parse(0, GameDataBuffer);
                Port.Write(OutBytes, 0, OutBytes.Length);
            }
        }

        static void Clean()
        {
            Port.Close();
            Accessor.Dispose();
            Map.Dispose();
        }

        static void Main(string[] args)
        {
        RetryFindingFile:
            if (!File.Exists(FilePath))
            {
                Console.WriteLine("Game not detected! Press enter to re-check.");
                Console.ReadLine();
                goto RetryFindingFile;
            }

            Initilize();
            Run();
            Clean();
            Console.WriteLine("Game exited.");
        }
    }
}