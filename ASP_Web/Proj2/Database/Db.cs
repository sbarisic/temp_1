﻿using Microsoft.EntityFrameworkCore;
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

namespace Proj2.Database {
	public class DatabaseContext : DbContext {
		public void CreateMissingTables() {
			RelationalDatabaseCreator DbCreator = (Database.GetService<IDatabaseCreator>() as RelationalDatabaseCreator);
			//DbCreator.CreateTables();

			string CreateScript = DbCreator.GenerateCreateScript();
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

		public T CreateNew<T>(Action<T> Act = null) where T : DbTable, new() {
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

			return NewObject;
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

		public void AddPermission(DbUser User, DbPermission.PermissionNames Permission, string Value) {
			if (HasPermission(User, Permission, Value))
				return;

			DbPermission NewPerm = CreateNew<DbPermission>(P => {
				P.Permission = Permission;
				P.Value = Value;
			});

			User.Permissions.Add(NewPerm);
			Commit();
		}
	}
}
