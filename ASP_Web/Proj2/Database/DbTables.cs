using Microsoft.EntityFrameworkCore;

using Proj2.Code;

using System.ComponentModel.DataAnnotations;
using System.Security.Cryptography;
using System.Xml.Linq;

namespace Proj2.Database {
    public abstract class DbTable {
        public virtual void InitializeNew() {
        }

        public virtual string GetPermissionID() {
            throw new NotImplementedException();
        }

        public abstract object GetID();

        public abstract string GetName();

        public override string ToString() {
            return string.Format("[{0}] {1}", GetType().Name, GetName());
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

        public override object GetID() {
            return ID;
        }

        public override string GetName() {
            return Username;
        }

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
        public enum PermissionNames : int {
            INVALID = 0,
            ADMIN = 1, // Everything

            VIEW_ADMINISTRATION, // ID of administration
            EDIT_ADMINISTRATION_DETAILS, // ID of administration, edit details of administration
            ADD_REMOVE_ADMINISTRATION_VEHICLE, // ID of administration, add/remove administration vehicles

            EDIT_VEHICLE_DETAILS, // ID of vehicle, edit details of vehicle
            ADD_REMOVE_VEHICLE_EQUIPMENT, // ID of vehicle, add/remove vehicle equipment
            GENERATE_API_KEY, // ID of vehicle, ability to generate API key for vehicle
            DELETE_ADMINISTRATION, // ID of vehicle, ability to generate API key for vehicle
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

        public string Description {
            get; set;
        }

        public static KeyValuePair<string, PermissionNames>[] GetPermissions() {
            List<KeyValuePair<string, PermissionNames>> Ret = new List<KeyValuePair<string, PermissionNames>>();

            string[] Names = Enum.GetNames<PermissionNames>();

            for (int i = 0; i < Names.Length; i++) {
                string Name = Names[i];
                PermissionNames Val = Enum.Parse<PermissionNames>(Name);

                Ret.Add(new KeyValuePair<string, PermissionNames>(Name, Val));
            }

            return Ret.ToArray();
        }

        public static Type GetValidTableType(PermissionNames Perm) {
            switch (Perm) {
                case PermissionNames.INVALID:
                    throw new InvalidOperationException();

                case PermissionNames.ADMIN:
                    return null;

                case PermissionNames.VIEW_ADMINISTRATION:
                case PermissionNames.EDIT_ADMINISTRATION_DETAILS:
                case PermissionNames.ADD_REMOVE_ADMINISTRATION_VEHICLE:
                    return typeof(DbAdministration);

                case PermissionNames.EDIT_VEHICLE_DETAILS:
                case PermissionNames.ADD_REMOVE_VEHICLE_EQUIPMENT:
                case PermissionNames.GENERATE_API_KEY:
                case PermissionNames.DELETE_ADMINISTRATION:
                    return typeof(DbVehicle);

                default:
                    throw new NotImplementedException();
            }
        }

        public override object GetID() {
            return ID;
        }

        public override string GetName() {
            return GetType().Name;
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

        public override object GetID() {
            return ID;
        }

        public override string GetName() {
            return Name;
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

        public override object GetID() {
            return ID;
        }

        public override string GetName() {
            return GetType().Name;
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

        public override object GetID() {
            return ID;
        }

        public override string GetName() {
            return GetType().Name;
        }
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

        public override string GetPermissionID() {
            return ID;
        }

        public bool HasErrors() {
            return false;
        }

        public bool HasWarnings() {
            return false;
        }

        public override object GetID() {
            return ID;
        }

        public override string GetName() {
            return Name;
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

        public override string GetPermissionID() {
            return ID;
        }

        public override object GetID() {
            return ID;
        }

        public override string GetName() {
            return Name;
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

        public virtual List<DbEquipmentNotifications> Notifications {
            get; set;
        } = new();

        public override void InitializeNew() {
            ID = Utils.GenerateShortID();
        }

        public override object GetID() {
            return ID;
        }

        public override string GetName() {
            return Name;
        }
    }

    [PrimaryKey(nameof(ID))]
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

        public override object GetID() {
            return ID;
        }

        public override string GetName() {
            return GetType().Name;
        }
    }

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

        public DateTime Local_CreatedOn {
            get {
                return CreatedOn.ToLocalTime();
            }
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

        public override object GetID() {
            return ID;
        }

        public override string GetName() {
            return GetType().Name;
        }
    }

    [PrimaryKey(nameof(ID))]
    public class DbJsonLog : DbTable {
        [Key]
        public int ID {
            get; set;
        }

        [Required]
        public DateTime CreatedOn {
            get; set;
        }

        [Required]
        public string JsonString {
            get; set;
        }

        public virtual DbDeviceAPIKey CreatedByKey {
            get; set;
        }

        public string ParseExcMessage {
            get; set;
        }

        public string ParseExcSource {
            get; set;
        }

        public string ParseExcStackTrace {
            get; set;
        }

        public bool ParseException {
            get; set;
        }

        public override void InitializeNew() {
            CreatedOn = DateTime.Now.ToUniversalTime();
            ParseException = false;
        }

        public override object GetID() {
            return ID;
        }

        public override string GetName() {
            return GetType().Name;
        }
    }

    /*[PrimaryKey(nameof(ID))]
    public class DbKeyValue : DbTable {
        [Key]
        public int ID {
            get; set;
        }

        [Required]
        public DateTime CreatedOn {
            get; set;
        }

        public string Key {
            get; set;
        }

        public string Value {
            get; set;
        }

        public override object GetID() {
            return ID;
        }

        public override string GetName() {
            return GetType().Name;
        }
    }*/

    public enum DbEquipmentType : int {
        NONE = 0,
        BATTERY = 1,
        PRESSURE = 2
    }
}
