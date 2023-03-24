using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Infrastructure;
using Microsoft.EntityFrameworkCore.Storage;
using Microsoft.Extensions.Configuration;
using Proj2.Code;
using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;
using System.Net.NetworkInformation;
using System.Security.Cryptography;

namespace Proj2.Database {
	[PrimaryKey(nameof(ID))]
	public class DbUser {
		[Key]
		public int ID {
			get; set;
		}

		[Key]
		[Required]
		public string Username {
			get; set;
		}

		[Required]
		public string Hash {
			get; set;
		}

		[Required]
		public byte[] Salt {
			get; set;
		}

		public List<DbPermission> Permissions {
			get;
			set;
		}

		public static DbUser CreateNew(string Username, string Password) {
			DbUser Usr = new DbUser();
			Usr.Username = Username;

			using (PasswdManager PassMgr = new PasswdManager()) {
				Usr.Salt = PassMgr.GenerateSalt();
				Usr.Hash = PassMgr.GenerateHash(Password, Usr.Salt);
			}

			return Usr;
		}
	}

	public class DbPermission {
		[Key]
		public int ID {
			get; set;
		}

		public string Name {
			get; set;
		}
	}

	public class DbItemData {
		[Key]
		public int ID {
			get; set;
		}

		public string Name {
			get; set;
		}

		public float Voltage {
			get; set;
		}

		public string Description {
			get; set;
		}

		/*public DbItemData(string Name, float Voltage, string Description) {
			//this.TimeStamp = TimeStamp.SetKindUtc();
			this.Name = Name;
			this.Voltage = Voltage;
			this.Description = Description;
		}*/

		public override string ToString() {
			return string.Format("{0} - {1}", Name, Description);
		}
	}

	[PrimaryKey(nameof(ID))]
	public class DbDeviceAPIKey {
		[Key]
		public int ID {
			get; set;
		}

		[Key]
		[Required]
		public string APIKey {
			get; set;
		}

		public string Description {
			get; set;
		}

		[Required]
		public bool Enabled {
			get; set;
		}

		public static DbDeviceAPIKey CreateNew(string Description) {
			DbDeviceAPIKey DeviceAPIKey = new DbDeviceAPIKey();

			byte[] APIKeyBytes = RandomNumberGenerator.GetBytes(256 / 8);
			DeviceAPIKey.APIKey = Convert.ToBase64String(APIKeyBytes);

			DeviceAPIKey.Description = Description;
			DeviceAPIKey.Enabled = true;
			return DeviceAPIKey;
		}
	}

	public class DatabaseContext : DbContext {
		public void CreateMissingTables() {
			RelationalDatabaseCreator DbCreator = (Database.GetService<IDatabaseCreator>() as RelationalDatabaseCreator);
			//DbCreator.CreateTables();

			string CreateScript = DbCreator.GenerateCreateScript();
		}

		protected override void OnConfiguring(DbContextOptionsBuilder optionsBuilder) {
			optionsBuilder.UseNpgsql(ConfigurationService.Instance.ConnectionString);
		}

		// Tables
		public DbSet<DbUser> Users {
			get; set;
		}

		public DbSet<DbPermission> AllPermissions {
			get; set;
		}

		public DbSet<DbItemData> Items {
			get; set;
		}

		public DbSet<DbDeviceAPIKey> APIKeys {
			get; set;
		}

		// Functions

		public DbItemData[] GetAllItems() {
			return Items.OrderBy(Itm => Itm.ID).ToArray();
		}

		public DbItemData GetItem(string ItemName) {
			return Items.Where(DbItem => DbItem.Name == ItemName).FirstOrDefault();
		}

		public void UpdateItem(DbItemData Item) {
			Items.Update(Item);
			SaveChanges();
		}
	}
}
