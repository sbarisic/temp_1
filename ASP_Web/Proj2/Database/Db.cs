using Microsoft.EntityFrameworkCore;
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

		public string Salt {
			get; set;
		}

		public List<DbPermission> Permissions {
			get;
		} = new List<DbPermission>();
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

	public class DatabaseContext : DbContext {
		public DbSet<DbUser> Users {
			get; set;
		}

		public DbSet<DbPermission> AllPermissions {
			get; set;
		}

		protected override void OnConfiguring(DbContextOptionsBuilder optionsBuilder) {
			optionsBuilder.UseSqlite("Data Source=local_sqlite.db");
		}
	}

	public class DatabaseService {
		public DatabaseContext Database;

		public DatabaseService() {
			Database = new DatabaseContext();
		}
	}
}
