namespace Proj2.Code {
	public class GarbageCollector {
		IDisposable[] Disposables;
		IAsyncDisposable[] DisposablesAsync;

		public T Add<T>(T Item) {
			if (Item is IDisposable Item1) {

				if (Disposables == null) 
					Disposables = new IDisposable[0];

				Array.Resize(ref Disposables, Disposables.Length + 1);
				Disposables[Disposables.Length - 1] = Item1;

			} else if (Item is IAsyncDisposable Item2) {

				if (DisposablesAsync == null)
					DisposablesAsync = new IAsyncDisposable[0];

				Array.Resize(ref DisposablesAsync, DisposablesAsync.Length + 1);
				DisposablesAsync[DisposablesAsync.Length - 1] = Item2;

			} else
				throw new NotImplementedException("Item is not IDisposable or IAsyncDisposable");

			return Item;
		}


		public void DisposeAll() {
			for (int i = 0; i < Disposables.Length; i++) {
				if (Disposables[i] != null)
					Disposables[i].Dispose();
			}

			Disposables = null;

			for (int i = 0; i < DisposablesAsync.Length; i++) {
				if (DisposablesAsync[i] != null)
					DisposablesAsync[i].DisposeAsync();
			}

			DisposablesAsync = null;
		}
	}
}
