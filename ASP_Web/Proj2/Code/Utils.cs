namespace Proj2.Code {
	public static class Utils {
		static Random Rnd = new Random();

		public static float RandomFloat(float Min, float Max) {
			return (float)(Min + Rnd.NextDouble() * (Max - Min));
		}
	}
}
