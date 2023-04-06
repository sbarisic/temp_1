using System.Collections;

namespace Proj2.Code {
	interface IKeyKeyValue {
		public string OptionTitle {
			get;
		}

		public string OptionValue {
			get;
		}
	}

	struct KeyKeyValue<T> : IKeyKeyValue {
		public string Key1 {
			get;
		}

		public string Key2 {
			get;
		}

		public T Value {
			get;
		}

		public string OptionTitle {
			get {
				return Key1;
			}
		}

		public string OptionValue {
			get {
				return Key2;
			}
		}

		public KeyKeyValue(string K1, string K2, T V) {
			Key1 = K1;
			Key2 = K2;
			Value = V;
		}
	}

	public class KeyValueOptions<T> : IEnumerable<KeyKeyValue<T>> {
		List<KeyKeyValue<T>> Options = new List<KeyKeyValue<T>>();
		int Slot = 0;

		public string Selected {
			get; set;
		}

		public T SelectedValue {
			get {
				if (string.IsNullOrEmpty(Selected))
					return default(T);

				foreach (KeyKeyValue<T> O in Options) {
					if (O.Key2 == Selected)
						return O.Value;
				}

				return default(T);
			}
		}

		public KeyValueOptions() {
		}

		public void Add(string Key, T Value) {
			string Key2 = "key_" + (Slot++);
			Options.Add(new KeyKeyValue<T>(Key, Key2, Value));
		}

		public void Add(KeyValuePair<string, T> KV) {
			Add(KV.Key, KV.Value);
		}

		public void ClearSelected() {
			Selected = "";
		}

		public void Clear() {
			Selected = "";
			Slot = 0;
			Options.Clear();
		}

		IEnumerator<KeyKeyValue<T>> IEnumerable<KeyKeyValue<T>>.GetEnumerator() {
			return Options.GetEnumerator();
		}

		public IEnumerator GetEnumerator() {
			return Options.GetEnumerator();
		}
	}
}
