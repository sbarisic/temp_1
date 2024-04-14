using System.Web;
using System.Collections.Specialized;
using System.Text.Json.Serialization;
using System.Text.Json.Nodes;
using System.Text.Json;
using Proj2.Database;

namespace Proj2.Code {
	public static class JsonUtils {
		public static JsonDocument Parse(string Source) {
			JsonDocument JsonDoc = JsonDocument.Parse(Source);
			return JsonDoc;
		}

		public static T Parse<T>(string Source) {
			return JsonSerializer.Deserialize<T>(Source, new JsonSerializerOptions() { ReadCommentHandling = JsonCommentHandling.Skip, AllowTrailingCommas = true });
		}

		public static IEnumerable<(string, object)> GetKeyValues(JsonDocument JsonDoc) {
			foreach (JsonProperty Prop in JsonDoc.RootElement.EnumerateObject()) {
				object Value = null;

				switch (Prop.Value.ValueKind) {
					case JsonValueKind.Object:
						Value = "(JsonObject)";
						break;

					case JsonValueKind.Array: {
							float[] ValueArray = new float[Prop.Value.GetArrayLength()];
							int Idx = 0;

							foreach (JsonElement Itm in Prop.Value.EnumerateArray()) {
								//if (Itm.trygetsin

								if (Itm.ValueKind == JsonValueKind.Number) {
									ValueArray[Idx] = Itm.GetSingle();
								} else if (Itm.ValueKind == JsonValueKind.String) {
									if (Utils.TryParseFloat(Itm.GetString(), out float F)) {
										ValueArray[Idx] = F;
									} else
										throw new Exception("Unable to parse number '" + Itm.GetString() + "'");

								} else
									throw new Exception("Unknown array element type " + Itm.ValueKind.ToString() + " = '" + Itm.GetRawText() + "'");


								Idx++;
							}

							Value = ValueArray;
							break;
						}

					case JsonValueKind.String:
						Value = Prop.Value.GetString();
						break;

					case JsonValueKind.Number:
						Value = (float)Prop.Value.GetDouble();
						break;

					case JsonValueKind.True:
					case JsonValueKind.False:
						Value = Prop.Value.GetBoolean();
						break;

					case JsonValueKind.Undefined:
					case JsonValueKind.Null:
						Value = null;
						break;

					default:
						throw new NotImplementedException();
				}

				yield return new(Prop.Name, Value);
			}
		}

		public static IEnumerable<(string, object)> GetKeyValues(DbJsonLog Log) {
			if (Log.ParseException)
				return new (string, object)[] { };

			JsonDocument JsonDoc = JsonDocument.Parse(Log.JsonString);
			return GetKeyValues(JsonDoc);
		}
	}
}
