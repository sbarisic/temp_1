using Proj2.Code;

namespace Proj2.Data {
	public class ItemDataService {
		List<ItemData> Items = new List<ItemData>() {
			new ItemData(DateTime.Now, "Item1", 14.2f, "Test item 1"),
			new ItemData(DateTime.Now, "Item2", 12.0f, "Test item 2"),
			new ItemData(DateTime.Now, "Item3", 13.4f, "Test item 3"),
		};

		public Task<ItemData[]> GetAllItemsAsync() {
			return Task.FromResult(Items.ToArray());
		}

		public Task<ItemData> GetItemAsync(string ItemName) {
			ItemData Itm = Items.Where(Itm => Itm.Name == ItemName).FirstOrDefault();

			if (Itm == null)
				return Task.FromResult<ItemData>(null);

			return Task.FromResult(Itm);
		}
	}
}