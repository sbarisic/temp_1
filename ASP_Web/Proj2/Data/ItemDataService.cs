using Proj2.Code;

namespace Proj2.Data {
    public class ItemDataService {
        List<ItemData> Items;

        public ItemDataService() {
            Items = new List<ItemData>();
            Items.Add(new ItemData(DateTime.Now, "Item1", 14.2f, "Test item 1"));
            Items.Add(new ItemData(DateTime.Now, "Item2", 12.0f, "Test item 2"));
            Items.Add(new ItemData(DateTime.Now, "Item3", 13.4f, "Test item 3"));

            
        }

        public ItemData[] GetAllItems() {
            return Items.ToArray();
        }

        public ItemData GetItem(string ItemName) {
            ItemData Itm = Items.Where(Itm => Itm.Name == ItemName).FirstOrDefault();

            if (Itm == null)
                return null;

            return Itm;
        }
    }
}