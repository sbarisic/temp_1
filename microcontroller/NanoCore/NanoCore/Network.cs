using System;
using System.Device.Wifi;
using System.Diagnostics;
using System.Net.NetworkInformation;
using System.Text;
using System.Threading;

namespace NanoCore {
	static class Network {
		static string SoftApIP = "192.168.42.69";
		static string SoftApSSID = "NanoFramework";
		static string SoftApPass = "12345678";

		static bool Setup() {
			NetworkInterface NI = GetInterface();
			WirelessAPConfiguration WapConf = GetConfiguration();

			if (WapConf.Options == (WirelessAPConfiguration.ConfigurationOptions.Enable | WirelessAPConfiguration.ConfigurationOptions.AutoStart) && NI.IPv4Address == SoftApIP) {
				return true;
			}

			NI.EnableStaticIPv4(SoftApIP, "255.255.255.0", SoftApIP);

			WapConf.Options = WirelessAPConfiguration.ConfigurationOptions.AutoStart | WirelessAPConfiguration.ConfigurationOptions.Enable;
			WapConf.Ssid = SoftApSSID;
			WapConf.MaxConnections = 1;
			WapConf.Authentication = AuthenticationType.WPA2;
			WapConf.Password = SoftApPass;
			WapConf.SaveConfiguration();

			return false;
		}

		public static NetworkInterface GetInterface() {
			NetworkInterface[] Interfaces = NetworkInterface.GetAllNetworkInterfaces();

			// Find WirelessAP interface
			foreach (NetworkInterface ni in Interfaces) {
				if (ni.NetworkInterfaceType == NetworkInterfaceType.WirelessAP) {
					return ni;
				}
			}

			return null;
		}

		public static WifiAdapter GetAdapter() {
			WifiAdapter[] Adapters = WifiAdapter.FindAllAdapters();
			WifiAdapter Ad = Adapters[0];
			return Ad;
		}

		public static WirelessAPConfiguration GetConfiguration() {
			NetworkInterface ni = GetInterface();
			return WirelessAPConfiguration.GetAllWirelessAPConfigurations()[ni.SpecificConfigId];
		}

		public static void Connect(string SSID, string Pass) {
			WifiAdapter Ad = GetAdapter();

			WifiConnectionResult Con = Ad.Connect(SSID, WifiReconnectionKind.Automatic, Pass);

			if (Con.ConnectionStatus == WifiConnectionStatus.Success) {
				Debug.WriteLine("Connected!");
				return;
			}

			Debug.WriteLine("Status: " + Con.ConnectionStatus.ToString());
			return;


			/*Ad.AvailableNetworksChanged += (Sender, E) => {
				Debug.WriteLine("AvailableNetworksChanged");

				foreach (WifiAvailableNetwork Net in Sender.NetworkReport.AvailableNetworks) {
					Debug.WriteLine($"Net SSID :{ Net.Ssid },  BSSID : { Net.Bsid },  rssi : { Net.NetworkRssiInDecibelMilliwatts },  signal : { Net.SignalBars }");
				}

			};

			while (true) {
				Ad.ScanAsync();
				Thread.Sleep(10000);
			}*/
		}
	}
}
