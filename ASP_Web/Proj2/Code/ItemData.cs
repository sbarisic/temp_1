namespace Proj2.Code {
    public class ItemData {
        public DateTime TimeStamp {
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

        public override string ToString() {
            return string.Format("{0} - {1}", Name, Description);
        }
    }
}
