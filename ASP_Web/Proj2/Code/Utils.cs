using System.Web;
using System.Collections.Specialized;
using System.Globalization;

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

		public static string ToShortString(Guid guid) {
			var base64Guid = Convert.ToBase64String(guid.ToByteArray());

			// Replace URL unfriendly characters
			base64Guid = base64Guid.Replace('+', '-').Replace('/', '_');

			// Remove the trailing ==
			return base64Guid.Substring(0, base64Guid.Length - 2);
		}

		public static Guid FromShortString(string str) {
			str = str.Replace('_', '/').Replace('-', '+');
			var byteArray = Convert.FromBase64String(str + "==");
			return new Guid(byteArray);
		}

		public static string GenerateShortID() {
			return ToShortString(Guid.NewGuid());
		}

		public static NameValueCollection ParseQuery(string Query) {
			Query = new Uri(Query).Query;

			return HttpUtility.ParseQueryString(Query);
		}

		public static string FormatDateTime(DateTime Dt, bool LongFormat = true) {
			string Fmt = "MM.yyyy HH:mm";

			if (LongFormat) {
				Fmt = "dd.MM.yyyy HH:mm:ss";
			}

			return Dt.ToString(Fmt);
		}

		public static bool TryParseFloat(string Str, out float F) {
			if (float.TryParse(Str, NumberStyles.Any, CultureInfo.InvariantCulture, out F))
				return true;

			return false;
		}

		/*public static string GenerateID() {
			return "id_" + Guid.NewGuid().ToString("N");
		}*/
	}
}
