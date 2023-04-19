using Microsoft.AspNetCore.Mvc;
using System.Globalization;
using Proj2.Database;
using Microsoft.AspNetCore.SignalR;
using Proj2.Hubs;
using System.Threading.Tasks;
using System.Text.Json.Serialization;
using System.Text.Json.Nodes;
using System.Text.Json;

namespace Proj2.Code {
	public enum DeviceAccessStatus : int {
		Invalid = 0,
		OK = 200,
		Forbidden = 403,
		NotFound = 404,
		BadJson = 469,
	}

	public enum DeviceAction : int {
		Invalid = 0,

		// Set equipment data value
		Data = 1,

		// Get all equipment assigned to API key
		GetEquipment = 2
	}

	public class DeviceAccessAPI {
		public string APIKey {
			get; set;
		}

		public DeviceAction Action {
			get; set;
		}

		public float Napon1 {
			get; set;
		}

		public float Napon2 {
			get; set;
		}

		public float Temperatura {
			get; set;
		}

		public float Tlak {
			get; set;
		}
	}

	public class DeviceAccessResponseAPI {
		public string Title {
			get; set;
		}

		public DeviceAccessStatus Status {
			get; set;
		}

		public string StatusString {
			get; set;
		}

		public string Message {
			get; set;
		}

		public DbVehicleEquipment[] Equipment {
			get; set;
		}

		public DeviceAccessResponseAPI(DeviceAccessStatus Status, string Message = null) {
			Title = nameof(DeviceAccessResponseAPI);

			this.Status = Status;
			StatusString = Status.ToString();
			Equipment = null;
			this.Message = Message;
		}
	}

	[ApiController]
	[RequireHttps]
	public class DeviceAccessController : ControllerBase {
		IHubContext<ComHub> HubContext;

		public DeviceAccessController(IHubContext<ComHub> HubContext) {
			this.HubContext = HubContext;
		}

		JsonResult Invalid() {
			return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.Invalid));
		}

		JsonResult BadJson(string Message) {
			return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.BadJson, Message));
		}

		JsonResult Forbidden() {
			return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.Forbidden));
		}

		[HttpPost("/deviceaccess")]
		public JsonResult Dev([FromBody] string JsonString) {
			using (DatabaseContext DbCtx = new DatabaseContext()) {
				DbJsonLog JsonLog = ParseJsonBody(DbCtx, JsonString);

				if (JsonLog.ParseException) {
					return BadJson(JsonLog.ParseExcMessage);
				}

				DeviceAccessAPI API = JsonUtils.Parse<DeviceAccessAPI>(JsonString);

				// Return invalid API calls
				if (API == null || API.Action == 0)
					return Invalid();

				if (string.IsNullOrEmpty(API.APIKey))
					return Forbidden();

				bool DispatchChanges = false;
				DbVehicleEquipment[] Equipment = null;
				DbDeviceAPIKey DbAPIKey = DbCtx.GetDeviceAPIKey(API.APIKey);

				// If no API key, forbidden
				if (DbAPIKey == null || DbAPIKey.APIKey != API.APIKey)
					return Forbidden();

				switch (API.Action) {
					// Post data to vehicle equipment
					case DeviceAction.Data: {
							DispatchChanges = SetEquipmentData(DbCtx, DbAPIKey, JsonLog);
							break;
						}

					// Get all vehicle equipment by key
					case DeviceAction.GetEquipment: {
							Equipment = GetAllEquipmentForKey(DbCtx, API, DbAPIKey).ToArray();
							break;
						}

					default:
						return Forbidden();
				}

				// Dispatch state changed event to all clients

				// TODO: Displatch only to vehicle
				if (DispatchChanges) {
					HubContext.Clients.All.SendAsync("OnStateHasChanged").Wait();
				}

				DeviceAccessResponseAPI ResponseAPI = new DeviceAccessResponseAPI(DeviceAccessStatus.OK);
				//ResponseAPI.AllEquipment = AllEquipment;
				//ResponseAPI.AllEquipmentTypes = AllEquipmentTypes;
				//ResponseAPI.AllEquipmentNames = AllEquipmentNames;

				ResponseAPI.Equipment = Equipment;
				return new JsonResult(ResponseAPI);
			}
		}

		DbJsonLog ParseJsonBody(DatabaseContext DbCtx, string JsonString) {
			JsonDocument JsonDoc = null;

			DbJsonLog JsonLog = DbCtx.CreateNew<DbJsonLog>(null, (JsonLog) => {
				JsonLog.JsonString = JsonString;
			});

			try {
				JsonDoc = JsonDocument.Parse(JsonString);
				JsonElement APIKeyNode = JsonDoc.RootElement.GetProperty("APIKey");

				if (JsonDoc.RootElement.TryGetProperty("APIKey", out JsonElement APIKeyElement)) {
					string APIKeyString = APIKeyElement.GetString();

					DbDeviceAPIKey APIKey = DbCtx.GetDeviceAPIKey(APIKeyString);
					JsonLog.CreatedByKey = APIKey;
				}
			} catch (Exception E) {
				JsonLog.ParseException = true;
				JsonLog.ParseExcMessage = E.Message;
				JsonLog.ParseExcSource = E.Source;
				JsonLog.ParseExcStackTrace = E.StackTrace;
			}

			DbCtx.Commit();
			return JsonLog;
		}

		bool SetEquipmentData(DatabaseContext DbCtx, DbDeviceAPIKey DbAPIKey, DbJsonLog JsonLog) {
			List<DbVehicle> Vehicles = DbCtx.GetVehiclesByAPIKey(DbAPIKey);
			bool Dirty = false;

			(string, object)[] KVs = JsonUtils.GetKeyValues(JsonLog).ToArray();

			foreach (DbVehicle V in Vehicles) {
				foreach (DbVehicleEquipment Eq in V.Equipment) {
					foreach ((string, object) KV in KVs) {
						if (Eq.Field == KV.Item1) {

							DbEquipmentValues EqVal = DbCtx.CreateNew<DbEquipmentValues>(null, V => {
								V.CreatedByKey = DbAPIKey;
								V.FloatValue = (float)KV.Item2;
							});

							Eq.Values.Add(EqVal);
							Dirty = true;
						}
					}
				}
			}

			if (Dirty)
				DbCtx.Commit();

			return Dirty;
		}

		IEnumerable<DbVehicleEquipment> GetAllEquipmentForKey(DatabaseContext DbCtx, DeviceAccessAPI API, DbDeviceAPIKey DbAPIKey) {
			List<DbVehicle> Vehicles = DbCtx.GetVehiclesByAPIKey(DbAPIKey);

			foreach (DbVehicle V in Vehicles) {
				foreach (DbVehicleEquipment Eq in V.Equipment) {
					DbVehicleEquipment EqUntracked = DbCtx.Untrack(Eq);

					EqUntracked.Notifications = null;
					EqUntracked.Values = null;

					yield return EqUntracked;
				}
			}
		}
	}
}
