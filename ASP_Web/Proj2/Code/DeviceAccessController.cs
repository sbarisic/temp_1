using Microsoft.AspNetCore.Mvc;
using Proj2.Database;

namespace Proj2.Code {
	public enum DeviceAccessStatus {
		Invalid = 0,
		Success = 1,
		NotAuthorized = 2
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
		public DeviceAccessStatus Status {
			get; set;
		}

		public string StatusString {
			get; set;
		}

		public DeviceAccessResponseAPI(DeviceAccessStatus Status) {
			this.Status = Status;
			StatusString = Status.ToString();
		}
	}

	[ApiController]
	public class DeviceAccessController : ControllerBase {
		[HttpPost("/deviceaccess")]
		public JsonResult Post([FromBody] DeviceAccessAPI API) {
			if (API == null || API.APIVersion == 0)
				return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.Invalid));

			if (string.IsNullOrEmpty(API.APIKey))
				return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.NotAuthorized));

			using (DatabaseContext DbCtx = new DatabaseContext()) {

				Console.WriteLine("Hello Database World!");

			}

			return new JsonResult(new DeviceAccessResponseAPI(DeviceAccessStatus.Success));
		}
	}
}
