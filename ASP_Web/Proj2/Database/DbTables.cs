using Microsoft.EntityFrameworkCore;

using Proj2.Code;

using System.ComponentModel.DataAnnotations;
using System.Security.Cryptography;

namespace Proj2.Database {
	public class DbTable {
		public virtual void InitializeNew() {
		}
	}

	[PrimaryKey(nameof(ID))]
	public class DbUser : DbTable {
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

		public virtual List<DbPermission> Permissions { get; set; } = new();

		public void SetUsernamePassword(string Username, string Password) {
			this.Username = Username;

			using (PasswdManager PassMgr = new PasswdManager()) {
				this.Salt = PassMgr.GenerateSalt();
				this.Hash = PassMgr.GenerateHash(Password, this.Salt);
			}
		}

		/*public static DbUser CreateNew(string Username, string Password) {
            DbUser Usr = new DbUser();
            Usr.Username = Username;

            using (PasswdManager PassMgr = new PasswdManager()) {
                Usr.Salt = PassMgr.GenerateSalt();
                Usr.Hash = PassMgr.GenerateHash(Password, Usr.Salt);
            }

            return Usr;
        }*/
	}

	public class DbPermission : DbTable {
		[Flags]
		public enum PermissionNames : int {
			INVALID = 0,
			ADMIN = 1,

			VIEW_ADMINISTRATION = 2,
			VIEW_VEHICLE = 4,

			EDIT_ADMINISTRATION = 8,
			EDIT_VEHICLE = 16,

			VIEW_EDIT_ADMINISTRATION = VIEW_ADMINISTRATION | EDIT_ADMINISTRATION,
			VIEW_EDIT_VEHICLE = VIEW_VEHICLE | EDIT_VEHICLE
		}

		[Key]
		public int ID {
			get; set;
		}

		public PermissionNames Permission {
			get; set;
		}

		public string Value {
			get; set;
		}

	}

	public class DbItemData : DbTable {
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

		public override string ToString() {
			return string.Format("{0} - {1}", Name, Description);
		}
	}

	[PrimaryKey(nameof(ID))]
	public class DbDeviceAPIKey : DbTable {
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

		public virtual List<DbVehicle> Vehicles {
			get; set;
		} = new();

		/*public static DbDeviceAPIKey CreateNew(string Description) {
            DbDeviceAPIKey DeviceAPIKey = new DbDeviceAPIKey();

            byte[] APIKeyBytes = RandomNumberGenerator.GetBytes(256 / 8);
            DeviceAPIKey.APIKey = Convert.ToBase64String(APIKeyBytes);

            DeviceAPIKey.Description = Description;
            DeviceAPIKey.Enabled = true;
            return DeviceAPIKey;
        }*/

		public override void InitializeNew() {
			byte[] APIKeyBytes = RandomNumberGenerator.GetBytes(256 / 8);

			APIKey = Convert.ToBase64String(APIKeyBytes);
			Description = "Empty Description";
			Enabled = true;
		}
	}

	[PrimaryKey(nameof(ID))]
	public class DbAddress : DbTable {
		[Key]
		public int ID {
			get; set;
		}

		public string Street {
			get; set;
		}

		public string City {
			get; set;
		}

		public string Region {
			get; set;
		}

		public string PostalCode {
			get; set;
		}

		public string PhoneNumber {
			get; set;
		}

		/*public static DbAddress CreateNew(string street, string city, string region, string postalCode, string phoneNumber) {
            DbAddress Address = new DbAddress();

            Address.Street = street;
            Address.City = city;
            Address.Region = region;
            Address.PostalCode = postalCode;
            Address.PhoneNumber = phoneNumber;

            return Address;
        }*/
	}

	[PrimaryKey(nameof(ID))]
	public class DbAdministration : DbTable {
		[Key]
		public string ID {
			get; set;
		}

		[Required]
		public string Name {
			get; set;
		}

		[Required]
		public virtual DbAddress Address {
			get; set;
		}

		public virtual List<DbVehicle> Vehicles {
			get; set;
		} = new();

		/*public DbAdministration CreateNew(string Name, DbAddress Address) {
            DbAdministration Admin = new DbAdministration();

            Admin.ID = Utils.GenerateShortID();
            Admin.Name = Name;
            Admin.Address = Address;

            return Admin;
        }*/

		public override void InitializeNew() {
			ID = Utils.GenerateShortID();
		}

		public bool HasErrors() {
			return false;
		}

		public bool HasWarnings() {
			return false;
		}
	}

	[PrimaryKey(nameof(ID))]
	public class DbVehicle : DbTable {
		[Key]
		public string ID {
			get; set;
		}

		[Required]
		public string Name {
			get; set;
		}

		[Key]
		[Required]
		public string LicensePlate {
			get; set;
		}

		public virtual List<DbVehicleEquipment> Equipment {
			get; set;
		} = new();

		public bool HasErrors() {
			return false;
		}

		public bool HasWarnings() {
			return false;
		}

		public override void InitializeNew() {
			ID = Utils.GenerateShortID();
		}
	}

	[PrimaryKey(nameof(ID))]
	public class DbVehicleEquipment : DbTable {
		[Key]
		public string ID {
			get; set;
		}

		[Required]
		public string Name {
			get; set;
		}

		[Required]
		public DbEquipmentType EquipmentType {
			get; set;
		}

		public virtual List<DbEquipmentValues> Values {
			get; set;
		} = new();

		/*public virtual List<DbEquipmentNotifications> Notifications {
			get; set;
		} = new();*/

		public override void InitializeNew() {
			ID = Utils.GenerateShortID();
		}
	}

	/*[PrimaryKey(nameof(ID))]
	public class DbEquipmentNotifications : DbTable {
		[Key]
		public int ID {
			get; set;
		}

		[Required]
		public string Message {
			get; set;
		}

		public string Comment {
			get; set;
		}

		public bool IsResolved {
			get; set;
		}

		[Required]
		public DateTime CreatedOn {
			get; set;
		}

		public override void InitializeNew() {
			CreatedOn = DateTime.Now.ToUniversalTime();
		}
	}*/

	[PrimaryKey(nameof(ID))]
	public class DbEquipmentValues : DbTable {
		[Key]
		public int ID {
			get; set;
		}

		[Required]
		public DateTime CreatedOn {
			get; set;
		}

		[Required]
		public virtual DbDeviceAPIKey CreatedByKey {
			get; set;
		}

		[Required]
		public float FloatValue {
			get; set;
		}

		public override void InitializeNew() {
			CreatedOn = DateTime.Now.ToUniversalTime();
		}
	}

	public enum DbEquipmentType : int {
		NONE = 0,
		BATTERY = 1,
		PRESSURE = 2
	}
}
