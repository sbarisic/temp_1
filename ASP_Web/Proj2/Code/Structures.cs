namespace Proj2.Code {
	public class Address {
		public string Street;
		public string City;
		public string Region;
		public string PostalCode;
		public string PhoneNumber;

		public Address(string street, string city, string region, string postalCode, string phoneNumber) {
			Street = street;
			City = city;
			Region = region;
			PostalCode = postalCode;
			PhoneNumber = phoneNumber;
		}
	}

	public class Vozilo {
		public string Name;
		public string Tablica;
		public List<OpremaVozila> Oprema = new List<OpremaVozila>();

		public Vozilo(string name, string tablica) {
			Name = name;
			Tablica = tablica;
		}

		public bool HasErrors() {
			foreach (OpremaVozila O in Oprema) {
				if (O.Error)
					return true;
			}

			return false;
		}

		public bool HasWarnings() {
			foreach (OpremaVozila O in Oprema) {
				if (O.Warning)
					return true;
			}

			return false;
		}
	}

	public class OpremaVozila {
		public string Name;

		public bool Error;
		public bool Warning;

		public OpremaVozila(string name) {
			Name = name;

			Error = false;
			Warning = false;
		}
	}

	public class Uprava {
		public string Name;
		public Address Address;
		public List<Vozilo> Vozila = new List<Vozilo>();

		public Uprava(string Name, Address Address) {
			this.Name = Name;
			this.Address = Address;
		}

		public bool HasErrors() {
			foreach (Vozilo V in Vozila)
				if (V.HasErrors())
					return true;

			return false;
		}

		public bool HasWarnings() {
			foreach (Vozilo V in Vozila)
				if (V.HasWarnings())
					return true;

			return false;
		}
	}

	public static class Structures {
		static Random Rnd = new Random(25848952);

		static string[] StreetArr = new[] { "Bana J. Jelačića 4", "Petra Zrisnkog 11", "Vjekoslava Bacha 18", "Trg maršala Tita 3", "Bobotine 2", "Teslina 2" };
		static string[] CityArr = new[] { "Zagreb", "Bjelovar", "Split", "Dubrovnik", "Rijeka", "Varaždin" };
		static string[] RegionArr = new[] { "Bjelovarsko-Bilogorska", "Zagrebačka", "Krapinsko-zagorska", "Sisačko-moslavačka", "Varaždinska", "Primorsko-goranska" };
		static string[] PostalCodeArr = new[] { "43000", "10000", "21000", "69000", "55000", "69420" };

		static string[] NumberPrefixA = new[] { "01", "020", "021", "022", "023", "031", "032", "033", "034", "035", "040", "042", "043", "044", "047", "048", "049", "051", "052", "053" };
		static string[] NumberPrefixB = new[] { "091", "092", "095", "097", "098", "099" };

		static string[] PhoneNumberArr;

		public static Address[] Address;

		//public static Vozilo[] Vozila;
		public static Uprava[] Uprave;


		static Structures() {
			PhoneNumberArr = new[] { GetRandomPhoneNumber(), GetRandomPhoneNumber(), GetRandomPhoneNumber(), GetRandomPhoneNumber(), GetRandomPhoneNumber(), GetRandomPhoneNumber() };

			Address = new[] {
				GetRandomAddress(),
				GetRandomAddress(),
				GetRandomAddress(),
				GetRandomAddress(),
				GetRandomAddress(),
				GetRandomAddress(),
				GetRandomAddress(),
				GetRandomAddress(),
				GetRandomAddress(),
				GetRandomAddress()
			};

			/*Vozila = new Vozilo[] {
				new Vozilo(GenerateVehicleName(), GenerateTablica()),
				new Vozilo(GenerateVehicleName(), GenerateTablica()),
				new Vozilo(GenerateVehicleName(), GenerateTablica()),
				new Vozilo(GenerateVehicleName(), GenerateTablica()),
				new Vozilo(GenerateVehicleName(), GenerateTablica()),
			};*/


			List<Uprava> UpraveTemp = new List<Uprava>();

			for (int i = 0; i < 3; i++) {
				Uprava Up = new Uprava("Uprava " + (i + 1).ToString(), Address[i]);

				int VoziloCount = 0;

				if (i != 2)
					VoziloCount = Rnd.Next(1, 6);

				for (int j = 0; j < VoziloCount; j++) {
					Vozilo V = new Vozilo(GenerateVehicleName(), GenerateTablica());

					OpremaVozila B1 = new OpremaVozila("Battery 1");
					OpremaVozila B2 = new OpremaVozila("Battery 2");

					if (i == 1) {
						if (j == 0) {
							B2.Error = true;

						} else if (j == 1) {
							B2.Warning = true;
						}
					}

					V.Oprema.Add(B1);
					V.Oprema.Add(B2);

					Up.Vozila.Add(V);
				}

				UpraveTemp.Add(Up);
			}

			Uprave = UpraveTemp.ToArray();
		}

		public static T GetRandom<T>(T[] Set) {
			return Set[Rnd.Next(0, Set.Length)];
		}

		public static string GetRandomNumberString(int Len) {
			int[] Numbers = new int[Len];

			for (int i = 0; i < Numbers.Length; i++) {
				Numbers[i] = Rnd.Next(0, 10);
			}

			return string.Join("", Numbers);
		}

		public static string GenerateLetter(bool Caps) {
			char FirstLetter = 'a';

			if (Caps)
				FirstLetter = 'A';

			return "" + (char)(FirstLetter + Rnd.Next(0, 26));
		}

		public static Address GetRandomAddress() {
			return new Address(GetRandom(StreetArr), GetRandom(CityArr), GetRandom(RegionArr), GetRandom(PostalCodeArr), GetRandom(PhoneNumberArr));
		}

		public static string GetRandomPhoneNumber() {
			int Len = Rnd.Next(6, 8);

			if (Len == 6)
				return GetRandom(NumberPrefixA) + " " + GetRandomNumberString(3) + " " + GetRandomNumberString(3);

			return GetRandom(NumberPrefixB) + " " + GetRandomNumberString(3) + " " + GetRandomNumberString(4);
		}

		public static string GenerateTablica() {
			string[] Prefix = new[] { "BJ", "BM", "ČK", "DA", "DE", "DJ", "DU", "GS", "IM", "KA", "KC", "KR", "KT", "KŽ", "MA", "NA", "NG", "OG", "OS", "PU", "PŽ", "RI", "SB", "SK", "SL", "ST", "ŠI", "VK", "VT", "VU", "VŽ", "ZD", "ZG", "ŽU" };

			int Len = 3;

			if (Rnd.Next(0, 101) > 70)
				Len = 4;


			return GetRandom(Prefix) + GetRandomNumberString(Len) + GenerateLetter(true) + GenerateLetter(true);
		}

		public static string GenerateVehicleName() {
			string[] Prefix = new[] { "Divni", "Veliki", "Smiješni", "Prekrasni", "Zanimljivi", "Pametni", "Snažni", "Lijepi" };
			string[] VehicleNames = new[] { "Auto", "Žujo", "Truli", "Žabac", "Fićo", "Rikverc", "Bubi", "Pufo" };
			return GetRandom(Prefix) + " " + GetRandom(VehicleNames);
		}
	}
}
