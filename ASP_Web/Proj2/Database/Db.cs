using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Infrastructure;
using Microsoft.EntityFrameworkCore.Storage;
using Microsoft.Extensions.Configuration;
using System.Reflection;
using System.Linq;
using System.Linq.Expressions;

using Proj2.Code;

using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;
using System.Net.NetworkInformation;
using System.Security.Cryptography;
using static System.Net.Mime.MediaTypeNames;

namespace Proj2.Database {
	public class DatabaseContext : DbContext {
		public static void CreateDb() {
			using (DatabaseContext DbCtx = new DatabaseContext()) {
				Console.Write("Recreating test database ...");
				DbCtx.Database.EnsureDeleted();
				DbCtx.Database.EnsureCreated();


				DbUser Usr_Admin = DbCtx.CreateNew<DbUser>(null, Usr => {
					Usr.SetUsernamePassword("admin", "admin");
				});

				DbCtx.AddPermission(Usr_Admin, DbPermission.PermissionNames.ADMIN, "", "");

				DbUser Usr_User = DbCtx.CreateNew<DbUser>(null, Usr => {
					Usr.SetUsernamePassword("user", "user");
				});

				DbAddress Addr1 = DbCtx.CreateNew<DbAddress>(null, Addr => {
					Addr.Street = "Ante Trumbica 1B";
					Addr.City = "Bjelovar";
					Addr.Region = "Bjelovarsko Bilogorska";
					Addr.PostalCode = "43000";
					Addr.PhoneNumber = "095 545 1181";
				});

				DbAdministration Test1 = DbCtx.CreateNew<DbAdministration>(Usr_Admin, Admin => {
					Admin.Name = "Glavna Uprava";
					Admin.Address = Addr1;
				});

				DbAddress Addr2 = DbCtx.CreateNew<DbAddress>(null, Addr => {
					Addr.Street = "Antuna Radica 52";
					Addr.City = "Bjelovar";
					Addr.Region = "Bjelovarsko Bilogorska";
					Addr.PostalCode = "43000";
					Addr.PhoneNumber = "091 816 0014";
				});

				DbAdministration Test2 = DbCtx.CreateNew<DbAdministration>(Usr_User, Admin => {
					Admin.Name = "Test 2";
					Admin.Address = Addr2;
				});


				DbVehicleEquipment Eq1 = DbCtx.CreateNew<DbVehicleEquipment>(null, Eq => {
					Eq.Name = "Akumulatori";
					Eq.EquipmentType = DbEquipmentType.BATTERY;
				});

				DbVehicle Veh1 = DbCtx.CreateNew<DbVehicle>(Usr_Admin, Veh => {
					Veh.Name = "Test Vozilo";
					Veh.LicensePlate = "BJ000AA";
				});

				Veh1.Equipment.Add(Eq1);
				Test1.Vehicles.Add(Veh1);

				Test2.Vehicles.Add(DbCtx.CreateNew<DbVehicle>(Usr_User, Veh => {
					Veh.Name = "Vozilo 2";
					Veh.LicensePlate = "BJ001AB";
				}));

				Test2.Vehicles.Add(DbCtx.CreateNew<DbVehicle>(Usr_User, Veh => {
					Veh.Name = "Vozilo 3";
					Veh.LicensePlate = "ZG1234HA";
				}));
				DbCtx.Commit();


				//DbCtx.AddPermission(Usr_User, DbPermission.PermissionNames.VIEW_ADMINISTRATION, Test1.ID, "");
				//DbCtx.AddPermission(Usr_User, DbPermission.PermissionNames.EDIT_ADMINISTRATION_DETAILS, Test1.ID, "");
				DbCtx.Commit();

				Console.WriteLine("OK");
			}
		}

		public string GenerateCreateScript() {
			RelationalDatabaseCreator DbCreator = (Database.GetService<IDatabaseCreator>() as RelationalDatabaseCreator);

			return DbCreator.GenerateCreateScript();
		}

		protected override void OnConfiguring(DbContextOptionsBuilder optionsBuilder) {
			optionsBuilder.UseLazyLoadingProxies().UseNpgsql(ConfigurationService.Instance.ConnectionString);
		}

		// Tables
		public DbSet<DbUser> Users {
			get; set;
		}

		public DbSet<DbPermission> AllPermissions {
			get; set;
		}

		public DbSet<DbDeviceAPIKey> APIKeys {
			get; set;
		}

		public DbSet<DbAddress> Addresses {
			get; set;
		}

		public DbSet<DbAdministration> Administrations {
			get; set;
		}

		public DbSet<DbVehicle> Vehicles {
			get; set;
		}

		public DbSet<DbVehicleEquipment> VehicleEquipment {
			get; set;
		}

		public DbSet<DbEquipmentValues> EquipmentValues {
			get; set;
		}

		public DbSet<DbEquipmentNotifications> EquipmentNotifications {
			get; set;
		}

		public DbSet<DbJsonLog> JsonLog {
			get; set;
		}

		// Functions

