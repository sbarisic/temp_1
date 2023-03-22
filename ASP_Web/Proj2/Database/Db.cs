using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Configuration;
using Proj2.Code;
using System.ComponentModel.DataAnnotations;
using System.Net.NetworkInformation;

namespace Proj2.Database {
	public class DbUser {
		[Key]
		public int ID {
			get; set;
		}

		public string Username {
			get; set;
		}

		public string Hash {
			get; set;
		}

		public byte[] Salt {
			get; set;
		}

		public List<DbPermission> Permissions {
			get;
		} = new List<DbPermission>();

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

		public DbItemData() {
		}

		public DbItemData(string Name, float Voltage, string Description) {
			//this.TimeStamp = TimeStamp.SetKindUtc();
			this.Name = Name;
			this.Voltage = Voltage;
			this.Description = Description;
		}

		public override string ToString() {
			return string.Format("{0} - {1}", Name, Description);
		}
	}

	public class DatabaseContext : DbContext {
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
