namespace Proj2.Code {
	public static class Utils {
		static Random Rnd = new Random();

		public static float RandomFloat(float Min, float Max) {
			return (float)(Min + Rnd.NextDouble() * (Max - Min));
		}

		public static DateTime SetKindUtc(this DateTime DT) {
			if (DT.Kind == DateTimeKind.Utc) {
				return DT;
			}

			return DateTime.SpecifyKind(DT, DateTimeKind.Utc);
		}
	}
}
