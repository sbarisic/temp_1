using Proj2.Code;
using Proj2.Database;

namespace Proj2.Data {
    public class ItemDataService {
        DatabaseContext Db {
            get {
                return DatabaseService.Instance.Database;
            }
        }

        public DbItemData[] GetAllItems() {
            return Db.Items.ToArray();
        }

        public DbItemData GetItem(string ItemName) {
            return Db.Items.Where(DbItem => DbItem.Name == ItemName).FirstOrDefault();
        }

        public void UpdateItem(DbItemData Item) {
            Db.Items.Update(Item);
            Db.SaveChanges();
        }
    }
}