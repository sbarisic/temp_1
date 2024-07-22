using System;
using System.Diagnostics;
using System.Threading;
using nanoFramework.Hardware.Esp32;
using System.Device.Gpio;
using Iot.Device.Ws28xx;
using Iot.Device.Ws28xx.Esp32;
using System.Device.Wifi;
using System.Drawing;
using System.IO;
using System.Net.NetworkInformation;
using System.Net;
using Iot.Device.DhcpServer;
using nanoFramework.Networking;
using nanoFramework.Runtime.Native;
using Windows.Storage.Devices;
using System.Device.Spi;
using Windows.Storage;
using NF.AwesomeLib;

namespace NanoCore {
	public class Program {
		static GpioController GPIO;

		static void SetupPins_LILYGO_CAN() {
			GPIO = new GpioController();

			GpioPin Pin5VEN = GPIO.OpenPin(16, PinMode.Output);
			Pin5VEN.Write(PinValue.High);

			GpioPin PinSDCS = GPIO.OpenPin(13, PinMode.Output);
			PinSDCS.Write(PinValue.Low);

			Configuration.SetPinFunction(14, DeviceFunction.SPI1_CLOCK);
			Configuration.SetPinFunction(15, DeviceFunction.SPI1_MOSI);
			Configuration.SetPinFunction(02, DeviceFunction.SPI1_MISO);

			//SDCard.MountSpi("SPI1", 13);
		}

		public static void Main() {
			SetupPins_LILYGO_CAN();

			if (Can.Init(26, 27, CanMode.Normal, CanTiming.CAN500KBIT))
				Debug.WriteLine("Can.Init - OK");
			else
				Debug.WriteLine("Can.Init - FAIL");

			//CanSend();
			CanListen();
		}

		static void CanSend() {
			Ws28xx Neo = new Ws2812c(4, 1);
			Neo.Image.SetPixel(0, 0, Color.FromArgb(50, 0, 0));
			Neo.Update();

			byte[] Dat = new byte[8];
			Random Rnd = new Random();

			while (true) {
				Thread.Sleep(500);

				uint ID = (uint)(Rnd.Next(100) + 100);
				byte Len = (byte)(Rnd.Next(7) + 1);
				Rnd.NextBytes(Dat);

				if (!Can.Send(ID, 1, Len, Dat))
					Debug.WriteLine("Can.Send - FAIL");
			}
		}

		static void CanListen() {
			Ws28xx Neo = new Ws2812c(4, 1);
			Neo.Image.SetPixel(0, 0, Color.FromArgb(0, 0, 50));
			Neo.Update();

			while (true) {
				Debug.WriteLine("Listening for CAN traffic");

				uint ID = 0;
				uint Ext = 0;
				uint RTR = 0;
				byte DataLen = 0;
				byte[] Data = null;

				if (Can.Receive(1000, ref ID, ref Ext, ref RTR, ref DataLen, Data)) {
					Debug.WriteLine("Can frame received!");
				}

				Thread.Sleep(0);
			}
		}

		static void PrintFolder(StorageFolder F1, int Level) {
			string Prefix = new string('-', Level * 2);
			Debug.WriteLine(Prefix + F1.Name);

			StorageFolder[] SDFolders = F1.GetFolders();
			StorageFile[] SDFiles = F1.GetFiles();

			foreach (StorageFolder SDFolder in SDFolders) {
				PrintFolder(SDFolder, Level + 1);
			}

			foreach (StorageFile SDFile in SDFiles) {
				Debug.WriteLine(Prefix + SDFile.Path);
			}
		}


		static void BlinkLEDs() {
			Debug.WriteLine("Hello from nanoFramework! Testing LED!");

			Ws28xx Neo = new Ws2812c(4, 1);
			while (true) {
				Neo.Image.SetPixel(0, 0, Color.FromArgb(50, 0, 0));
				Neo.Update();

				Thread.Sleep(500);

				Neo.Image.SetPixel(0, 0, Color.FromArgb(0, 50, 0));
				Neo.Update();

				Thread.Sleep(500);

				Neo.Image.SetPixel(0, 0, Color.FromArgb(0, 0, 50));
				Neo.Update();

				Thread.Sleep(500);
			}
		}
	}
}