		/*public DbItemData[] GetAllItems() {
            return Items.OrderBy(Itm => Itm.ID).ToArray();
        }

        public DbItemData GetItem(string ItemName) {
            return Items.Where(DbItem => DbItem.Name == ItemName).FirstOrDefault();
        }

        public void UpdateItem(DbItemData Item) {
            Items.Update(Item);
            SaveChanges();
        }*/

		public void Commit() {
			SaveChanges();
		}

		DbSet<T> GetDbSet<T>() where T : DbTable {
			PropertyInfo[] Props = typeof(DatabaseContext).GetProperties();

			for (int i = 0; i < Props.Length; i++) {
				if (Props[i].PropertyType == typeof(DbSet<T>)) {
					PropertyInfo Prop = Props[i];

					DbSet<T> DbSet = (DbSet<T>)Prop.GetGetMethod().Invoke(this, null);

					return DbSet;
				}
			}

			return null;
		}

		public void Reload<T>(T Ent) where T : DbTable {
			Entry(Ent).Reload();
		}

		bool RecursionCheck = false;

		public T CreateNew<T>(DbUser PermissionsOwner, Action<T> Act = null) where T : DbTable, new() {
			if (RecursionCheck) {
				throw new Exception("Cannot call DbContext.CreateNew recursively");
			}

			T NewObject = new T();
			NewObject.InitializeNew();

			DbSet<T> DbSet = GetDbSet<T>();
			DbSet.Add(NewObject);

			if (Act != null) {
				RecursionCheck = true;
				Act(NewObject);
				RecursionCheck = false;
			}

			SaveChanges();
			Reload(NewObject);

			if (PermissionsOwner != null)
				SetupDefaultPermissions(PermissionsOwner, NewObject);

			return NewObject;
		}

		public void SetupDefaultPermissions(DbUser Owner, DbTable Item) {
			if (Item is DbAdministration Admin) {
				AddPermission(Owner, DbPermission.PermissionNames.VIEW_ADMINISTRATION, Admin.ID, Admin.ToString());
				AddPermission(Owner, DbPermission.PermissionNames.EDIT_ADMINISTRATION_DETAILS, Admin.ID, Admin.ToString());
				AddPermission(Owner, DbPermission.PermissionNames.EDIT_ADMINISTRATION_DETAILS, Admin.ID, Admin.ToString());
			} else if (Item is DbVehicle Veh) {
				AddPermission(Owner, DbPermission.PermissionNames.EDIT_VEHICLE_DETAILS, Veh.ID, Veh.ToString());
				AddPermission(Owner, DbPermission.PermissionNames.ADD_REMOVE_VEHICLE_EQUIPMENT, Veh.ID, Veh.ToString());
				AddPermission(Owner, DbPermission.PermissionNames.GENERATE_API_KEY, Veh.ID, Veh.ToString());
				AddPermission(Owner, DbPermission.PermissionNames.DELETE_ADMINISTRATION, Veh.ID, Veh.ToString());
			} else {
				throw new Exception("Cannot setup default permissions for DbTable item " + Item.GetType().Name);
			}
		}

		public void DeleteEntity<T>(T Ent) where T : DbTable {
			/*DbSet<T> DbSet = GetDbSet<T>();
			DbSet.Remove(Ent);*/

			Remove(Ent);
			SaveChanges();
		}

		public DbVehicle GetVehicle(string ID, bool DoNotTrack) {
			IQueryable<DbVehicle> VehQ = Vehicles.Where(V => V.ID == ID);

			if (DoNotTrack)
				VehQ = VehQ.AsNoTracking();

			DbVehicle Veh = VehQ.SingleOrDefault();
			return Veh;
		}

		public DbAdministration GetAdministration(string ID) {
			DbAdministration Admin = Administrations.Where(A => A.ID == ID).SingleOrDefault();

			//Entry(Admin).Reference(X => X.Address).Load();

			return Admin;
		}

		public DbAdministration GetAdministrationForVehicle(DbVehicle Veh) {
			DbAdministration Admin = Administrations.Where(A => A.Vehicles.Contains(Veh)).SingleOrDefault();

			return Admin;
		}

		public DbDeviceAPIKey GetDeviceAPIKey(string APIKey) {
			// TODO:
			DbDeviceAPIKey DbAPIKey = APIKeys.Where(AT => AT.APIKey == APIKey && AT.Enabled).SingleOrDefault();

			return DbAPIKey;
		}

		public List<DbVehicle> GetVehiclesByAPIKey(DbDeviceAPIKey APIKey) {
			List<DbVehicle> Vehicles = APIKeys.Where(K => K.ID == APIKey.ID).SelectMany(K => K.Vehicles).OrderBy(V => V.ID).ToList();

			//DbVehicle Veh = Vehicles.Where(Veh => Veh.APIKey.ID == APIKey.ID).SingleOrDefault();

			return Vehicles;
		}

		public DbDeviceAPIKey GetValidAPIKey(DbVehicle Veh) {
			DbDeviceAPIKey APIKey = APIKeys.Where(K => K.Enabled && K.Vehicles.Contains(Veh)).FirstOrDefault();

			return APIKey;
		}

