using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Options;

namespace Proj2.Code {
	public class ConfigurationService {
		public static ConfigurationService Instance {
			get; private set;
		}

		public IConfiguration Config {
			get; private set;
		}

		public string ConnectionString {
			get; private set;
		}

		public ConfigurationService(IConfiguration Config) {
			Console.WriteLine("Creating Configuration Service");

			this.Config = Config;
			Instance = this;

			string DatabaseName = "Database_Dev";
			Console.WriteLine("Using Database: {0}", DatabaseName);
			ConnectionString = Config.GetConnectionString(DatabaseName);
		}
	}
}
