using Microsoft.EntityFrameworkCore;

using Proj2.Code;

using System.ComponentModel.DataAnnotations;

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

		public DateTime TimeStamp {
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

		public DbItemData(DateTime TimeStamp, string Name, float Voltage, string Description) {
			this.TimeStamp = TimeStamp;
			this.Name = Name;
			this.Voltage = Voltage;
			this.Description = Description;
		}

		public override string ToString() {
			return string.Format("{0} - {1}", Name, Description);
		}
	}

	public class DatabaseContext : DbContext {
		public DbSet<DbUser> Users {
			get; set;
		}

		public DbSet<DbPermission> AllPermissions {
			get; set;
		}

		public DbSet<DbItemData> Items {
			get; set;
		}

		protected override void OnConfiguring(DbContextOptionsBuilder optionsBuilder) {
			optionsBuilder.UseSqlite("Data Source=local_sqlite.db");
		}
	}

	public class DatabaseService {
		public static DatabaseService Instance;

		public DatabaseContext Database;

		public DatabaseService() {
			Instance = this;
			Database = new DatabaseContext();

			Database.Database.EnsureDeleted();
			if (Database.Database.EnsureCreated()) {
				Console.WriteLine("Creating new database");

				Database.Users.Add(DbUser.CreateNew("admin", "admin"));
				Database.Users.Add(DbUser.CreateNew("user", "user"));

				Database.Items.Add(new DbItemData(DateTime.Now, "Item1", 14.2f, "Test item 1"));
				Database.Items.Add(new DbItemData(DateTime.Now, "Item2", 12.0f, "Test item 2"));
				Database.Items.Add(new DbItemData(DateTime.Now, "Item3", 13.4f, "Test item 3"));
				Database.Items.Add(new DbItemData(DateTime.Now, "Test", 12.13f, "Test Test"));

				Database.SaveChanges();
			} else
				Console.WriteLine("Using existing database");
		}
	}
}
