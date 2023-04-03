using Microsoft.AspNetCore.Mvc;
using Proj2.Database;

namespace Proj2.Code {
	public enum DeviceAccessStatus {
		Invalid = 0,
		OK = 200,
		Forbidden = 403,
		NotFound = 404,
	}

	public class DeviceAccessAPI {
		public int APIVersion {
			get; set;
		}

		public string APIKey {
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

		public DeviceAccessResponseAPI(DeviceAccessStatus Status) {
			Title = nameof(DeviceAccessResponseAPI);

			this.Status = Status;
			StatusString = Status.ToString();
		}
	}

	[ApiController]
	[RequireHttps]
	public class DeviceAccessController : ControllerBase {
		[HttpPost("/deviceaccess")]
		public JsonResult Post([FromBody] DeviceAccessAPI API) {
			if (API == null || API.APIVersion == 0)
				return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.Invalid));

			if (string.IsNullOrEmpty(API.APIKey))
				return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.Forbidden));

			using (DatabaseContext DbCtx = new DatabaseContext()) {
				DbDeviceAPIKey DbAPIKey = DbCtx.GetDeviceAPIKey(API.APIKey);

				if (DbAPIKey == null || DbAPIKey.APIKey != API.APIKey)
					return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.Forbidden));

				//DbCtx.Database.

				Console.WriteLine("Hello Database World!");

			}

			return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.OK));
		}
	}
}