		public void InvalidateAllAPIKeysForVehicle(DbVehicle Veh) {
			APIKeys.Where(K => K.Enabled && K.Vehicles.Contains(Veh)).ToList().ForEach(APIKey => APIKey.Enabled = false);
			Commit();
		}

		public DbUser GetUser(string Username) {
			DbUser Usr = Users.Where(U => U.Username == Username).SingleOrDefault();

			return Usr;
		}

		public DbUser GetUser(int UserID) {
			DbUser Usr = Users.Where(U => U.ID == UserID).SingleOrDefault();

			return Usr;
		}

		public List<DbUser> GetAllUsers() {
			return Users.ToList();
		}

		public List<DbAdministration> GetAllAdministrations(DbUser User) {
			if (HasPermission(User, DbPermission.PermissionNames.ADMIN))
				return Administrations.ToList();

			List<DbAdministration> AllAdmininstrations = new List<DbAdministration>();

			foreach (DbAdministration A in Administrations) {
				if (HasPermission(User, DbPermission.PermissionNames.VIEW_ADMINISTRATION, A.ID))
					AllAdmininstrations.Add(A);
			}

			return AllAdmininstrations;
		}

		public bool HasPermission(AuthStateProvider Provider, DbPermission.PermissionNames Permission) {
			return HasPermission<DbTable>(Provider, Permission, null);
		}

		public bool HasPermission<T>(AuthStateProvider Provider, DbPermission.PermissionNames Permission, T Value) where T : DbTable {
			DbUser Usr = Provider.GetDbUser(this);
			return HasPermission(Usr, Permission, Value?.GetPermissionID() ?? "");
		}

		public bool HasPermission(DbUser User, DbPermission.PermissionNames Permission, string Value = "") {
			if (User == null)
				return false;

			List<DbPermission> Permissions = User.Permissions;

			if (Permissions == null || Permissions.Count == 0)
				return false;

			for (int i = 0; i < Permissions.Count; i++) {
				if (Permissions[i].Permission == DbPermission.PermissionNames.ADMIN)
					return true;

				if (Permissions[i].Permission == Permission && Permissions[i].Value == Value)
					return true;
			}

			return false;
		}

		public void AddPermission(DbUser User, DbPermission.PermissionNames Permission, string Value, string Description) {
			if (HasPermission(User, Permission, Value))
				return;

			DbPermission NewPerm = CreateNew<DbPermission>(null, P => {
				P.Permission = Permission;
				P.Value = Value;
				P.Description = Description;
			});

			User.Permissions.Add(NewPerm);
			Commit();
		}

		public void RemovePermission(DbUser User, DbPermission.PermissionNames Permission, string Value) {
			if (User == null)
				return;

			List<DbPermission> Permissions = User.Permissions;

			if (Permissions == null || Permissions.Count == 0)
				return;

			bool HasChanges = false;

			for (int i = 0; i < Permissions.Count; i++) {
				if (Permissions[i].Permission == Permission && Permissions[i].Value == Value) {
					Permissions[i] = null;
					HasChanges = true;
				}
			}

			if (HasChanges) {
				User.Permissions = Permissions.Where(P => P != null).ToList();
				Commit();
			}
		}

		public void RemovePermission(DbUser Usr, DbPermission Perm) {
			Usr.Permissions.Remove(Perm);
			Commit();
		}

		KeyValuePair<string, object>[] GetAllNamesIDs<T>() where T : DbTable {
			return GetDbSet<T>().AsNoTracking().ToList().Select(V => new KeyValuePair<string, object>(V.GetName(), V.GetID())).ToArray();
		}

		public KeyValuePair<string, object>[] GetAllNamesIDsForType(Type T) {
			if (!T.IsSubclassOf(typeof(DbTable)))
				throw new Exception(string.Format("{0} is not a DbTable", T.FullName));

			MethodInfo GetAllValuesMethod = GetType().GetMethod(nameof(GetAllNamesIDs), BindingFlags.Instance | BindingFlags.NonPublic);

			KeyValuePair<string, object>[] Result = (KeyValuePair<string, object>[])GetAllValuesMethod.MakeGenericMethod(T).Invoke(this, null);
			return Result;
		}

		DbTable[] GetAllDbTableItems<T>() where T : DbTable {
			return GetDbSet<T>().AsNoTracking().ToArray();
		}

		public DbTable[] GetAllDbTableItemsForType(Type T) {
			if (!T.IsSubclassOf(typeof(DbTable)))
				throw new Exception(string.Format("{0} is not a DbTable", T.FullName));

			MethodInfo GetAllDbTableItemsMethod = GetType().GetMethod(nameof(GetAllDbTableItems), BindingFlags.Instance | BindingFlags.NonPublic);

			DbTable[] Result = (DbTable[])GetAllDbTableItemsMethod.MakeGenericMethod(T).Invoke(this, null);
			return Result;
		}

		public T Untrack<T>(T Object) where T : DbTable {
			Entry(Object).State = EntityState.Detached;
			return Object;
		}
	}
}
