namespace Proj2.Code {
	public class ItemData {
		public DateTime TimeStamp;
		public string Name;
		public float Voltage;
		public string Description;

		public ItemData() {
		}

		public ItemData(DateTime TimeStamp, string Name, float Voltage, string Description) {
			this.TimeStamp = TimeStamp;
			this.Name = Name;
			this.Voltage = Voltage;
			this.Description = Description;
		}

		public void CopyFrom(ItemData Other) {
			this.TimeStamp = Other.TimeStamp;
			this.Voltage = Other.Voltage;
			this.Name = Other.Name;
			this.Description = Other.Description;
		}
	}
}
