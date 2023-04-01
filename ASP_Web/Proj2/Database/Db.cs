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

namespace Proj2.Database {
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

        public T CreateNew<T>(Action<T> Act) where T : DbTable, new() {
            T NewObject = new T();
            NewObject.InitializeNew();

            PropertyInfo[] Props = typeof(DatabaseContext).GetProperties();

            for (int i = 0; i < Props.Length; i++) {
                if (Props[i].PropertyType == typeof(DbSet<T>)) {
                    PropertyInfo Prop = Props[i];

                    DbSet<T> DbSet = (DbSet<T>)Prop.GetGetMethod().Invoke(this, null);

                    DbSet.Add(NewObject);
                    //SaveChanges();
                }
            }

            Act(NewObject);

            SaveChanges();
            Entry(NewObject).Reload();

            return NewObject;
        }

        public DbVehicle GetVehicle(string ID) {
            DbVehicle Veh = Vehicles.Where(V => V.ID == ID).FirstOrDefault();

            return Veh;
        }

        public DbAdministration GetAdministration(string ID) {
            DbAdministration Admin = Administrations.Where(A => A.ID == ID).FirstOrDefault();

            Entry(Admin).Reference(X => X.Address).Load();

            return Admin;
        }
    }
}
