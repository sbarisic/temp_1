using Microsoft.AspNetCore.Mvc;
using System.Globalization;
using Proj2.Database;
using Microsoft.AspNetCore.SignalR;
using Proj2.Hubs;
using System.Threading.Tasks;

namespace Proj2.Code {
	public enum DeviceAccessStatus {
		Invalid = 0,
		OK = 200,
		Forbidden = 403,
		NotFound = 404,
	}

	public enum DeviceAction {
		Invalid = 0,
		Data = 1,
		GetEquipment = 2
	}

	public class DeviceAccessAPI {
		public int APIVersion {
			get; set;
		}

		public string APIKey {
			get; set;
		}

		public int Action {
			get; set;
		}

		public string EquipmentKey {
			get; set;
		}

		public string Value {
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

		public object Data {
			get; set;
		}

		public DeviceAccessResponseAPI(DeviceAccessStatus Status) {
			Title = nameof(DeviceAccessResponseAPI);

			this.Status = Status;
			StatusString = Status.ToString();
		}

		public DeviceAccessResponseAPI(DeviceAccessStatus Status, object Data) : this(Status) {
			this.Data = Data;
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

		JsonResult Forbidden() {
			return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.Forbidden));
		}

		JsonResult OK(object Data = null) {
			if (Data != null)
				new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.OK, Data));

			return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.OK));
		}


		[HttpPost("/deviceaccess")]
		public JsonResult Post([FromBody] DeviceAccessAPI API) {
			// Return invalid API calls
			if (API == null || API.APIVersion == 0 || API.Action == 0)
				return Invalid();

			if (string.IsNullOrEmpty(API.APIKey))
				return Forbidden();

			bool DispatchChanges = false;
			object Data = null;

			using (DatabaseContext DbCtx = new DatabaseContext()) {
				DbDeviceAPIKey DbAPIKey = DbCtx.GetDeviceAPIKey(API.APIKey);

				// If no API key, forbidden
				if (DbAPIKey == null || DbAPIKey.APIKey != API.APIKey)
					return Forbidden();

				switch (API.Action) {
					// Post data to vehicle equipment
					case (int)DeviceAction.Data: {
							DispatchChanges = SetEquipmentData(DbCtx, API, DbAPIKey);
							break;
						}

					// Get all vehicle equipment by key
					case (int)DeviceAction.GetEquipment: {
							Data = GetAllEquipmentForKey(DbCtx, API, DbAPIKey).ToArray();
							break;
						}

					default:
						return Forbidden();
				}

				// Dispatch state changed event to all clients

			}

			if (DispatchChanges) {
				HubContext.Clients.All.SendAsync("OnStateHasChanged").Wait();
			}

			return OK(Data);
		}

		bool SetEquipmentData(DatabaseContext DbCtx, DeviceAccessAPI API, DbDeviceAPIKey DbAPIKey) {
			List<DbVehicle> Vehicles = DbCtx.GetVehiclesByAPIKey(DbAPIKey);
			bool Dirty = false;

			foreach (DbVehicle V in Vehicles) {
				DbVehicleEquipment Eq = V.Equipment.Where(E => E.ID == API.EquipmentKey).SingleOrDefault();

				DbEquipmentValues Val = DbCtx.CreateNew<DbEquipmentValues>(null, V => {
					V.CreatedByKey = DbAPIKey;
					V.FloatValue = float.Parse(API.Value, CultureInfo.InvariantCulture);
				});

				Eq.Values.Add(Val);
				DbCtx.Commit();
				Dirty = true;
			}

			return Dirty;
		}

		IEnumerable<string> GetAllEquipmentForKey(DatabaseContext DbCtx, DeviceAccessAPI API, DbDeviceAPIKey DbAPIKey) {
			List<DbVehicle> Vehicles = DbCtx.GetVehiclesByAPIKey(DbAPIKey);

			foreach (DbVehicle V in Vehicles) {
				foreach (DbVehicleEquipment Eq in V.Equipment) {
					yield return Eq.ID;
				}
			}
		}
	}
}
